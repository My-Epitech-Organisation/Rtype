/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Protocol - Public header for RTGP protocol types
*/

#pragma once

/**
 * @file Protocol.hpp
 * @brief Public interface for RTGP protocol as per RFC RTGP v1.0.0
 *
 * This header re-exports all protocol type definitions for external use.
 * For internal network library code, include the src/ headers directly.
 *
 * @see docs/RFC/RFC_RTGP_v1.0.0.md for protocol specification
 *
 * Provides:
 * - OpCode: All protocol operation codes (RFC Section 5)
 * - Header: 16-byte packet header structure (RFC Section 4)
 * - Payloads: POD structures for each OpCode payload
 * - Validator: Validation utilities (RFC Section 6)
 *
 * @note Serialization logic (serialize/deserialize) is provided separately.
 *
 * Example usage:
 * @code
 * #include <rtype/network/Protocol.hpp>
 *
 * using namespace rtype::network;
 *
 * // Create a header
 * Header h = Header::create(OpCode::C_INPUT, userId, seqId, sizeof(InputPayload));
 *
 * // Validate incoming data
 * if (Validator::validateMagic(data[0]).isOk()) {
 *     // Process packet
 * }
 * @endcode
 */

// Include all protocol components from src/network/
// These paths work because CMakeLists.txt adds src/network to include paths

// Core protocol types (RFC RTGP v1.0.0)
#include "protocol/OpCode.hpp"
#include "protocol/Header.hpp"
#include "protocol/Payloads.hpp"
#include "protocol/Validator.hpp"

// Core utilities
#include "core/Error.hpp"
#include "core/Types.hpp"
#include "core/ByteOrder.hpp"
