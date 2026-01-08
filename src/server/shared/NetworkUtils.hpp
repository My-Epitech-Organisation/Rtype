/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** Network utility helpers
*/

#ifndef SRC_SERVER_SHARED_NETWORKUTILS_HPP_
#define SRC_SERVER_SHARED_NETWORKUTILS_HPP_

#include <cstdint>

namespace rtype::server {

/**
 * @brief Check if a UDP port is available to bind on the local host
 *
 * Returns true if the port can be bound on INADDR_ANY, false otherwise.
 * Port 0 is treated as always available (OS-assigned).
 */
[[nodiscard]] bool isUdpPortAvailable(std::uint16_t port) noexcept;

}  // namespace rtype::server

#endif  // SRC_SERVER_SHARED_NETWORKUTILS_HPP_
