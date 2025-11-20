/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** UdpSocket
*/

#include "rtype/network/UdpSocket.hpp"
#include <string>

namespace rtype::network {

UdpSocket::UdpSocket() : socket_(-1) {}

UdpSocket::~UdpSocket() {
    close();
}

bool UdpSocket::bind(uint16_t port) {
    // Placeholder - would create and bind actual socket
    (void)port;
    return false;
}

bool UdpSocket::connect(const std::string& host, uint16_t port) {
    // Placeholder
    (void)host;
    (void)port;
    return false;
}

int UdpSocket::send(const void* data, size_t size) {
    // Placeholder
    (void)data;
    (void)size;
    return -1;
}

int UdpSocket::receive(void* buffer, size_t maxSize) {
    // Placeholder
    (void)buffer;
    (void)maxSize;
    return -1;
}

void UdpSocket::close() {
    socket_ = -1;
}

}  // namespace rtype::network
