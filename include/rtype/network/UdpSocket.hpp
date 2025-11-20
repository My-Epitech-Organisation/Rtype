/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** UdpSocket
*/

#pragma once

#include <string>
#include <cstdint>

namespace rtype::network {

class UdpSocket {
public:
    UdpSocket();
    ~UdpSocket();

    bool bind(uint16_t port);
    bool connect(const std::string& host, uint16_t port);

    int send(const void* data, size_t size);
    int receive(void* buffer, size_t maxSize);

    void close();

private:
    int socket_;
};

}  // namespace rtype::network
