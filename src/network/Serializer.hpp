/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Serializer
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "Packet.hpp"
#include "core/ByteOrder.hpp"

namespace rtype::network {

class Serializer {
   public:
    /**
     * @brief Serialize a Packet object to binary format
     *
     * Converts a Packet to its binary representation for network transmission.
     * The format is: [packet_type_byte][payload_data...]
     *
     * @param packet Reference to the Packet to serialize
     * @return Vector containing the serialized packet data
     *
     * Example usage:
     * @code
     * Packet packet(PacketType::PlayerInput);
     * packet.setData(inputData);
     * auto serialized = Serializer::serialize(packet);
     * // First byte is packet type, rest is payload data
     * @endcode
     */
    static std::vector<uint8_t> serialize(const Packet& packet);

    /**
     * @brief Deserialize binary data to a Packet object
     *
     * Converts binary data back to a Packet object. The first byte is
     * interpreted as the packet type, and the remaining bytes form the payload
     * data.
     *
     * @param data Binary data to deserialize
     * @return Deserialized Packet object
     *
     * Example usage:
     * @code
     * std::vector<uint8_t> data = {1, 10, 20, 30}; // type=1,
     * payload={10,20,30} Packet packet = Serializer::deserialize(data);
     * // packet.type() == PacketType::PlayerInput
     * // packet.data() == {10, 20, 30}
     * @endcode
     */
    static Packet deserialize(const std::vector<uint8_t>& data);

    /**
     * @brief Serialize a trivially copyable type to binary format
     *
     * Converts any type to its binary representation using memcpy.
     * The data is serialized in NATIVE byte order.
     *
     * **For cross-platform network transmission:**
     * Use toNetworkByteOrder() on the result before sending.
     *
     * **Workflow: Struct → Serialize → ByteOrder → Send**
     *
     * @tparam T Type to serialize (must be trivially copyable and standard
     * layout)
     * @param data Reference to the data to serialize
     * @return Vector containing the binary representation in NATIVE byte order
     *
     * Example usage:
     * @code
     * // Primitive types:
     * uint32_t playerId = 42;
     * auto bytes = Serializer::serialize(playerId);  // Native byte order
     * Serializer::toNetworkByteOrder<uint32_t>(bytes);  // Convert to network
     * order
     * // Send bytes over network
     *
     * // Full structs:
     * #pragma pack(1)
     * struct PlayerData {
     *     uint32_t id;
     *     float x, y;
     *     uint16_t health;
     * };
     * #pragma pack()
     *
     * PlayerData player{42, 10.5f, 20.3f, 100};
     * auto bytes = Serializer::serialize(player);  // Native byte order
     * Serializer::toNetworkByteOrder<PlayerData>(bytes);  // Convert all
     * fields!
     * // Send bytes - now cross-platform safe!
     *
     * // Receiving:
     * auto bytes = receiveBytes();  // Network byte order
     * Serializer::fromNetworkByteOrder<PlayerData>(bytes);  // Convert to
     * native auto player = Serializer::deserialize<PlayerData>(bytes);  //
     * Perfect!
     * @endcode
     */
    template <typename T>
    static std::vector<uint8_t> serialize(const T& data) {
        static_assert(
            std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>,
            "T must be trivially copyable and standard layout");
        std::vector<uint8_t> result(sizeof(T));
        std::memcpy(result.data(), &data, sizeof(T));
        return result;
    }

    /**
     * @brief Deserialize binary data to a trivially copyable type
     *
     * Converts binary data back to its original type using memcpy.
     * The buffer must be in NATIVE byte order.
     *
     * **For data received from network:**
     * Use fromNetworkByteOrder() on the buffer before deserializing.
     *
     * @tparam T Type to deserialize to (must be trivially copyable and standard
     * layout)
     * @param buffer Binary data to deserialize (must be in NATIVE byte order)
     * @return Deserialized object of type T
     * @throws std::runtime_error if buffer size doesn't match sizeof(T)
     *
     * Example usage:
     * @code
     * // Local round-trip (same platform):
     * PlayerData player{42, 10.5f, 20.3f, 100};
     * auto bytes = Serializer::serialize(player);
     * auto restored = Serializer::deserialize<PlayerData>(bytes);
     * // player == restored ✓
     *
     * // Network round-trip (cross-platform):
     * // Sender:
     * auto bytes = Serializer::serialize(player);
     * Serializer::toNetworkByteOrder<PlayerData>(bytes);
     * sendBytes(bytes);
     *
     * // Receiver:
     * auto bytes = receiveBytes();
     * Serializer::fromNetworkByteOrder<PlayerData>(bytes);
     * auto player = Serializer::deserialize<PlayerData>(bytes);
     * // player.id == 42, player.x == 10.5f, etc. ✓
     * @endcode
     */
    template <typename T>
    static T deserialize(const std::vector<uint8_t>& buffer) {
        static_assert(
            std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>,
            "T must be trivially copyable and standard layout");
        if (buffer.size() != sizeof(T)) {
            throw std::runtime_error(
                "Invalid buffer size for deserialization: expected " +
                std::to_string(sizeof(T)) + " bytes, got " +
                std::to_string(buffer.size()) + " bytes");
        }

        T result{};
        std::memcpy(&result, buffer.data(), sizeof(T));
        return result;
    }

