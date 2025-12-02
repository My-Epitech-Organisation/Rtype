/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Network Core - Aggregate header for all core types
*/

#pragma once

/**
 * @file Core.hpp
 * @brief Include all network core types
 *
 * This header provides convenient access to all fundamental network types:
 * - Buffer, Endpoint, and constants (Types.hpp)
 * - Byte order conversion utilities (ByteOrder.hpp)
 * - Error handling with Result<T> (Error.hpp)
 *
 * @example
 * ```cpp
 * #include "core/Core.hpp"
 *
 * using namespace rtype::network;
 *
 * Result<size_t> send(const Buffer& data, const Endpoint& dest) {
 *     if (!dest.isValid()) {
 *         return Err<size_t>(NetworkError::NotConnected);
 *     }
 *     // ... send logic ...
 *     return Ok(data.size());
 * }
 * ```
 */

#include "core/ByteOrder.hpp"
#include "core/Error.hpp"
#include "core/Types.hpp"
