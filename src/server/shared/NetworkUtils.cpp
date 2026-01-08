/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network utility implementation
*/

#include "NetworkUtils.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include "Logger/Macros.hpp"

namespace rtype::server {

bool isUdpPortAvailable(std::uint16_t port) noexcept {
    if (port == 0) return true;  // 0 = OS assigns, considered available

    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        LOG_DEBUG_CAT(::rtype::LogCategory::Network,
                      "[NetworkUtils] socket() failed: " << strerror(errno));
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int res = ::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (res == 0) {
        ::close(fd);
        return true;
    }

    ::close(fd);
    return false;
}

}  // namespace rtype::server
