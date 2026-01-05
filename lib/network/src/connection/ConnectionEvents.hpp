/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ConnectionEvents - Callback types for connection state changes
*/

#pragma once

#include <cstdint>
#include <functional>
#include <ostream>

#include "ConnectionState.hpp"
#include "core/Error.hpp"

namespace rtype::network {

/**
 * @brief Reason for connection termination
 */
enum class DisconnectReason : std::uint8_t {
    Timeout = 0,
    MaxRetriesExceeded = 1,
    ProtocolError = 2,
    RemoteRequest = 3,
    LocalRequest = 4
};

[[nodiscard]] constexpr std::string_view toString(
    DisconnectReason reason) noexcept {
    switch (reason) {
        case DisconnectReason::LocalRequest:
            return "LocalRequest";
        case DisconnectReason::RemoteRequest:
            return "RemoteRequest";
        case DisconnectReason::Timeout:
            return "Timeout";
        case DisconnectReason::MaxRetriesExceeded:
            return "MaxRetriesExceeded";
        case DisconnectReason::ProtocolError:
            return "ProtocolError";
    }
    return "Unknown";
}

using OnStateChange =
    std::function<void(ConnectionState oldState, ConnectionState newState)>;

using OnConnected = std::function<void(std::uint32_t assignedUserId)>;

using OnDisconnected = std::function<void(DisconnectReason reason)>;

using OnConnectFailed = std::function<void(NetworkError error)>;

/**
 * @brief Container for all connection event callbacks
 */
struct ConnectionCallbacks {
    OnStateChange onStateChange;
    OnConnected onConnected;
    OnDisconnected onDisconnected;
    OnConnectFailed onConnectFailed;
};

}  // namespace rtype::network

inline std::ostream& operator<<(std::ostream& os,
                                rtype::network::DisconnectReason reason) {
    return os << rtype::network::toString(reason);
}
