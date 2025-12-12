/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Validator - Protocol validation utilities
*/

#pragma once

#include <cstdint>
#include <span>

#include "../core/Error.hpp"
#include "ByteOrderSpec.hpp"
#include "Header.hpp"
#include "OpCode.hpp"
#include "Payloads.hpp"

namespace rtype::network {

/**
 * @brief Protocol validation utilities
 *
 * Provides validation functions for incoming packets as per RFC RTGP v1.0.0
 * Section 6 (Security Considerations).
 */
namespace Validator {

/**
 * @brief Validate the magic byte
 * @param magic The magic byte from the packet
 * @return Success if valid, InvalidMagic error otherwise
 */
[[nodiscard]] inline Result<void> validateMagic(std::uint8_t magic) noexcept {
    if (magic == kMagicByte) {
        return Result<void>::ok();
    }
    return Result<void>::err(NetworkError::InvalidMagic);
}

/**
 * @brief Validate and convert a raw byte to OpCode
 * @param value The raw opcode byte
 * @return The OpCode on success, UnknownOpcode error otherwise
 */
[[nodiscard]] inline Result<OpCode> validateOpCode(
    std::uint8_t value) noexcept {
    if (isValidOpCode(value)) {
        return Result<OpCode>::ok(static_cast<OpCode>(value));
    }
    return Result<OpCode>::err(NetworkError::UnknownOpcode);
}

/**
 * @brief Validate a complete header structure
 * @param header The header to validate
 * @return Success if valid, appropriate error otherwise
 */
[[nodiscard]] inline Result<void> validateHeader(
    const Header& header) noexcept {
    if (!header.hasValidMagic()) {
        return Result<void>::err(NetworkError::InvalidMagic);
    }

    if (!header.hasValidOpCode()) {
        return Result<void>::err(NetworkError::UnknownOpcode);
    }

    if (!header.hasValidReserved()) {
        return Result<void>::err(NetworkError::MalformedPacket);
    }

    return Result<void>::ok();
}

/**
 * @brief Validate packet size against minimum requirements
 * @param size The total received packet size (header + payload)
 * @return Success if size is valid, appropriate error otherwise
 */
[[nodiscard]] inline Result<void> validatePacketSize(
    std::size_t size) noexcept {
    if (size < kHeaderSize) {
        return Result<void>::err(NetworkError::PacketTooSmall);
    }
    if (size > kMaxPacketSize) {
        return Result<void>::err(NetworkError::PacketTooLarge);
    }
    return Result<void>::ok();
}

/**
 * @brief Validate payload size for a specific OpCode
 * @param opcode The operation code
 * @param payloadSize The received payload size
 * @param payload Span view of the payload buffer (required for R_GET_USERS)
 * @return Success if size matches expected, MalformedPacket otherwise
 */
[[nodiscard]] inline Result<void> validatePayloadSize(
    OpCode opcode, std::size_t payloadSize,
    std::span<const std::uint8_t> payload = {}) noexcept {
    if (hasVariablePayload(opcode)) {
        if (opcode == OpCode::R_GET_USERS) {
            if (payloadSize < 1) {
                return Result<void>::err(NetworkError::PacketTooSmall);
            }
            if (payload.empty()) {
                return Result<void>::err(NetworkError::MalformedPacket);
            }
            std::uint8_t count = payload[0];
            if (count > kMaxUsersInResponse) {
                return Result<void>::err(NetworkError::MalformedPacket);
            }
            std::size_t expected =
                1 + (static_cast<std::size_t>(count) * sizeof(std::uint32_t));
            if (payloadSize != expected) {
                return Result<void>::err(NetworkError::MalformedPacket);
            }
        }
        return Result<void>::ok();
    }

    std::size_t expected = getPayloadSize(opcode);
    if (payloadSize != expected) {
        return Result<void>::err(NetworkError::MalformedPacket);
    }

    return Result<void>::ok();
}

/**
 * @brief Validate R_GET_USERS payload content
 * @param payload Span view of the payload data
 * @return Success if valid, MalformedPacket otherwise
 */
[[nodiscard]] inline Result<void> validateRGetUsersPayload(
    std::span<const std::uint8_t> payload) noexcept {
    if (payload.size() < 1) {
        return Result<void>::err(NetworkError::PacketTooSmall);
    }

    std::uint8_t count = payload[0];
    if (count > kMaxUsersInResponse) {
        return Result<void>::err(NetworkError::MalformedPacket);
    }

    std::size_t expectedSize = 1 + static_cast<std::size_t>(count) * 4;
    if (payload.size() != expectedSize) {
        return Result<void>::err(NetworkError::MalformedPacket);
    }

    return Result<void>::ok();
}

/**
 * @brief Validate User ID for client-originated packets
 * @param userId The user ID from the packet
 * @param opcode The operation code (affects validation rules)
 * @return Success if valid, InvalidUserId error otherwise
 */
[[nodiscard]] inline Result<void> validateClientUserId(std::uint32_t userId,
                                                       OpCode opcode) noexcept {
    if (opcode == OpCode::C_CONNECT) {
        if (userId == kUnassignedUserId) {
            return Result<void>::ok();
        }
        return Result<void>::err(NetworkError::InvalidUserId);
    }

    if (userId == kServerUserId) {
        return Result<void>::err(NetworkError::InvalidUserId);
    }

    if (userId >= kMinClientUserId && userId <= kMaxClientUserId) {
        return Result<void>::ok();
    }

    if (userId == kUnassignedUserId) {
        return Result<void>::err(NetworkError::InvalidUserId);
    }

    return Result<void>::err(NetworkError::InvalidUserId);
}

/**
 * @brief Validate User ID for server-originated packets
 * @param userId The user ID from the packet
 * @return Success if valid (must be kServerUserId)
 */
[[nodiscard]] inline Result<void> validateServerUserId(
    std::uint32_t userId) noexcept {
    if (userId == kServerUserId) {
        return Result<void>::ok();
    }
    return Result<void>::err(NetworkError::InvalidUserId);
}

/**
 * @brief Validate payload size against maximum to prevent buffer overflow
 * @param payloadSize The payload size from the header
 * @return Success if within limits, PacketTooLarge otherwise
 */
[[nodiscard]] inline Result<void> validatePayloadMaxSize(
    std::uint16_t payloadSize) noexcept {
    if (payloadSize > kMaxPayloadSize) {
        return Result<void>::err(NetworkError::PacketTooLarge);
    }
    return Result<void>::ok();
}

/**
 * @brief Validate bounds before deserializing from buffer
 * @param buffer The buffer to check
 * @param offset Starting offset in the buffer
 * @param size Required number of bytes
 * @return Success if bounds are valid, MalformedPacket otherwise
 */
[[nodiscard]] inline Result<void> validateBufferBounds(
    std::span<const std::uint8_t> buffer, std::size_t offset,
    std::size_t size) noexcept {
    if (offset + size > buffer.size()) {
        return Result<void>::err(NetworkError::MalformedPacket);
    }
    return Result<void>::ok();
}

/**
 * @brief Perform complete validation of a received packet
 *
 * Implements the validation pipeline from RFC RTGP v1.1.0 Section 6:
 * 1. Size >= 16 bytes (minimum header size)
 * 2. Magic byte == 0xA1
 * 3. Payload size <= MAX_PAYLOAD (4096 bytes)
 * 4. Valid OpCode
 * 5. Reserved bytes == 0
 * 6. Payload size matches expected for OpCode
 * 7. UserID validation (server/client authority check)
 *
 * @param data Span view of the raw packet data
 * @param isFromServer Whether this packet claims to be from the server
 * @return Success if valid, first encountered error otherwise
 */
[[nodiscard]] inline Result<void> validatePacket(
    std::span<const std::uint8_t> data, bool isFromServer) noexcept {
    auto sizeResult = validatePacketSize(data.size());
    if (sizeResult.isErr()) {
        return sizeResult;
    }

    auto boundsResult = validateBufferBounds(data, 0, kHeaderSize);
    if (boundsResult.isErr()) {
        return boundsResult;
    }

    Header header = ByteOrderSpec::deserializeFromNetwork<Header>(
        data.subspan(0, kHeaderSize));

    if (!header.hasValidMagic()) {
        return Result<void>::err(NetworkError::InvalidMagic);
    }

    auto maxSizeResult = validatePayloadMaxSize(header.payloadSize);
    if (maxSizeResult.isErr()) {
        return maxSizeResult;
    }

    auto headerResult = validateHeader(header);
    if (headerResult.isErr()) {
        return headerResult;
    }

    std::size_t actualPayloadSize = data.size() - kHeaderSize;
    if (actualPayloadSize != header.payloadSize) {
        return Result<void>::err(NetworkError::MalformedPacket);
    }

    auto payload = data.subspan(kHeaderSize);
    auto payloadResult =
        validatePayloadSize(header.getOpCode(), header.payloadSize, payload);
    if (payloadResult.isErr()) {
        return payloadResult;
    }

    if (header.getOpCode() == OpCode::R_GET_USERS) {
        auto rGetUsersResult = validateRGetUsersPayload(payload);
        if (rGetUsersResult.isErr()) {
            return rGetUsersResult;
        }
    }

    if (isFromServer) {
        auto userResult = validateServerUserId(header.userId);
        if (userResult.isErr()) {
            return userResult;
        }
    } else {
        auto userResult =
            validateClientUserId(header.userId, header.getOpCode());
        if (userResult.isErr()) {
            return userResult;
        }
    }

    return Result<void>::ok();
}

/**
 * @brief Safely deserialize with bounds checking
 *
 * Validates buffer bounds before attempting to deserialize.
 * Prevents buffer overruns when reading from untrusted network data.
 *
 * @tparam T Type to deserialize
 * @param buffer The buffer containing network data
 * @param offset Starting offset in the buffer
 * @return Result containing deserialized value or error
 */
template <typename T>
[[nodiscard]] inline Result<T> safeDeserialize(
    std::span<const std::uint8_t> buffer, std::size_t offset = 0) noexcept {
    static_assert(
        std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>,
        "T must be trivially copyable and standard layout");

    auto boundsResult = validateBufferBounds(buffer, offset, sizeof(T));
    if (boundsResult.isErr()) {
        return Result<T>::err(boundsResult.error());
    }

    try {
        T value = ByteOrderSpec::deserializeFromNetwork<T>(
            buffer.subspan(offset, sizeof(T)));
        return Result<T>::ok(value);
    } catch (...) {
        return Result<T>::err(NetworkError::MalformedPacket);
    }
}

}  // namespace Validator

}  // namespace rtype::network
