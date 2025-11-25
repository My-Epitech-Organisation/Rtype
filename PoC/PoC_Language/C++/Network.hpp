/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Network
*/

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <tuple>

struct ReceiveResult {
    std::string message;
    std::string senderIp;
    int senderPort;
};

class UdpSocket {
public:
    UdpSocket() : sockfd(-1) {}

    ~UdpSocket() {
        if (sockfd != -1) {
            close(sockfd);
        }
    }

    bool create() {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        return sockfd != -1;
    }

    bool bindSocket(int port) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        return bind(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
    }

    bool sendTo(const std::string& message, const std::string& ip, int port) {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

        ssize_t sent = sendto(sockfd, message.c_str(), message.size(), 0,
                              reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        return sent == static_cast<ssize_t>(message.size());
    }

    ReceiveResult receiveFrom() {
        char buffer[1024];
        sockaddr_in senderAddr{};
        socklen_t addrLen = sizeof(senderAddr);

        ssize_t received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                    reinterpret_cast<sockaddr*>(&senderAddr), &addrLen);
        if (received > 0) {
            buffer[received] = '\0';
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &senderAddr.sin_addr, ipStr, sizeof(ipStr));
            return {std::string(buffer), std::string(ipStr), ntohs(senderAddr.sin_port)};
        }
        return {"", "", 0};
    }

private:
    int sockfd;
};
