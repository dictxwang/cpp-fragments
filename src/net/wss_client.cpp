#include <iostream>
#include <string>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>
#include <random>

// Helper function: Create a secure TLS connection
SSL* create_tls_connection(const std::string& host, int port, int& socket_fd) {
    // Step 1: Initialize OpenSSL
    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        std::cerr << "Failed to create SSL context." << std::endl;
        return nullptr;
    }

    // Step 2: Resolve the server address
    struct hostent* server = gethostbyname(host.c_str());
    if (!server) {
        std::cerr << "Error: No such host." << std::endl;
        return nullptr;
    }

    // Step 3: Create a TCP socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return nullptr;
    }

    // Step 4: Connect to the server
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    std::memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Failed to connect to server." << std::endl;
        close(socket_fd);
        return nullptr;
    }

    // Step 5: Create an SSL object and attach it to the socket
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket_fd);

    // Step 6: Perform the TLS handshake
    if (SSL_connect(ssl) <= 0) {
        std::cerr << "Error: TLS handshake failed." << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(socket_fd);
        return nullptr;
    }

    std::cout << "Connected to WSS server with TLS encryption!" << std::endl;
    return ssl;
}

// Helper function: Perform WebSocket handshake
bool perform_websocket_handshake(SSL* ssl, const std::string& host, const std::string& path) {
    // Step 1: Create WebSocket HTTP handshake request
    std::string websocket_key = "dGhlIHNhbXBsZSBub25jZQ=="; // Base64-encoded random key
    std::string request =
        "GET " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: " + websocket_key + "\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";

    // Step 2: Send the handshake request
    if (SSL_write(ssl, request.c_str(), request.size()) <= 0) {
        std::cerr << "Error: Failed to send WebSocket handshake request." << std::endl;
        return false;
    }

    // Step 3: Read the server's response
    char response[4096];
    int bytes_read = SSL_read(ssl, response, sizeof(response) - 1);
    if (bytes_read <= 0) {
        std::cerr << "Error: Failed to read WebSocket handshake response." << std::endl;
        return false;
    }

    response[bytes_read] = '\0';
    std::cout << "Server Response:\n" << response << std::endl;

    // Step 4: Check for "101 Switching Protocols" in the response
    if (std::string(response).find("101 Switching Protocols") == std::string::npos) {
        std::cerr << "Error: WebSocket handshake failed." << std::endl;
        return false;
    }

    std::cout << "WebSocket handshake successful!" << std::endl;
    return true;
}

// Generate a 4-byte random masking key
std::vector<uint8_t> generate_masking_key() {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<uint8_t> distribution(0, 255);

    std::vector<uint8_t> key(4);
    for (auto& byte : key) {
        byte = distribution(generator);
    }
    return key;
}

// Apply masking key to the payload
std::string mask_payload(const std::string& payload, const std::vector<uint8_t>& masking_key) {
    std::string masked_payload = payload;
    for (size_t i = 0; i < payload.size(); ++i) {
        masked_payload[i] ^= masking_key[i % 4];
    }
    return masked_payload;
}

std::string create_websocket_frame(const std::string& payload) {
    std::string frame;

    // Byte 1: FIN + Opcode (0x81 for text frame)
    frame += static_cast<char>(0x81);

    // Generate a 4-byte masking key
    std::vector<uint8_t> masking_key = generate_masking_key();

    // Byte 2: Mask flag (1 << 7) + Payload length
    size_t payload_length = payload.size();
    if (payload_length <= 125) {
        frame += static_cast<char>((1 << 7) | payload_length);
    } else if (payload_length <= 65535) {
        frame += static_cast<char>((1 << 7) | 126); // Extended payload length (16-bit)
        frame += static_cast<char>((payload_length >> 8) & 0xFF);
        frame += static_cast<char>(payload_length & 0xFF);
    } else {
        frame += static_cast<char>((1 << 7) | 127); // Extended payload length (64-bit)
        for (int i = 7; i >= 0; --i) {
            frame += static_cast<char>((payload_length >> (8 * i)) & 0xFF);
        }
    }

    // Bytes 3–6: Masking key
    for (const auto& byte : masking_key) {
        frame += static_cast<char>(byte);
    }

    // Mask the payload and append it
    std::string masked_payload = mask_payload(payload, masking_key);
    frame += masked_payload;

    return frame;
}

// Main function
int main() {
    // const std::string host = "echo.websocket.events"; // Example WSS server
    // const int port = 443;                             // WSS port
    // const std::string path = "/";
    const std::string host = "stream.binance.com";
    const int port = 9443;
    const std::string path = "/stream";

    // Step 1: Create a secure TLS connection
    int socket_fd = 0;
    SSL* ssl = create_tls_connection(host, port, socket_fd);
    if (!ssl) {
        return 1;
    }

    // Step 2: Perform the WebSocket handshake
    if (!perform_websocket_handshake(ssl, host, path)) {
        SSL_free(ssl);
        close(socket_fd);
        return 1;
    }

    // Step 3: Send a WebSocket message (optional)
    std::string message = "Hello, WSS!";
    std::string frame = "\x81" + std::string(1, message.size()) + message; // Simplified frame
    SSL_write(ssl, frame.c_str(), frame.size());
    std::cout << "Sent message: " << message << std::endl;

    char buffer[1024];
    int len = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (len < 0) {
        std::cerr << "Error: Failed to receive WebSocket message." << std::endl;
    } else {

        buffer[len] = '\0';
        std::cout << "Received WebSocket Message: " << buffer + 2 << std::endl;
    }

    // Step 4: Clean up
    SSL_free(ssl);
    close(socket_fd);
    return 0;
}