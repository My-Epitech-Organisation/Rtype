/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SaveTypes - Save system types and enumerations
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_TYPES_SAVETYPES_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_TYPES_SAVETYPES_HPP_

#include <cstdint>
#include <string>

namespace rtype::game::config {

/**
 * @brief Result of a save/load operation
 */
enum class SaveResult {
    Success,
    FileNotFound,
    FileCorrupted,
    VersionMismatch,
    IOError,
    InvalidData
};

/**
 * @brief Information about a save file
 */
struct SaveInfo {
    std::string filename;
    std::string saveName;
    uint64_t timestamp = 0;
    uint32_t version = 0;
    uint32_t currentLevel = 0;
    uint32_t currentWave = 0;
    uint32_t totalScore = 0;
    float playTimeSeconds = 0.0F;
    bool isValid = false;
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_SAVEMANAGER_TYPES_SAVETYPES_HPP_
