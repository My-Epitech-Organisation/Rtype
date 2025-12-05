/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ConnectionState - Connection lifecycle states and transition guards
*/

#pragma once

#include <cstdint>
#include <string_view>

namespace rtype::network {

/**
 * @brief Connection lifecycle states as per RFC RTGP v1.1.0
 *
 * State machine diagram:
 * @code
 *     DISCONNECTED ──connect()──► CONNECTING ──S_ACCEPT──► CONNECTED
 *           ▲                         │                        │
 *           │                    timeout/max                   │
 *           │                    retries                       │
 *           │                         │                   disconnect()
 *           │                         ▼                        │
 *           └─────────────────── DISCONNECTING ◄───────────────┘
 * @endcode
 */
enum class ConnectionState : std::uint8_t {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting
};

[[nodiscard]] constexpr std::string_view toString(
    ConnectionState state) noexcept {
    switch (state) {
        case ConnectionState::Disconnected:
            return "Disconnected";
        case ConnectionState::Connecting:
            return "Connecting";
        case ConnectionState::Connected:
            return "Connected";
        case ConnectionState::Disconnecting:
            return "Disconnecting";
    }
    return "Unknown";
}

[[nodiscard]] constexpr bool canInitiateConnect(
    ConnectionState current) noexcept {
    return current == ConnectionState::Disconnected;
}

[[nodiscard]] constexpr bool canReceiveAccept(
    ConnectionState current) noexcept {
    return current == ConnectionState::Connecting;
}

[[nodiscard]] constexpr bool canInitiateDisconnect(
    ConnectionState current) noexcept {
    return current == ConnectionState::Connected ||
           current == ConnectionState::Connecting;
}

[[nodiscard]] constexpr bool canFinalizeDisconnect(
    ConnectionState current) noexcept {
    return current == ConnectionState::Disconnecting;
}

[[nodiscard]] constexpr bool canSendData(ConnectionState current) noexcept {
    return current == ConnectionState::Connected;
}

[[nodiscard]] constexpr bool isTerminalState(ConnectionState current) noexcept {
    return current == ConnectionState::Disconnected;
}

}  // namespace rtype::network

#include <ostream>

inline std::ostream& operator<<(std::ostream& os,
                                rtype::network::ConnectionState state) {
    return os << rtype::network::toString(state);
}
