/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network utility implementation
*/

#include "NetworkUtils.hpp"

#include <cerrno>
#include <cstring>
#include <mutex>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "Logger/Macros.hpp"

namespace rtype::server {

#ifdef _WIN32
bool ensureWinsockInitialized() noexcept {
    static std::once_flag flag;
    static bool initialized = false;
    std::call_once(flag, []() {
        WSADATA wsa{};
        initialized = (WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
    });
    return initialized;
}
#else
bool ensureWinsockInitialized() noexcept { return true; }
#endif

bool isUdpPortAvailable(std::uint16_t port) noexcept {
    if (port == 0) return true;  // 0 = OS assigns, considered available

#ifdef _WIN32
    if (!ensure_winsock_initialized()) {
        LOG_DEBUG_CAT(::rtype::LogCategory::Network,
                      "[NetworkUtils] WSAStartup failed");
        return false;
    }

    SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        LOG_DEBUG_CAT(::rtype::LogCategory::Network,
                      "[NetworkUtils] socket() failed: " << WSAGetLastError());
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    int res = ::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (res == 0) {
        ::closesocket(sock);
        return true;
    }

    ::closesocket(sock);
    return false;
#else
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
#endif
}

}  // namespace rtype::server
