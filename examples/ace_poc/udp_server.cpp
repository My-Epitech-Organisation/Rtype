/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ACE UDP Server PoC - Simple implementation without full ACE build
*/

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class SimpleUdpServer {
public:
    SimpleUdpServer(unsigned short port) : port_(port), socket_fd_(-1) {
        std::cout << "ACE-style UDP Server (Simple Implementation)" << std::endl;
    }

    ~SimpleUdpServer() {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
        }
    }

    bool open() {
        socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port_);

        if (bind(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to bind to port " << port_ << std::endl;
            close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }

        std::cout << "Server listening on port " << port_ << std::endl;
        return true;
    }

    void run() {
        char buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        std::cout << "ACE-style Reactor pattern (simplified) running..." << std::endl;

        while (true) {
            std::memset(buffer, 0, sizeof(buffer));

            ssize_t recv_len = recvfrom(socket_fd_, buffer, sizeof(buffer) - 1, 0,
                                       (struct sockaddr*)&client_addr, &client_len);

            if (recv_len < 0) {
                std::cerr << "Receive error" << std::endl;
                continue;
            }

            std::string message(buffer, recv_len);
            std::cout << "Received: \"" << message << "\" from "
                      << inet_ntoa(client_addr.sin_addr) << ":"
                      << ntohs(client_addr.sin_port) << std::endl;

            // Echo back
            std::string response = "Echo: " + message;
            sendto(socket_fd_, response.c_str(), response.length(), 0,
                  (struct sockaddr*)&client_addr, client_len);
        }
    }

private:
    unsigned short port_;
    int socket_fd_;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    unsigned short port = static_cast<unsigned short>(std::atoi(argv[1]));

    SimpleUdpServer server(port);

    if (!server.open()) {
        return 1;
    }

    server.run();

    return 0;
}
