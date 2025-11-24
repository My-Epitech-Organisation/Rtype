/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ACE UDP Client PoC - Simple implementation without full ACE build
*/

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <string>

class SimpleUdpClient {
public:
    SimpleUdpClient(const std::string& host, unsigned short port) 
        : host_(host), port_(port), socket_fd_(-1) {
        std::cout << "ACE-style UDP Client (Simple Implementation)" << std::endl;
    }

    ~SimpleUdpClient() {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
        }
    }

    bool connect() {
        socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        std::memset(&server_addr_, 0, sizeof(server_addr_));
        server_addr_.sin_family = AF_INET;
        server_addr_.sin_port = htons(port_);
        
        if (inet_pton(AF_INET, host_.c_str(), &server_addr_.sin_addr) <= 0) {
            std::cerr << "Invalid address: " << host_ << std::endl;
            close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }

        std::cout << "Connected to " << host_ << ":" << port_ << std::endl;
        return true;
    }

    bool send_message(const std::string& message) {
        std::cout << "Sending: \"" << message << "\"" << std::endl;

        ssize_t sent = sendto(socket_fd_, message.c_str(), message.length(), 0,
                             (struct sockaddr*)&server_addr_, sizeof(server_addr_));
        
        if (sent < 0) {
            std::cerr << "Send error" << std::endl;
            return false;
        }

        // Receive response
        char buffer[1024];
        std::memset(buffer, 0, sizeof(buffer));
        
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        
        ssize_t recv_len = recvfrom(socket_fd_, buffer, sizeof(buffer) - 1, 0,
                                   (struct sockaddr*)&from_addr, &from_len);
        
        if (recv_len > 0) {
            std::string response(buffer, recv_len);
            std::cout << "Received: \"" << response << "\"" << std::endl;
            return true;
        }

        return false;
    }

private:
    std::string host_;
    unsigned short port_;
    int socket_fd_;
    struct sockaddr_in server_addr_;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    std::string host = argv[1];
    unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

    SimpleUdpClient client(host, port);

    if (!client.connect()) {
        return 1;
    }

    // Send test messages
    std::vector<std::string> test_messages = {
        "Hello from ACE-style client!",
        "Testing ACE pattern",
        "Performance benchmark"
    };

    for (const auto& msg : test_messages) {
        if (!client.send_message(msg)) {
            std::cerr << "Failed to send message" << std::endl;
            return 1;
        }
    }

    std::cout << "All messages sent successfully!" << std::endl;
    return 0;
}
