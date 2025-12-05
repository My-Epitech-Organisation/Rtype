/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** FileOperations - Implementation
*/

#include "FileOperations.hpp"

namespace rtype::game::config {

bool FileOperations::writeToFile(const std::filesystem::path& filepath,
                                 const std::vector<uint8_t>& data,
                                 std::string& outError) {
    if (filepath.has_parent_path() &&
        !std::filesystem::exists(filepath.parent_path())) {
        try {
            std::filesystem::create_directories(filepath.parent_path());
        } catch (const std::exception& e) {
            outError =
                std::string("Cannot create save directory: ") + e.what();
            return false;
        }
    }
    auto tempPath = filepath.string() + ".tmp";

    std::ofstream file(tempPath, std::ios::binary);
    if (!file.is_open()) {
        outError = "Cannot create save file: " + filepath.string();
        return false;
    }
    file.write(reinterpret_cast<const char*>(data.data()),
               static_cast<std::streamsize>(data.size()));
    file.close();
    if (!file) {
        outError = "Failed to write save file";
        std::filesystem::remove(tempPath);
        return false;
    }
    try {
        std::filesystem::rename(tempPath, filepath);
    } catch (const std::exception& e) {
        outError = std::string("Failed to finalize save: ") + e.what();
        std::filesystem::remove(tempPath);
        return false;
    }

    return true;
}

std::optional<std::vector<uint8_t>> FileOperations::readFromFile(
    const std::filesystem::path& filepath, std::string& outError) {
    if (!std::filesystem::exists(filepath)) {
        outError = "Save file not found: " + filepath.string();
        return std::nullopt;
    }

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        outError = "Cannot open save file: " + filepath.string();
        return std::nullopt;
    }

    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    if (!file) {
        outError = "Failed to read save file";
        return std::nullopt;
    }

    return data;
}

bool FileOperations::deleteFile(const std::filesystem::path& filepath,
                                std::string& outError) {
    try {
        if (std::filesystem::exists(filepath)) {
            std::filesystem::remove(filepath);
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        outError = std::string("Failed to delete file: ") + e.what();
        return false;
    }
}

bool FileOperations::copyFile(const std::filesystem::path& source,
                              const std::filesystem::path& destination,
                              std::string& outError) {
    try {
        std::filesystem::copy_file(
            source, destination,
            std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        outError = std::string("Failed to copy file: ") + e.what();
        return false;
    }
}

bool FileOperations::exists(const std::filesystem::path& filepath) {
    return std::filesystem::exists(filepath);
}

}  // namespace rtype::game::config
