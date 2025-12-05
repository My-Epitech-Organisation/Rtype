/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Types - Common types shared between client and server
*/

#ifndef SRC_COMMON_TYPES_HPP_
#define SRC_COMMON_TYPES_HPP_

#include <array>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace rtype {

/**
 * @brief Unique identifier for a connected client
 */
using ClientId = uint32_t;

/**
 * @brief Network endpoint information
 *
 * Represents a network address and port combination.
 * Provides comparison operators for use in containers.
 *
 * @note Supports move semantics for efficient transfers without string copies.
 */
struct Endpoint {
    std::string address;
    uint16_t port = 0;

    /**
     * @brief Default constructor
     */
    Endpoint() = default;

    /**
     * @brief Construct from address and port
     * @param addr Network address
     * @param p Port number
     */
    Endpoint(std::string addr, uint16_t p)
        : address(std::move(addr)), port(p) {}

    /**
     * @brief Copy constructor
     */
    Endpoint(const Endpoint&) = default;

    /**
     * @brief Move constructor (no string copy)
     * @param other Endpoint to move from
     */
    Endpoint(Endpoint&& other) noexcept
        : address(std::move(other.address)), port(other.port) {}

    /**
     * @brief Copy assignment
     */
    Endpoint& operator=(const Endpoint&) = default;

    /**
     * @brief Move assignment (no string copy)
     * @param other Endpoint to move from
     * @return Reference to this endpoint
     */
    Endpoint& operator=(Endpoint&& other) noexcept {
        if (this != &other) {
            address = std::move(other.address);
            port = other.port;
        }
        return *this;
    }

    ~Endpoint() = default;

    /**
     * @brief Check if the endpoint is valid
     * @return true if address is non-empty and port is non-zero
     */
    [[nodiscard]] bool isValid() const noexcept {
        return !address.empty() && port != 0;
    }

    /**
     * @brief Compare two endpoints for equality
     * @param other The endpoint to compare with
     * @return true if both address and port match
     */
    [[nodiscard]] bool operator==(const Endpoint& other) const noexcept {
        return port == other.port && address == other.address;
    }

    /**
     * @brief Compare two endpoints for inequality
     * @param other The endpoint to compare with
     * @return true if address or port differ
     */
    [[nodiscard]] bool operator!=(const Endpoint& other) const noexcept {
        return !(*this == other);
    }

    /**
     * @brief Less-than comparison for ordered containers
     * @param other The endpoint to compare with
     * @return true if this endpoint is less than other
     */
    [[nodiscard]] bool operator<(const Endpoint& other) const noexcept {
        if (address != other.address) return address < other.address;
        return port < other.port;
    }

    /**
     * @brief Convert endpoint to string representation using std::format
     * (C++20)
     * @return String in format "address:port"
     */
    [[nodiscard]] std::string toString() const {
        return std::format("{}:{}", address, port);
    }
};

/**
 * @brief Stream insertion operator for Endpoint
 * @param os Output stream
 * @param endpoint Endpoint to stream
 * @return Reference to the output stream
 */
inline std::ostream& operator<<(std::ostream& os, const Endpoint& endpoint) {
    return os << endpoint.toString();
}

}  // namespace rtype

/**
 * @brief Hash specialization for Endpoint to enable use in unordered containers
 *
 * Uses a hash combine algorithm similar to boost::hash_combine for better
 * distribution and fewer collisions.
 */
template <>
struct std::hash<rtype::Endpoint> {
    [[nodiscard]] std::size_t operator()(
        const rtype::Endpoint& endpoint) const noexcept {
        std::size_t seed = std::hash<std::string>{}(endpoint.address);
        seed ^= std::hash<uint16_t>{}(endpoint.port) + 0x9e3779b9 +
                (seed << 6) + (seed >> 2);
        return seed;
    }
};

namespace rtype {

/**
 * @brief Client connection state
 */
enum class ClientState {
    Connecting,    ///< Client is in the process of connecting
    Connected,     ///< Client is fully connected and active
    Disconnecting  ///< Client is being disconnected
};

/**
 * @brief Reason for client disconnection
 */
enum class DisconnectReason {
    Disconnected,  ///< Client disconnected gracefully
    Timeout,       ///< Client timed out (no activity)
    Kicked,        ///< Client was kicked by server
    Error          ///< Network error occurred
};

/**
 * @brief String representations of DisconnectReason values
 */
inline constexpr std::array<std::string_view, 4> DisconnectReasonStrings = {
    "disconnected", "timeout", "kicked", "error"};

/**
 * @brief Convert DisconnectReason to string representation
 * @param reason The disconnect reason
 * @return String view of the reason, or "unknown" if out of range
 */
[[nodiscard]] inline constexpr std::string_view toString(
    DisconnectReason reason) noexcept {
    const auto index = static_cast<std::size_t>(reason);
    if (index >= DisconnectReasonStrings.size()) {
        return "unknown";
    }
    return DisconnectReasonStrings[index];
}

/**
 * @brief String representations of ClientState values
 */
inline constexpr std::array<std::string_view, 3> ClientStateStrings = {
    "connecting", "connected", "disconnecting"};

/**
 * @brief Convert ClientState to string representation
 * @param state The client state
 * @return String view of the state, or "unknown" if out of range
 */
[[nodiscard]] inline constexpr std::string_view toString(
    ClientState state) noexcept {
    const auto index = static_cast<std::size_t>(state);
    if (index >= ClientStateStrings.size()) {
        return "unknown";
    }
    return ClientStateStrings[index];
}

}  // namespace rtype

#endif  // SRC_COMMON_TYPES_HPP_
