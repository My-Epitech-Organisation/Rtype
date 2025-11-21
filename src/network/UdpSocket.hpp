/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** UdpSocket
*/

#pragma once

#include <string>
#include <cstdint>
#include "rtype/network/CircularBuffer.hpp"

namespace rtype::network {

class UdpSocket {
public:
    explicit UdpSocket(size_t bufferSize = 4096);
    ~UdpSocket();

    bool bind(uint16_t port);
    bool connect(const std::string& host, uint16_t port);

    int send(const void* data, size_t size);
    int receive(void* buffer, size_t maxSize);

    void close();

private:
    int socket_;
    CircularBuffer receiveBuffer;
};

}  // namespace rtype::network
