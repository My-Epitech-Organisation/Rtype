/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeSaveManager - Implementation
*/

#include "RTypeSaveManager.hpp"

#include <algorithm>

#include "Operations/FileOperations.hpp"
#include "Operations/SaveInfoReader.hpp"
#include "Serialization/GameStateSerializer.hpp"

namespace rtype::game::config {

RTypeSaveManager::RTypeSaveManager(std::filesystem::path saveDirectory,
                                   std::string fileExtension)
    : _saveDirectory(std::move(saveDirectory)),
      _fileExtension(std::move(fileExtension)) {
    if (!std::filesystem::exists(_saveDirectory)) {
        std::filesystem::create_directories(_saveDirectory);
    }
}

std::filesystem::path RTypeSaveManager::getFilePath(
    const std::string& slotName) const {
    return _saveDirectory / (slotName + _fileExtension);
}

SaveResult RTypeSaveManager::save(const RTypeGameState& state,
                                  const std::string& slotName) {
    _lastError.clear();

    if (!state.isValid()) {
        _lastResult = SaveResult::InvalidData;
        _lastError = "Invalid game state";
        return _lastResult;
    }

    auto data = GameStateSerializer::serialize(state);

    if (!FileOperations::writeToFile(getFilePath(slotName), data, _lastError)) {
        _lastResult = SaveResult::IOError;
        return _lastResult;
    }

    _lastResult = SaveResult::Success;
    return _lastResult;
}

std::optional<RTypeGameState> RTypeSaveManager::load(
    const std::string& slotName) {
    _lastError.clear();

    auto filepath = getFilePath(slotName);

    if (!FileOperations::exists(filepath)) {
        _lastResult = SaveResult::FileNotFound;
        _lastError = "Save file not found: " + filepath.string();
        return std::nullopt;
    }

    auto data = FileOperations::readFromFile(filepath, _lastError);
    if (!data) {
        _lastResult = SaveResult::IOError;
        return std::nullopt;
    }

    auto state = GameStateSerializer::deserialize(*data, _lastError);
    if (!state) {
        _lastResult = SaveResult::FileCorrupted;
        return std::nullopt;
    }

    auto calculatedChecksum = state->calculateChecksum();
    if (calculatedChecksum != state->header.checksum) {
        _lastResult = SaveResult::FileCorrupted;
        _lastError = "Save file checksum mismatch - file may be corrupted";
        return std::nullopt;
    }

    if (state->header.version < SAVE_FORMAT_VERSION) {
        if (_migrationCallback) {
            if (!_migrationCallback(*state, state->header.version)) {
                _lastResult = SaveResult::VersionMismatch;
                _lastError = "Failed to migrate save from version " +
                             std::to_string(state->header.version);
                return std::nullopt;
            }
            state->header.version = SAVE_FORMAT_VERSION;
        }
    } else if (state->header.version > SAVE_FORMAT_VERSION) {
        _lastResult = SaveResult::VersionMismatch;
        _lastError = "Save file version " +
                     std::to_string(state->header.version) +
                     " is newer than supported version " +
                     std::to_string(SAVE_FORMAT_VERSION);
        return std::nullopt;
    }

    _lastResult = SaveResult::Success;
    return state;
}

bool RTypeSaveManager::deleteSave(const std::string& slotName) {
    return FileOperations::deleteFile(getFilePath(slotName), _lastError);
}

bool RTypeSaveManager::saveExists(const std::string& slotName) const {
    return FileOperations::exists(getFilePath(slotName));
}

std::vector<SaveInfo> RTypeSaveManager::listSaves() const {
    std::vector<SaveInfo> saves;

    if (!std::filesystem::exists(_saveDirectory)) {
        return saves;
    }

    for (const auto& entry :
         std::filesystem::directory_iterator(_saveDirectory)) {
        if (entry.is_regular_file() &&
            entry.path().extension() == _fileExtension) {
            auto info = SaveInfoReader::readSaveInfo(entry.path());
            if (info.isValid) {
                saves.push_back(info);
            }
        }
    }
    std::sort(saves.begin(), saves.end(),
              [](const SaveInfo& a, const SaveInfo& b) {
                  return a.timestamp > b.timestamp;
              });

    return saves;
}

std::optional<SaveInfo> RTypeSaveManager::getSaveInfo(
    const std::string& slotName) const {
    auto filepath = getFilePath(slotName);
    if (!FileOperations::exists(filepath)) {
        return std::nullopt;
    }

    auto info = SaveInfoReader::readSaveInfo(filepath);
    if (!info.isValid) {
        return std::nullopt;
    }

    return info;
}

bool RTypeSaveManager::createBackup(const std::string& slotName,
                                    const std::string& backupName) {
    auto filepath = getFilePath(slotName);
    if (!FileOperations::exists(filepath)) {
        _lastError = "Save file not found";
        return false;
    }

    std::string backupSlot =
        backupName.empty() ? slotName + ".bak" : backupName;
    auto backupPath = getFilePath(backupSlot);

    return FileOperations::copyFile(filepath, backupPath, _lastError);
}

bool RTypeSaveManager::restoreBackup(const std::string& slotName,
                                     const std::string& backupName) {
    std::string backupSlot =
        backupName.empty() ? slotName + ".bak" : backupName;
    auto backupPath = getFilePath(backupSlot);

    if (!FileOperations::exists(backupPath)) {
        _lastError = "Backup file not found";
        return false;
    }

    auto filepath = getFilePath(slotName);
    return FileOperations::copyFile(backupPath, filepath, _lastError);
}

}  // namespace rtype::game::config
