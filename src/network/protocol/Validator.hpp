/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Validator - Protocol validation utilities
*/

#pragma once

#include <cstdint>

#include "../core/Error.hpp"
#include "Header.hpp"
#include "OpCode.hpp"
#include "Payloads.hpp"

namespace rtype::network {

/**
 * @brief Protocol validation utilities
 *
 * Provides validation functions for incoming packets as per RFC RTGP v1.1.0
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
    // Check magic byte
    if (!header.hasValidMagic()) {
        return Result<void>::err(NetworkError::InvalidMagic);
    }

    // Check opcode
    if (!header.hasValidOpCode()) {
        return Result<void>::err(NetworkError::UnknownOpcode);
    }

    // Check reserved bytes
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
 * @return Success if size matches expected, MalformedPacket otherwise
 */
[[nodiscard]] inline Result<void> validatePayloadSize(
    OpCode opcode, std::size_t payloadSize) noexcept {
    if (hasVariablePayload(opcode)) {
        if (opcode == OpCode::R_GET_USERS && payloadSize < 1) {
            return Result<void>::err(NetworkError::PacketTooSmall);
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
 * @param data Pointer to the raw packet data
 * @param size Size of the received data
 * @param isFromServer Whether this packet claims to be from the server
 * @return Success if valid, first encountered error otherwise
 */
[[nodiscard]] inline Result<void> validatePacket(const std::uint8_t* data,
                                                 std::size_t size,
                                                 bool isFromServer) noexcept {
    auto sizeResult = validatePacketSize(size);
    if (sizeResult.isErr()) {
        return sizeResult;
    }

    const auto* header = reinterpret_cast<const Header*>(data);

    auto headerResult = validateHeader(*header);
    if (headerResult.isErr()) {
        return headerResult;
    }

    std::size_t payloadSize = size - kHeaderSize;
    auto payloadResult = validatePayloadSize(header->getOpCode(), payloadSize);
    if (payloadResult.isErr()) {
        return payloadResult;
    }

    if (isFromServer) {
        auto userResult = validateServerUserId(header->userId);
        if (userResult.isErr()) {
            return userResult;
        }
    } else {
        auto userResult =
            validateClientUserId(header->userId, header->getOpCode());
        if (userResult.isErr()) {
            return userResult;
        }
    }

    return Result<void>::ok();
}

}  // namespace Validator

}  // namespace rtype::network
