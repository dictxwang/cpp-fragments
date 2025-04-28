#include <iostream>
#include <string>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>
#include <random>
#include <arpa/inet.h>

#include "rapidjson/document.h"
#include "util/ws/ws_packet.h"
#include "config/config.h"

const int WEBSOCKET_HEADER_MIN_SIZE = 5;

struct TickerInfo {
    std::string symbol;
    double best_bid;
    double bid_size;
    double best_ask;
    double ask_size;
    uint64_t update_id;
};

int generateRandomInt(int min, int max) {
    // Thread-local random engine
    thread_local std::mt19937 generator(std::random_device{}()); // Seed per thread

    // Uniform integer distribution
    std::uniform_int_distribution<int> distribution(min, max);

    // Generate random number
    return distribution(generator);
}

// Helper function: Create a secure TLS connection
SSL* create_tls_connection(const std::string& host, int port, int& socket_fd, const char* remote_ip, const char* local_ip) {
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
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (remote_ip != nullptr && strlen(remote_ip) > 0) {
        // Use the provided server address
        server_addr.sin_addr.s_addr = inet_addr(remote_ip);
    } else {
        // Use the resolved server address
        std::memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    }

    // Step 3: Create a TCP socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Error: Failed to create socket." << std::endl;
        return nullptr;
    }

    // Step 4: Set the local IP address (optional)
    if (local_ip != nullptr && strlen(local_ip) > 0) {
        struct sockaddr_in local_addr{};

        // std::memset(&local_addr, 0, sizeof(local_addr)); // Clear the structure, setting all fields to zero
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(0); // Use any available port
        local_addr.sin_addr.s_addr = inet_addr(local_ip);
        if (bind(socket_fd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1) {
            std::cerr << "Error: Failed to bind local IP." << std::endl;
            close(socket_fd);
            return nullptr;
        }
    }

    // Step 5: Connect to the server
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error: Failed to connect to server." << std::endl;
        close(socket_fd);
        return nullptr;
    }

    // Step 6: Create an SSL object and attach it to the socket
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket_fd);

    // Step 7: Perform the TLS handshake
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

bool subscribe_ticker_book(SSL* ssl) {

    std::string message = "{\"method\":\"SUBSCRIBE\", \"id\":1, \"params\": [\"btcusdt@bookTicker\"]}";
    std::string frame = "\x81" + std::string(1, message.size()) + message; // Simplified frame
    int writeLength = SSL_write(ssl, frame.c_str(), frame.size());
    std::cout << "Sent message: " << message << std::endl;
    if (writeLength <= 0) {
        std::cerr << "Error: Failed to send WebSocket message." << std::endl;
        return false;
    }

    char buffer[1024];
    int len = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (len < 0) {
        std::cerr << "Error: Failed to receive subscribe message." << std::endl;
        return false;
    } else {
        buffer[len] = '\0';
        std::cout << "Received Subscribe Message: " << buffer + 2 << std::endl;
        return true;
    }
}

int process_one_message(SSL* ssl, WebSocketPacket& packet, ByteBuffer& msg_buf) {
    
    if (packet.get_opcode() == WebSocketPacket::WSOpcode_Ping) {
        // Handle Ping
        std::cout << "Received Ping Frame" << std::endl;
        // Send Pong response
        if (ssl == nullptr) {
            std::cerr << "Error: SSL object is null." << std::endl;
            return 1;
        }
        packet.set_fin(1);
        packet.set_opcode(WebSocketPacket::WSOpcode_Pong);
        ByteBuffer pong_buf;
        packet.pack_dataframe(pong_buf);
        int writeLength = SSL_write(ssl, pong_buf.bytes(), pong_buf.length());
        if (writeLength <= 0) {
            std::cerr << "Error: Failed to send Pong message." << std::endl;
            return 1;
        }
        std::cout << "Sent Pong Frame" << std::endl;
    } else if (packet.get_opcode() == WebSocketPacket::WSOpcode_Continue) {
        // Handle continuation frame
    }
    if (packet.get_opcode() == WebSocketPacket::WSOpcode_Text) {

        int rand_int = generateRandomInt(1, 1000);

        // Handle text frame
        if (rand_int < 20) {
            std::cout << "Received Text Frame: " << std::string(msg_buf.bytes(), msg_buf.length()) << std::endl;
        }

        rapidjson::Document document;
        document.Parse(std::string(msg_buf.bytes(), msg_buf.length()).c_str());
        if (document.HasParseError()) {
            std::cerr << "Error: Failed to parse JSON message." << std::endl;
            return 1;
        }
        if (!document.HasMember("data")) {
            std::cerr << "Error: JSON message does not contain 'data' field." << std::endl;
            return 1;
        }
        rapidjson::Value &value = document["data"];
        if (value.HasMember("s") && value.HasMember("b") && value.HasMember("B") &&
            value.HasMember("a") && value.HasMember("A") && value.HasMember("u")) {
            TickerInfo ticker_info;
            ticker_info.symbol = value["s"].GetString();
            ticker_info.best_bid = std::atof(value["b"].GetString());
            ticker_info.bid_size = std::atof(value["B"].GetString());
            ticker_info.best_ask = std::atof(value["a"].GetString());
            ticker_info.ask_size = std::atof(value["A"].GetString());
            ticker_info.update_id = value["u"].GetUint64();

            if (rand_int < 20) {
                std::cout << "Ticker Info: " << ticker_info.symbol << ", "
                        << "Best Bid: " << ticker_info.best_bid << ", "
                        << "Bid Size: " << ticker_info.bid_size << ", "
                        << "Best Ask: " << ticker_info.best_ask << ", "
                        << "Ask Size: " << ticker_info.ask_size << ", "
                        << "Update Id: " << ticker_info.update_id << std::endl;
            }
        } else {
            std::cerr << "Error: JSON message does not contain expected fields." << std::endl;
        }
    }

    return 0;
}

