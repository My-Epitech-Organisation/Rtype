/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Config - Generic TOML parser for configuration files
*/

#ifndef SRC_COMMON_CONFIG_CONFIG_HPP_
#define SRC_COMMON_CONFIG_CONFIG_HPP_

#include "TomlParser.hpp"

/**
 * @namespace rtype::config
 * @brief Generic configuration parsing utilities
 *
 * This module provides a generic TOML configuration parser:
 *
 * - TomlParser: Generic TOML parser with value extraction helpers
 * - ParseResult: Result structure with success flag and errors
 * - ParseError: Error details with section, key, and message
 *
 * For R-Type specific configuration, see:
 * - rtype::game::config::RTypeConfigParser
 * - rtype::game::config::RTypeGameConfig
 * - rtype::game::config::RTypeSaveManager
 *
 * Example usage:
 * @code
 * #include "Config/Config.hpp"
 *
 * rtype::config::TomlParser parser;
 * auto table = parser.parseFile("config/settings.toml");
 * if (table) {
 *     auto width = parser.getValue<uint32_t>(*table, "video", "width", 1280);
 *     auto name = parser.getString(*table, "player", "name", "Player1");
 * } else {
 *     for (const auto& error : parser.getLastErrors()) {
 *         LOG_WARN("Config: {}", error.toString());
 *     }
 * }
 * @endcode
 */

#endif  // SRC_COMMON_CONFIG_CONFIG_HPP_
