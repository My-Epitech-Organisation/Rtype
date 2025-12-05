/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Network Byte Order - Big-endian conversion utilities
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <type_traits>

// Platform detection for byte order intrinsics
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace rtype::network {

/**
 * @brief Byte order conversion utilities for network protocol compliance
 *
 * All multi-byte fields in RTGP must be transmitted in Network Byte Order
 * (Big-Endian) as specified in RFC Section 2.2.
 *
 * These utilities handle conversion between host byte order and network
 * byte order transparently across platforms.
 */
namespace ByteOrder {

// ============================================================================
// Type Traits
// ============================================================================

/// Check if type is a supported numeric type (RFC-compliant)
template <typename T>
constexpr bool is_network_numeric_v =
    std::is_same_v<T, std::uint16_t> || std::is_same_v<T, std::int16_t> ||
    std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::int32_t> ||
    std::is_same_v<T, float>;

// ============================================================================
// Host to Network (Big-Endian)
// ============================================================================

/**
 * @brief Convert 16-bit value from host to network byte order
 */
[[nodiscard]] inline std::uint16_t toNetwork(std::uint16_t value) noexcept {
    return htons(value);
}

/**
 * @brief Convert signed 16-bit value from host to network byte order
 */
[[nodiscard]] inline std::int16_t toNetwork(std::int16_t value) noexcept {
    std::uint16_t temp;
    std::memcpy(&temp, &value, sizeof(temp));
    temp = htons(temp);
    std::int16_t result;
    std::memcpy(&result, &temp, sizeof(result));
    return result;
}

/**
 * @brief Convert 32-bit value from host to network byte order
 */
[[nodiscard]] inline std::uint32_t toNetwork(std::uint32_t value) noexcept {
    return htonl(value);
}

/**
 * @brief Convert signed 32-bit value from host to network byte order
 */
[[nodiscard]] inline std::int32_t toNetwork(std::int32_t value) noexcept {
    std::uint32_t temp;
    std::memcpy(&temp, &value, sizeof(temp));
    temp = htonl(temp);
    std::int32_t result;
    std::memcpy(&result, &temp, sizeof(result));
    return result;
}

/**
 * @brief Convert 32-bit float from host to network byte order
 *
 * IEEE 754 floats are converted by treating them as uint32_t.
 */
[[nodiscard]] inline float toNetwork(float value) noexcept {
    static_assert(sizeof(float) == sizeof(std::uint32_t),
                  "float must be 32 bits");
    std::uint32_t temp;
    std::memcpy(&temp, &value, sizeof(temp));
    temp = htonl(temp);
    float result;
    std::memcpy(&result, &temp, sizeof(result));
    return result;
}

/**
 * @brief Convert 8-bit value (no-op, but provides consistent API)
 */
[[nodiscard]] inline std::uint8_t toNetwork(std::uint8_t value) noexcept {
    return value;
}

[[nodiscard]] inline std::int8_t toNetwork(std::int8_t value) noexcept {
    return value;
}

// ============================================================================
// Network to Host (From Big-Endian)
// ============================================================================

/**
 * @brief Convert 16-bit value from network to host byte order
 */
[[nodiscard]] inline std::uint16_t fromNetwork(std::uint16_t value) noexcept {
    return ntohs(value);
}

/**
 * @brief Convert signed 16-bit value from network to host byte order
 */
[[nodiscard]] inline std::int16_t fromNetwork(std::int16_t value) noexcept {
    std::uint16_t temp;
    std::memcpy(&temp, &value, sizeof(temp));
    temp = ntohs(temp);
    std::int16_t result;
    std::memcpy(&result, &temp, sizeof(result));
    return result;
}

/**
 * @brief Convert 32-bit value from network to host byte order
 */
[[nodiscard]] inline std::uint32_t fromNetwork(std::uint32_t value) noexcept {
    return ntohl(value);
}

/**
 * @brief Convert signed 32-bit value from network to host byte order
 */
[[nodiscard]] inline std::int32_t fromNetwork(std::int32_t value) noexcept {
    std::uint32_t temp;
    std::memcpy(&temp, &value, sizeof(temp));
    temp = ntohl(temp);
    std::int32_t result;
    std::memcpy(&result, &temp, sizeof(result));
    return result;
}

/**
 * @brief Convert 32-bit float from network to host byte order
 */
[[nodiscard]] inline float fromNetwork(float value) noexcept {
    static_assert(sizeof(float) == sizeof(std::uint32_t),
                  "float must be 32 bits");
    std::uint32_t temp;
    std::memcpy(&temp, &value, sizeof(temp));
    temp = ntohl(temp);
    float result;
    std::memcpy(&result, &temp, sizeof(result));
    return result;
}

/**
 * @brief Convert 8-bit value (no-op, but provides consistent API)
 */
[[nodiscard]] inline std::uint8_t fromNetwork(std::uint8_t value) noexcept {
    return value;
}

[[nodiscard]] inline std::int8_t fromNetwork(std::int8_t value) noexcept {
    return value;
}

// ============================================================================
// Buffer Operations
// ============================================================================

/**
 * @brief Write value to buffer in network byte order
 *
 * @tparam T Numeric type to write
 * @param buffer Destination buffer (must have sizeof(T) bytes available)
 * @param value Value to write
 *
 * @example
 * ```cpp
 * std::array<uint8_t, 4> buf;
 * ByteOrder::writeTo(buf.data(), uint32_t{0x12345678});
 * // buf now contains: [0x12, 0x34, 0x56, 0x78] (big-endian)
 * ```
 */
template <typename T>
inline void writeTo(std::uint8_t* buffer, T value) noexcept {
    static_assert(is_network_numeric_v<T> || sizeof(T) == 1,
                  "Type must be a supported numeric type");
    T networkValue = toNetwork(value);
    std::memcpy(buffer, &networkValue, sizeof(T));
}

/**
 * @brief Read value from buffer in network byte order
 *
 * @tparam T Numeric type to read
 * @param buffer Source buffer (must have sizeof(T) bytes available)
 * @return Value in host byte order
 *
 * @example
 * ```cpp
 * std::array<uint8_t, 4> buf = {0x12, 0x34, 0x56, 0x78};
 * auto value = ByteOrder::readFrom<uint32_t>(buf.data());
 * // value == 0x12345678 on big-endian, 0x78563412 on little-endian? NO!
 * // value == 0x12345678 always (correctly converted)
 * ```
 */
template <typename T>
[[nodiscard]] inline T readFrom(const std::uint8_t* buffer) noexcept {
    static_assert(is_network_numeric_v<T> || sizeof(T) == 1,
                  "Type must be a supported numeric type");
    T networkValue;
    std::memcpy(&networkValue, buffer, sizeof(T));
    return fromNetwork(networkValue);
}

}  // namespace ByteOrder

}  // namespace rtype::network
