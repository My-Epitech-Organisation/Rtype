/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ByteOrderSpec - RFC-compliant byte order specializations for protocol types
*/

#pragma once

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#include "../core/ByteOrder.hpp"
#include "Header.hpp"
#include "Payloads.hpp"

namespace rtype::network {

namespace ByteOrderSpec {

[[nodiscard]] inline std::uint8_t toNetwork(std::uint8_t v) noexcept {
    return v;
}
[[nodiscard]] inline std::uint8_t fromNetwork(std::uint8_t v) noexcept {
    return v;
}

[[nodiscard]] inline std::int8_t toNetwork(std::int8_t v) noexcept { return v; }
[[nodiscard]] inline std::int8_t fromNetwork(std::int8_t v) noexcept {
    return v;
}

[[nodiscard]] inline std::uint16_t toNetwork(std::uint16_t v) noexcept {
    return ByteOrder::toNetwork(v);
}
[[nodiscard]] inline std::uint16_t fromNetwork(std::uint16_t v) noexcept {
    return ByteOrder::fromNetwork(v);
}

[[nodiscard]] inline std::int16_t toNetwork(std::int16_t v) noexcept {
    return ByteOrder::toNetwork(v);
}
[[nodiscard]] inline std::int16_t fromNetwork(std::int16_t v) noexcept {
    return ByteOrder::fromNetwork(v);
}

[[nodiscard]] inline std::uint32_t toNetwork(std::uint32_t v) noexcept {
    return ByteOrder::toNetwork(v);
}
[[nodiscard]] inline std::uint32_t fromNetwork(std::uint32_t v) noexcept {
    return ByteOrder::fromNetwork(v);
}

[[nodiscard]] inline std::int32_t toNetwork(std::int32_t v) noexcept {
    return ByteOrder::toNetwork(v);
}
[[nodiscard]] inline std::int32_t fromNetwork(std::int32_t v) noexcept {
    return ByteOrder::fromNetwork(v);
}

[[nodiscard]] inline float toNetwork(float v) noexcept {
    return ByteOrder::toNetwork(v);
}
[[nodiscard]] inline float fromNetwork(float v) noexcept {
    return ByteOrder::fromNetwork(v);
}

/// Type trait to detect RFC protocol types with explicit specializations
template <typename T>
struct is_rfc_type : std::false_type {};

// Specialize for RFC types
template <>
struct is_rfc_type<Header> : std::true_type {};
template <>
struct is_rfc_type<AcceptPayload> : std::true_type {};
template <>
struct is_rfc_type<EntitySpawnPayload> : std::true_type {};
template <>
struct is_rfc_type<EntityMovePayload> : std::true_type {};
template <>
struct is_rfc_type<EntityDestroyPayload> : std::true_type {};
template <>
struct is_rfc_type<UpdatePosPayload> : std::true_type {};
template <>
struct is_rfc_type<UpdateStatePayload> : std::true_type {};
template <>
struct is_rfc_type<InputPayload> : std::true_type {};
template <>
struct is_rfc_type<ConnectPayload> : std::true_type {};
template <>
struct is_rfc_type<DisconnectPayload> : std::true_type {};
template <>
struct is_rfc_type<PingPayload> : std::true_type {};
template <>
struct is_rfc_type<PongPayload> : std::true_type {};

template <typename T>
inline constexpr bool is_rfc_type_v = is_rfc_type<T>::value;

/**
 * @brief Generic toNetwork for non-RFC trivially copyable types
 *
 * Uses field-agnostic byte swapping (4-byte chunks, then 2-byte).
 * Only use for testing or types with uniform field sizes.
 */
template <typename T>
[[nodiscard]] inline std::enable_if_t<!is_rfc_type_v<T> &&
                                          std::is_trivially_copyable_v<T> &&
                                          !ByteOrder::is_network_numeric_v<T>,
                                      T>
toNetwork(const T& data) noexcept {
    T result;
    std::uint8_t buffer[sizeof(T)];
    std::memcpy(buffer, &data, sizeof(T));

    // Convert 4-byte chunks
    std::size_t offset = 0;
    while (offset + 4 <= sizeof(T)) {
        std::uint32_t val;
        std::memcpy(&val, buffer + offset, 4);
        val = ByteOrder::toNetwork(val);
        std::memcpy(buffer + offset, &val, 4);
        offset += 4;
    }
    // Convert 2-byte chunks
    while (offset + 2 <= sizeof(T)) {
        std::uint16_t val;
        std::memcpy(&val, buffer + offset, 2);
        val = ByteOrder::toNetwork(val);
        std::memcpy(buffer + offset, &val, 2);
        offset += 2;
    }

    std::memcpy(&result, buffer, sizeof(T));
    return result;
}

/**
 * @brief Generic fromNetwork for non-RFC trivially copyable types
 */
template <typename T>
[[nodiscard]] inline std::enable_if_t<!is_rfc_type_v<T> &&
                                          std::is_trivially_copyable_v<T> &&
                                          !ByteOrder::is_network_numeric_v<T>,
                                      T>
fromNetwork(const T& data) noexcept {
    T result;
    std::uint8_t buffer[sizeof(T)];
    std::memcpy(buffer, &data, sizeof(T));

    // Convert 4-byte chunks
    std::size_t offset = 0;
    while (offset + 4 <= sizeof(T)) {
        std::uint32_t val;
        std::memcpy(&val, buffer + offset, 4);
        val = ByteOrder::fromNetwork(val);
        std::memcpy(buffer + offset, &val, 4);
        offset += 4;
    }
    // Convert 2-byte chunks
    while (offset + 2 <= sizeof(T)) {
        std::uint16_t val;
        std::memcpy(&val, buffer + offset, 2);
        val = ByteOrder::fromNetwork(val);
        std::memcpy(buffer + offset, &val, 2);
        offset += 2;
    }

    std::memcpy(&result, buffer, sizeof(T));
    return result;
}

/**
 * @brief Convert Header to network byte order (Big-Endian)
 */
[[nodiscard]] inline Header toNetwork(const Header& h) noexcept {
    Header result = h;
    result.payloadSize = ByteOrder::toNetwork(h.payloadSize);
    result.userId = ByteOrder::toNetwork(h.userId);
    result.seqId = ByteOrder::toNetwork(h.seqId);
    result.ackId = ByteOrder::toNetwork(h.ackId);
    return result;
}

/**
 * @brief Convert Header from network byte order to host order
 */
[[nodiscard]] inline Header fromNetwork(const Header& h) noexcept {
    Header result = h;
    result.payloadSize = ByteOrder::fromNetwork(h.payloadSize);
    result.userId = ByteOrder::fromNetwork(h.userId);
    result.seqId = ByteOrder::fromNetwork(h.seqId);
    result.ackId = ByteOrder::fromNetwork(h.ackId);
    return result;
}

[[nodiscard]] inline AcceptPayload toNetwork(const AcceptPayload& p) noexcept {
    AcceptPayload result;
    result.newUserId = ByteOrder::toNetwork(p.newUserId);
    return result;
}

[[nodiscard]] inline AcceptPayload fromNetwork(
    const AcceptPayload& p) noexcept {
    AcceptPayload result;
    result.newUserId = ByteOrder::fromNetwork(p.newUserId);
    return result;
}

[[nodiscard]] inline EntitySpawnPayload toNetwork(
    const EntitySpawnPayload& p) noexcept {
    EntitySpawnPayload result;
    result.entityId = ByteOrder::toNetwork(p.entityId);
    result.type = p.type;
    result.posX = ByteOrder::toNetwork(p.posX);
    result.posY = ByteOrder::toNetwork(p.posY);
    return result;
}

[[nodiscard]] inline EntitySpawnPayload fromNetwork(
    const EntitySpawnPayload& p) noexcept {
    EntitySpawnPayload result;
    result.entityId = ByteOrder::fromNetwork(p.entityId);
    result.type = p.type;
    result.posX = ByteOrder::fromNetwork(p.posX);
    result.posY = ByteOrder::fromNetwork(p.posY);
    return result;
}

[[nodiscard]] inline EntityMovePayload toNetwork(
    const EntityMovePayload& p) noexcept {
    EntityMovePayload result;
    result.entityId = ByteOrder::toNetwork(p.entityId);
    result.posX = ByteOrder::toNetwork(p.posX);
    result.posY = ByteOrder::toNetwork(p.posY);
    result.velX = ByteOrder::toNetwork(p.velX);
    result.velY = ByteOrder::toNetwork(p.velY);
    return result;
}

[[nodiscard]] inline EntityMovePayload fromNetwork(
    const EntityMovePayload& p) noexcept {
    EntityMovePayload result;
    result.entityId = ByteOrder::fromNetwork(p.entityId);
    result.posX = ByteOrder::fromNetwork(p.posX);
    result.posY = ByteOrder::fromNetwork(p.posY);
    result.velX = ByteOrder::fromNetwork(p.velX);
    result.velY = ByteOrder::fromNetwork(p.velY);
    return result;
}

[[nodiscard]] inline EntityDestroyPayload toNetwork(
    const EntityDestroyPayload& p) noexcept {
    EntityDestroyPayload result;
    result.entityId = ByteOrder::toNetwork(p.entityId);
    return result;
}

[[nodiscard]] inline EntityDestroyPayload fromNetwork(
    const EntityDestroyPayload& p) noexcept {
    EntityDestroyPayload result;
    result.entityId = ByteOrder::fromNetwork(p.entityId);
    return result;
}

[[nodiscard]] inline UpdatePosPayload toNetwork(
    const UpdatePosPayload& p) noexcept {
    UpdatePosPayload result;
    result.posX = ByteOrder::toNetwork(p.posX);
    result.posY = ByteOrder::toNetwork(p.posY);
    return result;
}

[[nodiscard]] inline UpdatePosPayload fromNetwork(
    const UpdatePosPayload& p) noexcept {
    UpdatePosPayload result;
    result.posX = ByteOrder::fromNetwork(p.posX);
    result.posY = ByteOrder::fromNetwork(p.posY);
    return result;
}

[[nodiscard]] inline UpdateStatePayload toNetwork(
    const UpdateStatePayload& p) noexcept {
    return p;
}

[[nodiscard]] inline UpdateStatePayload fromNetwork(
    const UpdateStatePayload& p) noexcept {
    return p;
}

[[nodiscard]] inline InputPayload toNetwork(const InputPayload& p) noexcept {
    return p;
}

[[nodiscard]] inline InputPayload fromNetwork(const InputPayload& p) noexcept {
    return p;
}

[[nodiscard]] inline ConnectPayload toNetwork(
    const ConnectPayload& p) noexcept {
    return p;
}
[[nodiscard]] inline ConnectPayload fromNetwork(
    const ConnectPayload& p) noexcept {
    return p;
}

[[nodiscard]] inline DisconnectPayload toNetwork(
    const DisconnectPayload& p) noexcept {
    return p;
}
[[nodiscard]] inline DisconnectPayload fromNetwork(
    const DisconnectPayload& p) noexcept {
    return p;
}

[[nodiscard]] inline PingPayload toNetwork(const PingPayload& p) noexcept {
    return p;
}
[[nodiscard]] inline PingPayload fromNetwork(const PingPayload& p) noexcept {
    return p;
}

[[nodiscard]] inline PongPayload toNetwork(const PongPayload& p) noexcept {
    return p;
}
[[nodiscard]] inline PongPayload fromNetwork(const PongPayload& p) noexcept {
    return p;
}

/**
 * @brief Serialize a type to network byte order buffer
 *
 * This is the recommended way to serialize RFC types for network transmission.
 *
 * @tparam T RFC protocol type (Header or Payload)
 * @param data The data to serialize
 * @return Buffer in network byte order, ready to send
 */
template <typename T>
[[nodiscard]] inline std::vector<std::uint8_t> serializeToNetwork(
    const T& data) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");
    T networkOrder = toNetwork(data);
    std::vector<std::uint8_t> buffer(sizeof(T));
    std::memcpy(buffer.data(), &networkOrder, sizeof(T));
    return buffer;
}

/**
 * @brief Deserialize a buffer from network byte order
 *
 * @tparam T RFC protocol type (Header or Payload)
 * @param buffer Buffer in network byte order
 * @return Deserialized data in host byte order
 * @throws std::runtime_error if buffer size is incorrect
 */
template <typename T>
[[nodiscard]] inline T deserializeFromNetwork(
    const std::vector<std::uint8_t>& buffer) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");
    if (buffer.size() < sizeof(T)) {
        throw std::runtime_error(
            "Buffer too small for deserialization: expected " +
            std::to_string(sizeof(T)) + " bytes, got " +
            std::to_string(buffer.size()));
    }
    T networkOrder;
    std::memcpy(&networkOrder, buffer.data(), sizeof(T));
    return fromNetwork(networkOrder);
}

/**
 * @brief Deserialize from raw pointer (for parsing received packets)
 */
template <typename T>
[[nodiscard]] inline T deserializeFromNetwork(const std::uint8_t* data,
                                              std::size_t size) {
    static_assert(std::is_trivially_copyable_v<T>,
                  "T must be trivially copyable");
    if (size < sizeof(T)) {
        throw std::runtime_error(
            "Buffer too small for deserialization: expected " +
            std::to_string(sizeof(T)) + " bytes, got " + std::to_string(size));
    }
    T networkOrder;
    std::memcpy(&networkOrder, data, sizeof(T));
    return fromNetwork(networkOrder);
}

}  // namespace ByteOrderSpec

}  // namespace rtype::network
