/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** FileOperations - Implementation
*/

#include "FileOperations.hpp"

namespace rtype::game::config {

std::optional<std::string> FileOperations::writeToFile(
    const std::filesystem::path& filepath, const std::vector<uint8_t>& data) {
    if (filepath.has_parent_path() &&
        !std::filesystem::exists(filepath.parent_path())) {
        try {
            std::filesystem::create_directories(filepath.parent_path());
        } catch (const std::exception& e) {
            return std::string("Cannot create save directory: ") + e.what();
        }
    }
    auto tempPath = filepath.string() + ".tmp";

    std::ofstream file(tempPath, std::ios::binary);
    if (!file.is_open()) {
        return "Cannot create save file: " + filepath.string();
    }
    file.write(reinterpret_cast<const char*>(data.data()),
               static_cast<std::streamsize>(data.size()));
    file.close();
    if (!file) {
        std::filesystem::remove(tempPath);
        return std::string("Failed to write save file");
    }
    try {
        std::filesystem::rename(tempPath, filepath);
    } catch (const std::exception& e) {
        std::filesystem::remove(tempPath);
        return std::string("Failed to finalize save: ") + e.what();
    }

    return std::nullopt;
}

std::pair<std::optional<std::vector<uint8_t>>, std::optional<std::string>>
FileOperations::readFromFile(const std::filesystem::path& filepath) {
    if (!std::filesystem::exists(filepath)) {
        return {std::nullopt, "Save file not found: " + filepath.string()};
    }

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {std::nullopt, "Cannot open save file: " + filepath.string()};
    }

    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    if (!file) {
        return {std::nullopt, std::string("Failed to read save file")};
    }

    return {data, std::nullopt};
}

std::optional<std::string> FileOperations::deleteFile(
    const std::filesystem::path& filepath) {
    try {
        if (std::filesystem::exists(filepath)) {
            std::filesystem::remove(filepath);
        }
        return std::nullopt;
    } catch (const std::exception& e) {
        return std::string("Failed to delete file: ") + e.what();
    }
}

std::optional<std::string> FileOperations::copyFile(
    const std::filesystem::path& source,
    const std::filesystem::path& destination) {
    try {
        std::filesystem::copy_file(
            source, destination,
            std::filesystem::copy_options::overwrite_existing);
        return std::nullopt;
    } catch (const std::exception& e) {
        return std::string("Failed to copy file: ") + e.what();
    }
}

bool FileOperations::exists(const std::filesystem::path& filepath) {
    return std::filesystem::exists(filepath);
}

}  // namespace rtype::game::config
