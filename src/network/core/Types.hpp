/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Network Core Types - Fundamental types for network operations
*/

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace rtype::network {

// ============================================================================
// Buffer Types
// ============================================================================

/**
 * @brief Dynamic buffer for network data
 *
 * Used for variable-length packets and serialization.
 */
using Buffer = std::vector<std::uint8_t>;

/**
 * @brief Fixed-size buffer matching MTU safety limit
 *
 * Used for receive operations to avoid IP fragmentation.
 * Size chosen to stay well under typical MTU (1500 bytes).
 */
static constexpr std::size_t kMaxPacketSize = 1400;
using FixedBuffer = std::array<std::uint8_t, kMaxPacketSize>;

// ============================================================================
// Network Constants (from RFC RTGP v1.1.0)
// ============================================================================

/// Default server port
static constexpr std::uint16_t kDefaultPort = 4242;

/// Magic byte for packet validation (RFC Section 4.1)
static constexpr std::uint8_t kMagicByte = 0xA1;

/// Header size in bytes (RFC Section 4.1)
static constexpr std::size_t kHeaderSize = 16;

/// Server authority User ID (RFC Section 4.2)
static constexpr std::uint32_t kServerUserId = 0xFFFFFFFF;

/// Unassigned client User ID (RFC Section 4.2)
static constexpr std::uint32_t kUnassignedUserId = 0x00000000;

// ============================================================================
// Endpoint
// ============================================================================

/**
 * @brief Network endpoint (IP address + port)
 */
struct Endpoint {
    std::string address;  ///< IP address (IPv4 or IPv6 string)
    std::uint16_t port;   ///< Port number

    Endpoint() = default;

    Endpoint(std::string addr, std::uint16_t p)
        : address(std::move(addr)), port(p) {}

    /**
     * @brief Check if endpoint is valid (non-empty address and non-zero port)
     */
    [[nodiscard]] bool isValid() const noexcept {
        return !address.empty() && port != 0;
    }

    /**
     * @brief Equality comparison
     */
    bool operator==(const Endpoint& other) const noexcept {
        return address == other.address && port == other.port;
    }

    bool operator!=(const Endpoint& other) const noexcept {
        return !(*this == other);
    }

    /**
     * @brief String representation for logging
     */
    [[nodiscard]] std::string toString() const {
        return address + ":" + std::to_string(port);
    }
};

// ============================================================================
// Reliability Flags (RFC Section 4.3)
// ============================================================================

/**
 * @brief Packet reliability flags as per RFC RTGP v1.1.0
 */
namespace Flags {
/// No special flags (unreliable packet)
static constexpr std::uint8_t kNone = 0x00;

/// Sender requests acknowledgement (RUDP)
static constexpr std::uint8_t kReliable = 0x01;

/// Ack ID field is valid (acknowledging a previous packet)
static constexpr std::uint8_t kIsAck = 0x02;
}  // namespace Flags

}  // namespace rtype::network
