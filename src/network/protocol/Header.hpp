/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Header - RTGP Protocol Header (16 bytes) as per RFC RTGP v1.1.0
*/

#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

#include "OpCode.hpp"

namespace rtype::network {

/// Dynamic buffer for network data
using Buffer = std::vector<std::uint8_t>;

/// Magic byte for packet validation (RFC Section 4.1)
inline constexpr std::uint8_t kMagicByte = 0xA1;

/// Header size in bytes (RFC Section 4.1)
inline constexpr std::size_t kHeaderSize = 16;

/// Maximum packet size to avoid IP fragmentation (RFC Section 3)
inline constexpr std::size_t kMaxPacketSize = 1400;

/// Maximum payload size (packet size minus header)
inline constexpr std::size_t kMaxPayloadSize = kMaxPacketSize - kHeaderSize;

/// Default server port (RFC Section 3)
inline constexpr std::uint16_t kDefaultPort = 4242;

/// Server authority User ID (RFC Section 4.2)
inline constexpr std::uint32_t kServerUserId = 0xFFFFFFFF;

/// Unassigned client User ID during handshake (RFC Section 4.2)
inline constexpr std::uint32_t kUnassignedUserId = 0x00000000;

/// Minimum valid assigned client ID
inline constexpr std::uint32_t kMinClientUserId = 0x00000001;

/// Maximum valid assigned client ID
inline constexpr std::uint32_t kMaxClientUserId = 0xFFFFFFFE;

namespace Flags {
/// No special flags (unreliable packet)
inline constexpr std::uint8_t kNone = 0x00;

/// Sender requests acknowledgement (RUDP)
inline constexpr std::uint8_t kReliable = 0x01;

/// Ack ID field is valid (acknowledging a previous packet)
inline constexpr std::uint8_t kIsAck = 0x02;
}  // namespace Flags

#pragma pack(push, 1)

/**
 * @brief RTGP Protocol Header - 16 bytes, network byte order
 *
 * Layout as per RFC RTGP v1.1.0 Section 4.1:
 * ```
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Magic Byte   |  Packet Type  |          Packet Size          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                            User ID                            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |          Sequence ID          |            Ack ID             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Flags     |                   Reserved                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ```
 *
 * @note All multi-byte fields are stored in NETWORK BYTE ORDER (Big-Endian)
 *       Use the accessor methods which handle byte order conversion.
 */
struct Header {
    std::uint8_t magic;                    ///< Must be 0xA1 (kMagicByte)
    std::uint8_t opcode;                   ///< Operation code (OpCode enum)
    std::uint16_t payloadSize;             ///< Payload size (excludes header)
    std::uint32_t userId;                  ///< Sender's unique ID
    std::uint16_t seqId;                   ///< Sequence number (wraps at 65535)
    std::uint16_t ackId;                   ///< Last received sequence ID
    std::uint8_t flags;                    ///< Reliability flags
    std::array<std::uint8_t, 3> reserved;  ///< Padding (must be 0)

    /**
     * @brief Create a new header with default values
     * @param op The operation code
     * @param user The sender's user ID
     * @param seq The sequence number
     * @param payload The payload size in bytes
     * @return Initialized header (in HOST byte order - must be converted)
     */
    [[nodiscard]] static Header create(OpCode op, std::uint32_t user,
                                       std::uint16_t seq,
                                       std::uint16_t payload = 0) noexcept {
        Header h{};
        h.magic = kMagicByte;
        h.opcode = static_cast<std::uint8_t>(op);
        h.payloadSize = payload;
        h.userId = user;
        h.seqId = seq;
        h.ackId = 0;
        h.flags =
            rtype::network::isReliable(op) ? Flags::kReliable : Flags::kNone;
        h.reserved = {0, 0, 0};
        return h;
    }

    /**
     * @brief Create a header for server-originated packets
     */
    [[nodiscard]] static Header createServer(
        OpCode op, std::uint16_t seq, std::uint16_t payload = 0) noexcept {
        return create(op, kServerUserId, seq, payload);
    }

    /**
     * @brief Create a header for client connection request
     */
    [[nodiscard]] static Header createConnect(std::uint16_t seq) noexcept {
        return create(OpCode::C_CONNECT, kUnassignedUserId, seq, 0);
    }

    /// Check if the RELIABLE flag is set
    [[nodiscard]] constexpr bool isReliable() const noexcept {
        return (flags & Flags::kReliable) != 0;
    }

    /// Check if the IS_ACK flag is set
    [[nodiscard]] constexpr bool isAck() const noexcept {
        return (flags & Flags::kIsAck) != 0;
    }

    /// Set the RELIABLE flag
    constexpr void setReliable(bool value = true) noexcept {
        if (value) {
            flags |= Flags::kReliable;
        } else {
            flags &= ~Flags::kReliable;
        }
    }

    /// Set the IS_ACK flag and the ack ID
    constexpr void setAck(std::uint16_t ackSeqId) noexcept {
        flags |= Flags::kIsAck;
        ackId = ackSeqId;
    }

    /// Check if the magic byte is valid
    [[nodiscard]] constexpr bool hasValidMagic() const noexcept {
        return magic == kMagicByte;
    }

    /// Check if the opcode is a known value
    [[nodiscard]] constexpr bool hasValidOpCode() const noexcept {
        return isValidOpCode(opcode);
    }

    /// Get the OpCode enum value
    [[nodiscard]] constexpr OpCode getOpCode() const noexcept {
        return static_cast<OpCode>(opcode);
    }

    /// Check if reserved bytes are zero (as required by RFC)
    [[nodiscard]] constexpr bool hasValidReserved() const noexcept {
        return reserved[0] == 0 && reserved[1] == 0 && reserved[2] == 0;
    }

    /// Perform full header validation
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return hasValidMagic() && hasValidOpCode() && hasValidReserved();
    }

    /// Check if this packet is from the server
    [[nodiscard]] constexpr bool isFromServer() const noexcept {
        return userId == kServerUserId;
    }

    /// Check if this is from an unassigned client (handshake)
    [[nodiscard]] constexpr bool isFromUnassigned() const noexcept {
        return userId == kUnassignedUserId;
    }

    /// Check if the user ID is in valid client range
    [[nodiscard]] constexpr bool hasValidClientId() const noexcept {
        return userId >= kMinClientUserId && userId <= kMaxClientUserId;
    }
};

#pragma pack(pop)

// Compile-time verification of struct size
static_assert(sizeof(Header) == kHeaderSize,
              "Header must be exactly 16 bytes as per RFC RTGP v1.1.0");

// Verify Header is trivially copyable for network operations
static_assert(std::is_trivially_copyable_v<Header>,
              "Header must be trivially copyable for memcpy/network ops");

// Verify Header is standard layout for safe casting
static_assert(std::is_standard_layout_v<Header>,
              "Header must be standard layout for safe buffer casting");

}  // namespace rtype::network
