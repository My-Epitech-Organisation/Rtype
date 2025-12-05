/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TomlParser - Generic TOML configuration parser
*/

#ifndef SRC_COMMON_CONFIG_TOMLPARSER_HPP_
#define SRC_COMMON_CONFIG_TOMLPARSER_HPP_

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <toml++/toml.hpp>

namespace rtype::config {

/**
 * @brief Generic configuration error structure
 */
struct ParseError {
    std::string section;
    std::string key;
    std::string message;

    [[nodiscard]] std::string toString() const {
        if (key.empty()) {
            return "[" + section + "] " + message;
        }
        return "[" + section + "." + key + "] " + message;
    }
};

/**
 * @brief Result of a parse operation
 */
struct ParseResult {
    bool success = false;
    std::vector<ParseError> errors;
    std::string errorMessage;  ///< General error message (file not found, parse error, etc.)

    [[nodiscard]] explicit operator bool() const noexcept { return success; }
};

/**
 * @class TomlParser
 * @brief Generic TOML configuration parser
 *
 * This class provides a generic interface for parsing TOML configuration files.
 * It handles:
 * - File reading and parsing
 * - Detailed error reporting
 * - Safe file writing with atomic operations
 *
 * Game-specific configuration parsing should extend or use this class
 * to handle their specific configuration structures.
 *
 * Example usage:
 * @code
 * TomlParser parser;
 * auto table = parser.parseFile("config/client.toml");
 * if (table) {
 *     auto width = parser.getValue<uint32_t>(*table, "video", "width", 1280);
 *     auto address = parser.getString(*table, "network", "serverAddress", "127.0.0.1");
 * } else {
 *     for (const auto& error : parser.getLastErrors()) {
 *         LOG_ERROR("{}", error.toString());
 *     }
 * }
 * @endcode
 *
 * @note The parser uses tomlplusplus for TOML parsing.
 */
class TomlParser {
   public:
    using ErrorCallback = std::function<void(const ParseError&)>;

    TomlParser() = default;
    ~TomlParser() = default;

    TomlParser(const TomlParser&) = delete;
    TomlParser& operator=(const TomlParser&) = delete;
    TomlParser(TomlParser&&) = default;
    TomlParser& operator=(TomlParser&&) = default;

    /**
     * @brief Parse TOML from file
     * @param filepath Path to TOML file
     * @return Parsed TOML table, or nullopt on failure
     */
    [[nodiscard]] std::optional<toml::table> parseFile(
        const std::filesystem::path& filepath);

    /**
     * @brief Parse TOML from string
     * @param content TOML content string
     * @return Parsed TOML table, or nullopt on failure
     */
    [[nodiscard]] std::optional<toml::table> parseString(const std::string& content);

    /**
     * @brief Save TOML table to file
     * @param table TOML table to save
     * @param filepath Output file path
     * @return true if successful
     */
    bool saveToFile(const toml::table& table, const std::filesystem::path& filepath);

    /**
     * @brief Get a value from a TOML table with default fallback
     * @tparam T Value type
     * @param table TOML table
     * @param section Section name
     * @param key Key name
     * @param defaultValue Default value if key not found
     * @return Value from table or default
     */
    template <typename T>
    [[nodiscard]] T getValue(const toml::table& table, std::string_view section,
                             std::string_view key, const T& defaultValue) {
        try {
            if (auto* sec = table[section].as_table()) {
                if (auto val = (*sec)[key].value<T>()) {
                    return *val;
                }
            }
        } catch (const std::exception& e) {
            reportError({std::string(section), std::string(key), e.what()});
        }
        return defaultValue;
    }

    /**
     * @brief Get a string value from a TOML table with default fallback
     * @param table TOML table
     * @param section Section name
     * @param key Key name
     * @param defaultValue Default value if key not found
     * @return String value from table or default
     */
    [[nodiscard]] std::string getString(const toml::table& table, std::string_view section,
                                        std::string_view key, const std::string& defaultValue);

    /**
     * @brief Get the last parse result
     * @return Last parse result
     */
    [[nodiscard]] const ParseResult& getLastResult() const noexcept {
        return _lastResult;
    }

    /**
     * @brief Get errors from last parse operation
     * @return Vector of parse errors
     */
    [[nodiscard]] const std::vector<ParseError>& getLastErrors() const noexcept {
        return _lastResult.errors;
    }

    /**
     * @brief Set callback for error reporting
     * @param callback Function to call for each error
     */
    void setErrorCallback(ErrorCallback callback) {
        _errorCallback = std::move(callback);
    }

    /**
     * @brief Report an error
     * @param error Error to report
     */
    void reportError(const ParseError& error);

   protected:
    ParseResult _lastResult;
    ErrorCallback _errorCallback;
};

}  // namespace rtype::config

#endif  // SRC_COMMON_CONFIG_TOMLPARSER_HPP_
