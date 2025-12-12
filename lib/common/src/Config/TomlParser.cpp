/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TomlParser - Implementation
*/

#include "TomlParser.hpp"

#include <fstream>
#include <sstream>

namespace rtype::config {

std::optional<toml::table> TomlParser::parseFile(
    const std::filesystem::path& filepath) {
    _lastResult = ParseResult{};

    if (!std::filesystem::exists(filepath)) {
        _lastResult.errorMessage = "File not found: " + filepath.string();
        reportError({"file", "", _lastResult.errorMessage});
        return std::nullopt;
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        _lastResult.errorMessage = "Cannot open file: " + filepath.string();
        reportError({"file", "", _lastResult.errorMessage});
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return parseString(buffer.str());
}

std::optional<toml::table> TomlParser::parseString(const std::string& content) {
    _lastResult = ParseResult{};

    try {
        auto table = toml::parse(content);
        _lastResult.success = true;
        return table;
    } catch (const toml::parse_error& e) {
        _lastResult.errorMessage = std::string("TOML parse error: ") + std::string(e.what());
        reportError({"parser", "", _lastResult.errorMessage});
        return std::nullopt;
    } catch (const std::exception& e) {
        _lastResult.errorMessage = std::string("Parse error: ") + std::string(e.what());
        reportError({"parser", "", _lastResult.errorMessage});
        return std::nullopt;
    }
}

bool TomlParser::saveToFile(const toml::table& table,
                            const std::filesystem::path& filepath) {
    if (filepath.has_parent_path()) {
        std::filesystem::create_directories(filepath.parent_path());
    }

    auto tempPath = filepath.string() + ".tmp";
    std::ofstream file(tempPath);
    if (!file.is_open()) {
        _lastResult.errorMessage = "Cannot create file: " + filepath.string();
        reportError({"file", "", _lastResult.errorMessage});
        return false;
    }

    file << table;
    file.close();

    if (!file) {
        _lastResult.errorMessage =
            "Failed to write to file: " + filepath.string();
        reportError({"file", "", _lastResult.errorMessage});
        std::filesystem::remove(tempPath);
        return false;
    }

    try {
        std::filesystem::rename(tempPath, filepath);
    } catch (const std::exception& e) {
        _lastResult.errorMessage =
            std::string("Failed to save file: ") + std::string(e.what());
        reportError({"file", "", _lastResult.errorMessage});
        std::filesystem::remove(tempPath);
        return false;
    }

    return true;
}

std::string TomlParser::getString(const toml::table& table,
                                  std::string_view section,
                                  std::string_view key,
                                  const std::string& defaultValue) {
    try {
        if (auto val = table[section][key].value<std::string>()) {
            return *val;
        }
    } catch (const std::exception& e) {
        reportError({std::string(section), std::string(key), std::string(e.what())});
    }
    return defaultValue;
}

void TomlParser::reportError(const ParseError& error) {
    _lastResult.errors.push_back(error);
    if (_errorCallback) {
        _errorCallback(error);
    }
}

}  // namespace rtype::config
