/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeConfig - Main include file for R-Type configuration system
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_RTYPECONFIG_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_RTYPECONFIG_HPP_

#include "Parser/RTypeConfigParser.hpp"
#include "GameConfig/RTypeGameConfig.hpp"
#include "GameState/RTypeGameState.hpp"
#include "SaveManager/RTypeSaveManager.hpp"

/**
 * @namespace rtype::game::config
 * @brief Configuration and save system for the R-Type game
 *
 * This module provides a complete configuration and save system with:
 *
 * Configuration System:
 * - RTypeGameConfig: Complete game configuration with validation
 * - RTypeConfigParser: R-Type specific parser using generic TomlParser
 * - Default values for missing configuration
 * - Schema versioning for forward compatibility
 *
 * Save System:
 * - RTypeGameState: Serializable game state structure
 * - RTypeSaveManager: Binary save/load with version control
 * - Checksum validation for corruption detection
 * - Safe file writing with atomic operations
 *
 * Example usage - Configuration:
 * @code
 * #include "Config/RTypeConfig.hpp"
 *
 * rtype::game::config::RTypeConfigParser parser;
 * auto config = parser.loadFromFile("config/client.toml");
 * if (!config) {
 *     // Use default configuration
 *     config = rtype::game::config::RTypeGameConfig::createDefault();
 *     for (const auto& error : parser.getLastErrors()) {
 *         LOG_WARN("Config: {}", error.toString());
 *     }
 * }
 *
 * // Apply settings
 * window.setSize(config->video.width, config->video.height);
 * audio.setVolume(config->audio.masterVolume);
 * @endcode
 *
 * Example usage - Saves:
 * @code
 * rtype::game::config::RTypeSaveManager saveManager("saves/");
 *
 * // Save game
 * rtype::game::config::RTypeGameState state =
 * rtype::game::config::RTypeGameState::createNew(); state.players[0].score =
 * 5000; state.progression.currentLevel = 3; saveManager.save(state, "slot1");
 *
 * // Load game
 * auto loaded = saveManager.load("slot1");
 * if (loaded) {
 *     restoreGameState(*loaded);
 * }
 *
 * // List saves
 * for (const auto& info : saveManager.listSaves()) {
 *     std::cout << info.saveName << " - Level " << info.currentLevel << "\n";
 * }
 * @endcode
 */

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_RTYPECONFIG_HPP_
