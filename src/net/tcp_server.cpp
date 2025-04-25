#include <iostream>
#include <thread>
#include <cstring>
#include <chrono>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// using namespace std;

std::mutex cout_mutex; // Mutex to synchronize console output

void handle_client(int client_socket) {
    {
        // Use mutex to safely print to console
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Client connected. Socket: " << client_socket << std::endl;
    }
    std::cout<< "aaaa" << std::endl;
    char buffer[1024] = {0};
    /**
    while (true) {
        try {
            // memset(buffer, 0, sizeof(buffer)-1);
            // buffer[1023] = '\0';
            std::cout<< "xxxx" << std::endl;
            int bytes_received = read(client_socket, buffer, sizeof(buffer));
            std::cout<< "yyyy" << std::endl;
            if (bytes_received <= 0) {
                // Client disconnected or error occurred
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Client disconnected. Socket: " << client_socket << std::endl;
                break;
            }

            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Received from client [" << client_socket << "]: " << buffer << std::endl;
            }
            const char* message = "Hello, Client!";
            write(client_socket, message, strlen(message)); // Echo back
        } catch (const std::exception& e) {
            std::cerr << "Caught exception: " << e.what() << std::endl;
        }
    }
    */

    while (true) {
        std::cout << "Keep Connection..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    close(client_socket);
}

int main(int argc, char const *argv[])
{
    std::cout << "Arguments Count Is " << argc << std::endl;

    const int PORT = 18080;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    // Set socket options for reuse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Set socket options failed");
        close(server_socket);
        return -1;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        close(server_socket);
        return -1;
    }

    std::cout<< "Server listening on port " << PORT << std::endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        std::cout << "Waiting for client connection..." << std::endl;
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl;

        // Set read timeout
        timeval timeout;
        timeout.tv_sec = 30;  // Timeout in seconds
        timeout.tv_usec = 0; // Timeout in microseconds
        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("Set timeout failed");
        }

        // print client info
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "New connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
        }

        std::thread(handle_client, client_socket); // Handle client in a separate thread
    }
    

    while(true) {
        std::cout << "Keep Running..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}
