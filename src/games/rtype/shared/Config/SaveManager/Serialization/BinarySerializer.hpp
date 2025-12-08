/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** BinarySerializer - Binary serialization/deserialization helpers
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_SERIALIZATION_BINARYSERIALIZER_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_SERIALIZATION_BINARYSERIALIZER_HPP_

#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace rtype::game::config {

/**
 * @class BinarySerializer
 * @brief Provides binary serialization and deserialization utilities
 *
 * Uses little-endian format for cross-platform compatibility.
 */
class BinarySerializer {
   public:
    // ==================== Write methods ====================

    /**
     * @brief Write a uint8_t to the buffer
     */
    static void writeUint8(std::shared_ptr<std::vector<uint8_t>> buffer,
                           uint8_t value);

    /**
     * @brief Write a uint16_t to the buffer (little-endian)
     */
    static void writeUint16(std::shared_ptr<std::vector<uint8_t>> buffer,
                            uint16_t value);

    /**
     * @brief Write a uint32_t to the buffer (little-endian)
     */
    static void writeUint32(std::shared_ptr<std::vector<uint8_t>> buffer,
                            uint32_t value);

    /**
     * @brief Write a uint64_t to the buffer (little-endian)
     */
    static void writeUint64(std::shared_ptr<std::vector<uint8_t>> buffer,
                            uint64_t value);

    /**
     * @brief Write an int32_t to the buffer
     */
    static void writeInt32(std::shared_ptr<std::vector<uint8_t>> buffer,
                           int32_t value);

    /**
     * @brief Write a float to the buffer
     */
    static void writeFloat(std::shared_ptr<std::vector<uint8_t>> buffer,
                           float value);

    /**
     * @brief Write a string to the buffer (length-prefixed)
     */
    static void writeString(std::shared_ptr<std::vector<uint8_t>> buffer,
                            const std::string& value);

    // ==================== Read methods ====================

    /**
     * @brief Read a uint8_t from the buffer
     * @throws std::out_of_range if buffer overflow
     */
    [[nodiscard]] static uint8_t readUint8(
        std::shared_ptr<const std::vector<uint8_t>> buffer,
        std::shared_ptr<size_t> offset);

    /**
     * @brief Read a uint16_t from the buffer (little-endian)
     * @throws std::out_of_range if buffer overflow
     */
    [[nodiscard]] static uint16_t readUint16(
        std::shared_ptr<const std::vector<uint8_t>> buffer,
        std::shared_ptr<size_t> offset);

    /**
     * @brief Read a uint32_t from the buffer (little-endian)
     * @throws std::out_of_range if buffer overflow
     */
    [[nodiscard]] static uint32_t readUint32(
        std::shared_ptr<const std::vector<uint8_t>> buffer,
        std::shared_ptr<size_t> offset);

    /**
     * @brief Read a uint64_t from the buffer (little-endian)
     * @throws std::out_of_range if buffer overflow
     */
    [[nodiscard]] static uint64_t readUint64(
        std::shared_ptr<const std::vector<uint8_t>> buffer,
        std::shared_ptr<size_t> offset);

    /**
     * @brief Read an int32_t from the buffer
     * @throws std::out_of_range if buffer overflow
     */
    [[nodiscard]] static int32_t readInt32(
        std::shared_ptr<const std::vector<uint8_t>> buffer,
        std::shared_ptr<size_t> offset);

    /**
     * @brief Read a float from the buffer
     * @throws std::out_of_range if buffer overflow
     */
    [[nodiscard]] static float readFloat(
        std::shared_ptr<const std::vector<uint8_t>> buffer,
        std::shared_ptr<size_t> offset);

    /**
     * @brief Read a string from the buffer (length-prefixed)
     * @throws std::out_of_range if buffer overflow
     */
    [[nodiscard]] static std::string readString(
        std::shared_ptr<const std::vector<uint8_t>> buffer,
        std::shared_ptr<size_t> offset);
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_SERIALIZATION_BINARYSERIALIZER_HPP_