bool read_and_process_message(SSL* ssl) {
    
    if (ssl == nullptr) {
        std::cerr << "Error: SSL object is null." << std::endl;
        return false;
    }

    ByteBuffer recv_buf;
    ByteBuffer msg_buf;
    
    while (true) {
        int total_recv_len = 0;
        int max_buffer_size = 1024;
        char buffer[max_buffer_size];
        int read_len = 0;
        int pendding_len = 0;
        int recv_start_len = recv_buf.length();
        do {
            read_len = SSL_read(ssl, buffer, sizeof(buffer) - 1);
            if (read_len > 0) {
                recv_buf.append(buffer, read_len);
                total_recv_len += read_len;
            } else {
                break;
            }
            pendding_len = SSL_pending(ssl);
        } while (pendding_len > 0);

        if (read_len < 0) {
            int err = SSL_get_error(ssl, read_len);
            ERR_clear_error();
            if (err == SSL_ERROR_WANT_READ) {
                // No data available, continue reading
                continue;
            } else if (err == SSL_ERROR_WANT_WRITE) {
                // No data available, continue reading
                continue;
            } else {
                std::cerr << "Error: Failed to read WebSocket message." << std::endl;
                return false;
            }
        }

        if (total_recv_len <= 0) {
            return false;
        }

        if (recv_buf.length() > recv_start_len && recv_buf.length() > recv_buf.getoft() + WEBSOCKET_HEADER_MIN_SIZE) {
            // parse the message
            WebSocketPacket ws_packet;
            while (true) {
                uint32_t offset = ws_packet.recv_dataframe(recv_buf);
                if (offset == 0) {
                    // Not enough data to process, break the loop
                    break;
                }
                ByteBuffer &payload = ws_packet.get_payload();
                msg_buf.append(payload.bytes(), payload.length());
                if (ws_packet.get_fin() == 1) {
                    // Complete message received
                    // std::cout << "Received WebSocket Message: " << std::string(msg_buf.bytes(), msg_buf.length()) << std::endl;
                    // Process one complete message
                    process_one_message(ssl, ws_packet, msg_buf);
                    msg_buf.clear();
                }

                if (recv_buf.getoft() + WEBSOCKET_HEADER_MIN_SIZE >= recv_buf.length()) {
                    // No more data to process, break the packet parsing loop and continue reading
                    break;
                }
                ws_packet.get_payload().clear();
            }

            recv_buf.erase(recv_buf.getoft());
            recv_buf.resetoft();
        } else {
            // No complete message received yet
            continue;
        }
    }
}

// Main function
int main(int argc, char **argv) {

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " config_file" << std::endl;
        return 0;
    }

    Config config;
    if (!config.LoadConfig(argv[1])) {
        std::cerr << "Load config error : " << argv[1] << std::endl;
        return 1;
    }

    // const std::string host = "echo.websocket.events"; // Example WSS server
    // const int port = 443;                             // WSS port
    // const std::string path = "/";
    const std::string host = "stream.binance.com";
    const int port = 9443;
    const std::string path = "/stream";
    const std::string remote_ip = config.binance_ticker_ws_ip; // Optional: Set to your remote IP address if needed
    const std::string local_ip = config.binance_ticker_local_ip; // Optional: Set to your local IP address if needed

    // Step 1: Create a secure TLS connection
    int socket_fd = 0;
    SSL* ssl = create_tls_connection(host, port, socket_fd, remote_ip.c_str(), local_ip.c_str());
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
    // std::string message = "Hello, WSS!";
    // std::string frame = "\x81" + std::string(1, message.size()) + message; // Simplified frame
    // SSL_write(ssl, frame.c_str(), frame.size());
    // std::cout << "Sent message: " << message << std::endl;

    // Step 3: Subscribe to a WebSocket channel
    if (!subscribe_ticker_book(ssl)) {
        SSL_free(ssl);
        close(socket_fd);
        return 1;
    }

    // Step 4: Receive WebSocket messages
    if (!read_and_process_message(ssl)) {
        std::cerr << "Error: Failed to read and process WebSocket messages." << std::endl;
    }

    // Step 5: Clean up
    SSL_free(ssl);
    close(socket_fd);
    return 0;
}