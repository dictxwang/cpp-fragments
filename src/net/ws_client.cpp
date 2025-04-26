#include <iostream>
#include <string>
#include <cstring>
#include <array>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/sha.h>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <vector>
#include <random>
#include <iomanip>


// Helper: Base64 encode
std::string base64_encode(const std::string& input) {
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string result;
    int val = 0, valb = -6;
    for (uint8_t c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');
    return result;
}

// Helper: SHA-1 hash
std::string sha1_hash(const std::string& input) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        oss << hash[i];
    }
    return oss.str();
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

int main() {
    const std::string host = "echo.websocket.events";
    const int port = 80;
    const std::string path = "/";
    // const std::string host = "stream.binance.com";
    // const int port = 9443;
    // const std::string path = "/stream";

    // Step 1: Resolve Hostname to IP
    struct hostent* server = gethostbyname(host.c_str());
    if (!server) {
        std::cerr << "Error: No such host." << std::endl;
        return 1;
    }

    // Step 2: Create a Socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Error: Cannot open socket." << std::endl;
        return 1;
    }

    // Step 3: Connect to the Server
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    std::memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Cannot connect to server." << std::endl;
        close(sock);
        return 1;
    }

    // Step 4: Perform WebSocket Handshake
    std::string websocket_key = "dGhlIHNhbXBsZSBub25jZQ=="; // Base64-encoded random key
    std::string handshake_request =
        "GET " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: " + websocket_key + "\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";

    if (send(sock, handshake_request.c_str(), handshake_request.size(), 0) < 0) {
        std::cerr << "Error: Handshake request failed." << std::endl;
        close(sock);
        return 1;
    }

    // Step 5: Read Handshake Response
    char response[1024];
    int response_len = recv(sock, response, sizeof(response) - 1, 0);
    if (response_len < 0) {
        std::cerr << "Error: Failed to read handshake response." << std::endl;
        close(sock);
        return 1;
    }

    response[response_len] = '\0';
    std::cout << "Server Response:\n" << response << std::endl;

    // Step 6: Send a WebSocket Message
    std::string message = "Hello, WebSocket!";
    // std::string frame = "\x81" + std::string(1, message.size()) + message; // Frame the message
    std::string frame = create_websocket_frame(message);
    if (send(sock, frame.c_str(), frame.size(), 0) < 0) {
        std::cerr << "Error: Failed to send WebSocket message." << std::endl;
        close(sock);
        return 1;
    }

    // Step 7: Receive WebSocket Message
    char buffer[1024];
    int len = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (len < 0) {
        std::cerr << "Error: Failed to receive WebSocket message." << std::endl;
        close(sock);
        return 1;
    }

    buffer[len] = '\0';
    std::cout << "Received WebSocket Message: " << buffer + 2 << std::endl; // Skip frame header

    // Step 8: Close the Connection
    close(sock);
    return 0;
}