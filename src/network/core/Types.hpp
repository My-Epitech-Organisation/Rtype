/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Network Core Types - Fundamental types for network operations
*/

#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace rtype::network {

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

using Endpoint = rtype::Endpoint;

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