    /**
     * @brief Serialize a string to binary format with length prefix
     * (Network-safe)
     *
     * Converts a string to binary format by first writing the string length
     * as a uint32_t in network byte order, followed by the string data.
     * This allows for safe deserialization of strings of any length across
     * platforms.
     *
     * @param str Reference to the string to serialize
     * @return Vector containing the serialized string data
     *
     * @note The length is stored as a uint32_t (RFC-compliant) in network byte
     * order, followed by the string characters. Maximum string length is
     * 4,294,967,295 bytes. This format is portable across different
     * architectures and endianness.
     *
     * Example usage:
     * @code
     * std::string message = "Hello World";
     * auto serialized = Serializer::serialize(message);
     * // Contains: [4 bytes length in network order][11 bytes "Hello World"]
     * @endcode
     */
    static std::vector<uint8_t> serialize(const std::string& str);

    /**
     * @brief Deserialize a string from binary format
     *
     * Converts binary data back to a string. The first 4 bytes contain the
     * string length in network byte order, followed by the string characters.
     *
     * @param buffer Binary data to deserialize
     * @return Deserialized string
     * @throws std::runtime_error if buffer is malformed
     *
     * Example usage:
     * @code
     * auto buffer = Serializer::serialize("Hello World");
     * std::string restored = Serializer::deserializeString(buffer);
     * // restored == "Hello World"
     * @endcode
     */
    static std::string deserializeString(const std::vector<uint8_t>& buffer);

    /**
     * @brief Convert serialized buffer to network byte order (Big-Endian)
     *
     * Converts all multi-byte numeric fields in a buffer from host byte order
     * to network byte order. This must be called AFTER serialize() and BEFORE
     * sending data over the network.
     *
     * @tparam T Type of the serialized data
     * @param buffer Buffer to convert
     * @return New buffer with data in network byte order
     *
     * @note For primitives (uint16_t, uint32_t, int*, float), converts the
     * value. For structs, converts all 2-byte and 4-byte fields assuming
     * packed layout with RFC-compliant types.
     *
     * Example:
     * @code
     * uint32_t value = 42;
     * auto bytes = Serializer::serialize(value);
     * auto networkBytes = Serializer::toNetworkByteOrder<uint32_t>(bytes);
     * // networkBytes now in big-endian format
     * @endcode
     */
    template <typename T>
    static std::vector<uint8_t> toNetworkByteOrder(
        const std::vector<uint8_t>& buffer) {
        static_assert(
            std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>,
            "T must be trivially copyable and standard layout");

        if (buffer.size() != sizeof(T)) {
            throw std::runtime_error(
                "Buffer size mismatch for byte order conversion: expected " +
                std::to_string(sizeof(T)) + " bytes, got " +
                std::to_string(buffer.size()) + " bytes");
        }

        std::vector<uint8_t> result = buffer;

        if constexpr (ByteOrder::is_network_numeric_v<T>) {
            T value;
            std::memcpy(&value, result.data(), sizeof(T));
            T networkValue = ByteOrder::toNetwork(value);
            std::memcpy(result.data(), &networkValue, sizeof(T));
        } else {
            size_t offset = 0;
            while (offset + 4 <= result.size()) {
                uint32_t value32;
                std::memcpy(&value32, result.data() + offset, sizeof(uint32_t));
                uint32_t network32 = ByteOrder::toNetwork(value32);
                std::memcpy(result.data() + offset, &network32,
                            sizeof(uint32_t));
                offset += 4;
            }
            while (offset + 2 <= result.size()) {
                uint16_t value16;
                std::memcpy(&value16, result.data() + offset, sizeof(uint16_t));
                uint16_t network16 = ByteOrder::toNetwork(value16);
                std::memcpy(result.data() + offset, &network16,
                            sizeof(uint16_t));
                offset += 2;
            }
        }

        return result;
    }

    /**
     * @brief Convert buffer from network byte order to host byte order
     *
     * Converts all multi-byte numeric fields in a buffer from network byte
     * order to host byte order. This must be called AFTER receiving data from
     * the network and BEFORE deserialize().
     *
     * @tparam T Type of the serialized data
     * @param buffer Buffer to convert
     * @return New buffer with data in host byte order
     *
     * Example:
     * @code
     * auto networkBytes = receiveFromNetwork();
     * auto hostBytes =
     * Serializer::fromNetworkByteOrder<uint32_t>(networkBytes); uint32_t value
     * = Serializer::deserialize<uint32_t>(hostBytes);
     * // value now in native byte order
     * @endcode
     */
    template <typename T>
    static std::vector<uint8_t> fromNetworkByteOrder(
        const std::vector<uint8_t>& buffer) {
        static_assert(
            std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>,
            "T must be trivially copyable and standard layout");

        if (buffer.size() != sizeof(T)) {
            throw std::runtime_error(
                "Buffer size mismatch for byte order conversion: expected " +
                std::to_string(sizeof(T)) + " bytes, got " +
                std::to_string(buffer.size()) + " bytes");
        }

        std::vector<uint8_t> result = buffer;

        if constexpr (ByteOrder::is_network_numeric_v<T>) {
            T networkValue;
            std::memcpy(&networkValue, result.data(), sizeof(T));
            T value = ByteOrder::fromNetwork(networkValue);
            std::memcpy(result.data(), &value, sizeof(T));
        } else {
            size_t offset = 0;
            while (offset + 4 <= result.size()) {
                uint32_t network32;
                std::memcpy(&network32, result.data() + offset,
                            sizeof(uint32_t));
                uint32_t value32 = ByteOrder::fromNetwork(network32);
                std::memcpy(result.data() + offset, &value32, sizeof(uint32_t));
                offset += 4;
            }
            while (offset + 2 <= result.size()) {
                uint16_t network16;
                std::memcpy(&network16, result.data() + offset,
                            sizeof(uint16_t));
                uint16_t value16 = ByteOrder::fromNetwork(network16);
                std::memcpy(result.data() + offset, &value16, sizeof(uint16_t));
                offset += 2;
            }
        }

        return result;
    }
};

}  // namespace rtype::network
