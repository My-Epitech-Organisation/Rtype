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
 * @brief Perform complete validation of a received packet
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

    Header header = ByteOrderSpec::deserializeFromNetwork<Header>(
        data.subspan(0, kHeaderSize));

    auto headerResult = validateHeader(header);
    if (headerResult.isErr()) {
        return headerResult;
    }

    std::size_t payloadSize = data.size() - kHeaderSize;
    auto payload = data.subspan(kHeaderSize);
    auto payloadResult =
        validatePayloadSize(header.getOpCode(), payloadSize, payload);
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

}  // namespace Validator

}  // namespace rtype::network
