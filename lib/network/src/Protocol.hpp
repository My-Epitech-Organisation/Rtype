/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Protocol - Aggregate header for all protocol types
*/

#pragma once

/**
 * @file Protocol.hpp
 * @brief Include all RTGP protocol types
 *
 * This header provides a single include for the complete RTGP protocol
 * definition as per RFC RTGP v1.1.0.
 *
 * Includes:
 * - OpCode: Protocol operation codes (RFC Section 5)
 * - Header: 16-byte packet header (RFC Section 4)
 * - Payloads: All payload structures
 * - Validator: Packet validation utilities (RFC Section 6)
 * - SecurityContext: Anti-replay protection and security state
 *
 * @note Serialization logic (PacketWriter, PacketReader, Packet class)
 *       is implemented separately in issue #161.
 *
 * @see RFC RTGP v1.2.0 in docs/RFC/RFC_RTGP_v1.2.0.md
 */

#include "protocol/Header.hpp"
#include "protocol/OpCode.hpp"
#include "protocol/Payloads.hpp"
#include "protocol/SecurityContext.hpp"
#include "protocol/Validator.hpp"
