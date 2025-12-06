/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeConfigParser - R-Type specific configuration parser
*/

#ifndef SRC_GAMES_RTYPE_SHARED_CONFIG_PARSER_RTYPECONFIGPARSER_HPP_
#define SRC_GAMES_RTYPE_SHARED_CONFIG_PARSER_RTYPECONFIGPARSER_HPP_

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <rtype/common.hpp>

#include "../GameConfig/RTypeGameConfig.hpp"

namespace rtype::game::config {

/**
 * @class RTypeConfigParser
 * @brief R-Type specific configuration parser
 *
 * This class provides R-Type specific configuration parsing using the
 * generic TomlParser. It handles:
 * - Loading R-Type game configuration from TOML files
 * - Validation of R-Type specific configuration values
 * - Default value handling for missing keys
 * - Serialization of R-Type configuration to TOML
 *
 * Example usage:
 * @code
 * RTypeConfigParser parser;
 * auto config = parser.loadFromFile("config/client.toml");
 * if (config) {
 *     // Use config value
 * } else {
 *     for (const auto& error : parser.getLastErrors()) {
 *         LOG_ERROR("{}", error.toString());
 *     }
 * }
 * @endcode
 */
class RTypeConfigParser {
   public:
    using ErrorCallback = std::function<void(const ConfigError&)>;

    RTypeConfigParser() = default;
    ~RTypeConfigParser() = default;

    RTypeConfigParser(const RTypeConfigParser&) = delete;
    RTypeConfigParser& operator=(const RTypeConfigParser&) = delete;
    RTypeConfigParser(RTypeConfigParser&&) = default;
    RTypeConfigParser& operator=(RTypeConfigParser&&) = default;

    /**
     * @brief Load configuration from file
     * @param filepath Path to configuration file
     * @return Loaded configuration, or nullopt on failure
     */
    [[nodiscard]] std::optional<RTypeGameConfig> loadFromFile(
        const std::filesystem::path& filepath);

    /**
     * @brief Load configuration from string
     * @param content Configuration content
     * @return Loaded configuration, or nullopt on failure
     */
    [[nodiscard]] std::optional<RTypeGameConfig> loadFromString(
        const std::string& content);

    /**
     * @brief Save configuration to file
     * @param config Configuration to save
     * @param filepath Output file path
     * @return true if successful
     */
    bool saveToFile(const RTypeGameConfig& config,
                    const std::filesystem::path& filepath);

    /**
     * @brief Serialize configuration to string
     * @param config Configuration to serialize
     * @return Serialized configuration string
     */
    [[nodiscard]] std::string serializeToString(const RTypeGameConfig& config);

    /**
     * @brief Get the last parse result
     * @return Last parse result
     */
    [[nodiscard]] const rtype::config::ParseResult& getLastResult()
        const noexcept {
        return _parser.getLastResult();
    }

    /**
     * @brief Get errors from last parse operation
     * @return Vector of parse errors
     */
    [[nodiscard]] const std::vector<rtype::config::ParseError>& getLastErrors()
        const noexcept {
        return _parser.getLastErrors();
    }

    /**
     * @brief Set callback for error reporting
     * @param callback Function to call for each error
     */
    void setErrorCallback(ErrorCallback callback);

   private:
    [[nodiscard]] RTypeGameConfig parseFromTable(const toml::table& table);
    [[nodiscard]] std::string serializeToToml(const RTypeGameConfig& config);

    rtype::config::TomlParser _parser;
    std::vector<ConfigError> _validationErrors;
};

}  // namespace rtype::game::config

#endif  // SRC_GAMES_RTYPE_SHARED_CONFIG_PARSER_RTYPECONFIGPARSER_HPP_
