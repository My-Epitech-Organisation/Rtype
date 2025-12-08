/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** FileOperations - File I/O operations for save system
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_OPERATIONS_FILEOPERATIONS_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_OPERATIONS_FILEOPERATIONS_HPP_

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace rtype::game::config {

/**
 * @class FileOperations
 * @brief Handles file I/O operations for save system
 */
class FileOperations {
   public:
    /**
     * @brief Write binary data to file with safe write pattern
     * @param filepath Path to the file
     * @param data Binary data to write
     * @return nullopt if successful, error message otherwise
     */
    [[nodiscard]] static std::optional<std::string> writeToFile(
        const std::filesystem::path& filepath,
        const std::vector<uint8_t>& data);

    /**
     * @brief Read binary data from file
     * @param filepath Path to the file
     * @return Pair of optional data and optional error message
     */
    [[nodiscard]] static std::pair<std::optional<std::vector<uint8_t>>,
                                   std::optional<std::string>>
    readFromFile(const std::filesystem::path& filepath);

    /**
     * @brief Delete a file
     * @param filepath Path to the file
     * @return nullopt if successful, error message otherwise
     */
    [[nodiscard]] static std::optional<std::string> deleteFile(
        const std::filesystem::path& filepath);

    /**
     * @brief Copy a file
     * @param source Source path
     * @param destination Destination path
     * @return nullopt if successful, error message otherwise
     */
    [[nodiscard]] static std::optional<std::string> copyFile(
        const std::filesystem::path& source,
        const std::filesystem::path& destination);

    /**
     * @brief Check if a file exists
     * @param filepath Path to check
     * @return true if file exists
     */
    [[nodiscard]] static bool exists(const std::filesystem::path& filepath);
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_OPERATIONS_FILEOPERATIONS_HPP_
