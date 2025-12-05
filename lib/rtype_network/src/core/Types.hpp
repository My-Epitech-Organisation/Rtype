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

#include "../../../rtype_common/src/Types.hpp"
#include "protocol/Header.hpp"

namespace rtype::network {

/**
 * @brief Dynamic buffer for network data
 *
 * Used for variable-length packets and serialization.
 */
using Buffer = std::vector<std::uint8_t>;

/**
 * @brief Fixed-size buffer for receive operations
 *
 * Size chosen to stay well under typical MTU (1500 bytes).
 * Matches kMaxPacketSize defined in protocol/Header.hpp.
 */
using FixedBuffer = std::array<std::uint8_t, kMaxPacketSize>;

using Endpoint = rtype::Endpoint;

}  // namespace rtype::network
