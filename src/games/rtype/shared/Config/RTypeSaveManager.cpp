/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeSaveManager - Implementation
*/

#include "RTypeSaveManager.hpp"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace rtype::game::config {

RTypeSaveManager::RTypeSaveManager(std::filesystem::path saveDirectory, std::string fileExtension)
    : _saveDirectory(std::move(saveDirectory))
    , _fileExtension(std::move(fileExtension)) {
    // Create save directory if it doesn't exist
    if (!std::filesystem::exists(_saveDirectory)) {
        std::filesystem::create_directories(_saveDirectory);
    }
}

std::filesystem::path RTypeSaveManager::getFilePath(const std::string& slotName) const {
    return _saveDirectory / (slotName + _fileExtension);
}

SaveResult RTypeSaveManager::save(const RTypeGameState& state, const std::string& slotName) {
    _lastError.clear();

    // Validate state before saving
    if (!state.isValid()) {
        _lastResult = SaveResult::InvalidData;
        _lastError = "Invalid game state";
        return _lastResult;
    }

    // Serialize the state
    auto data = serialize(state);

    // Create parent directories if needed
    if (!std::filesystem::exists(_saveDirectory)) {
        try {
            std::filesystem::create_directories(_saveDirectory);
        } catch (const std::exception& e) {
            _lastResult = SaveResult::IOError;
            _lastError = std::string("Cannot create save directory: ") + e.what();
            return _lastResult;
        }
    }

    // Write to temporary file first (safe write pattern)
    auto filepath = getFilePath(slotName);
    auto tempPath = filepath.string() + ".tmp";

    std::ofstream file(tempPath, std::ios::binary);
    if (!file.is_open()) {
        _lastResult = SaveResult::IOError;
        _lastError = "Cannot create save file: " + filepath.string();
        return _lastResult;
    }

    file.write(reinterpret_cast<const char*>(data.data()),
               static_cast<std::streamsize>(data.size()));
    file.close();

    if (!file) {
        _lastResult = SaveResult::IOError;
        _lastError = "Failed to write save file";
        std::filesystem::remove(tempPath);
        return _lastResult;
    }

    // Atomic rename
    try {
        std::filesystem::rename(tempPath, filepath);
    } catch (const std::exception& e) {
        _lastResult = SaveResult::IOError;
        _lastError = std::string("Failed to finalize save: ") + e.what();
        std::filesystem::remove(tempPath);
        return _lastResult;
    }

    _lastResult = SaveResult::Success;
    return _lastResult;
}

std::optional<RTypeGameState> RTypeSaveManager::load(const std::string& slotName) {
    _lastError.clear();

    auto filepath = getFilePath(slotName);

    if (!std::filesystem::exists(filepath)) {
        _lastResult = SaveResult::FileNotFound;
        _lastError = "Save file not found: " + filepath.string();
        return std::nullopt;
    }

    // Read file
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        _lastResult = SaveResult::IOError;
        _lastError = "Cannot open save file: " + filepath.string();
        return std::nullopt;
    }

    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(data.data()), fileSize);
    file.close();

    if (!file) {
        _lastResult = SaveResult::IOError;
        _lastError = "Failed to read save file";
        return std::nullopt;
    }

    // Deserialize
    auto state = deserialize(data);
    if (!state) {
        return std::nullopt;  // Error already set in deserialize
    }

    // Verify checksum
    auto calculatedChecksum = state->calculateChecksum();
    if (calculatedChecksum != state->header.checksum) {
        _lastResult = SaveResult::FileCorrupted;
        _lastError = "Save file checksum mismatch - file may be corrupted";
        return std::nullopt;
    }

    // Handle version migration
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
        _lastError = "Save file version " + std::to_string(state->header.version) +
                     " is newer than supported version " +
                     std::to_string(SAVE_FORMAT_VERSION);
        return std::nullopt;
    }

    _lastResult = SaveResult::Success;
    return state;
}

bool RTypeSaveManager::deleteSave(const std::string& slotName) {
    auto filepath = getFilePath(slotName);

    try {
        if (std::filesystem::exists(filepath)) {
            std::filesystem::remove(filepath);
            return true;
        }
    } catch (const std::exception& e) {
        _lastError = std::string("Failed to delete save: ") + e.what();
    }
    return false;
}

bool RTypeSaveManager::saveExists(const std::string& slotName) const {
    return std::filesystem::exists(getFilePath(slotName));
}

std::vector<SaveInfo> RTypeSaveManager::listSaves() const {
    std::vector<SaveInfo> saves;

    if (!std::filesystem::exists(_saveDirectory)) {
        return saves;
    }

    for (const auto& entry : std::filesystem::directory_iterator(_saveDirectory)) {
        if (entry.is_regular_file() &&
            entry.path().extension() == _fileExtension) {
            auto info = readSaveInfo(entry.path());
            if (info.isValid) {
                saves.push_back(info);
            }
        }
    }

    // Sort by timestamp (most recent first)
    std::sort(saves.begin(), saves.end(),
              [](const SaveInfo& a, const SaveInfo& b) {
                  return a.timestamp > b.timestamp;
              });

    return saves;
}

std::optional<SaveInfo> RTypeSaveManager::getSaveInfo(const std::string& slotName) const {
    auto filepath = getFilePath(slotName);
    if (!std::filesystem::exists(filepath)) {
        return std::nullopt;
    }

    auto info = readSaveInfo(filepath);
    if (!info.isValid) {
        return std::nullopt;
    }

    return info;
}

bool RTypeSaveManager::createBackup(const std::string& slotName, const std::string& backupName) {
    auto filepath = getFilePath(slotName);
    if (!std::filesystem::exists(filepath)) {
        _lastError = "Save file not found";
        return false;
    }

    std::string backupSlot = backupName.empty() ? slotName + ".bak" : backupName;
    auto backupPath = getFilePath(backupSlot);

    try {
        std::filesystem::copy_file(filepath, backupPath,
            std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        _lastError = std::string("Failed to create backup: ") + e.what();
        return false;
    }
}

bool RTypeSaveManager::restoreBackup(const std::string& slotName, const std::string& backupName) {
    std::string backupSlot = backupName.empty() ? slotName + ".bak" : backupName;
    auto backupPath = getFilePath(backupSlot);

    if (!std::filesystem::exists(backupPath)) {
        _lastError = "Backup file not found";
        return false;
    }

    auto filepath = getFilePath(slotName);

    try {
        std::filesystem::copy_file(backupPath, filepath,
            std::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        _lastError = std::string("Failed to restore backup: ") + e.what();
        return false;
    }
}

SaveInfo RTypeSaveManager::readSaveInfo(const std::filesystem::path& filepath) const {
    SaveInfo info;
    info.filename = filepath.stem().string();

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return info;
    }

    // Read just the header + minimal data for info
    std::vector<uint8_t> headerData(sizeof(SaveHeader));
    file.read(reinterpret_cast<char*>(headerData.data()), sizeof(SaveHeader));

    if (!file || file.gcount() != sizeof(SaveHeader)) {
        return info;
    }

    size_t offset = 0;
    uint32_t magic = readUint32(headerData, offset);
    if (magic != SAVE_MAGIC_NUMBER) {
        return info;
    }

    info.version = readUint32(headerData, offset);
    info.timestamp = readUint64(headerData, offset);

    // Skip checksum and dataSize
    offset += 8;

    // Read progression data
    // This requires reading more of the file to get save name and progression
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> fullData(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(fullData.data()), fileSize);
    file.close();

    // Try to deserialize to get more info
    offset = sizeof(SaveHeader);
    if (offset < fullData.size()) {
        try {
            // Read save name
            info.saveName = readString(fullData, offset);

            // Skip player count and player data to get to progression
            uint32_t playerCount = readUint32(fullData, offset);
            for (uint32_t i = 0; i < playerCount && offset < fullData.size(); ++i) {
                offset += sizeof(uint32_t);  // playerId
                offset += sizeof(float) * 3; // position, rotation
                offset += sizeof(int32_t) * 3; // health, maxHealth, lives
                offset += sizeof(uint32_t);  // score
                offset += sizeof(uint8_t);   // powerUp
                offset += sizeof(float);     // powerUpTime
                offset += sizeof(uint32_t);  // weaponLevel
            }

            // Skip enemy data
            uint32_t enemyCount = readUint32(fullData, offset);
            for (uint32_t i = 0; i < enemyCount && offset < fullData.size(); ++i) {
                offset += sizeof(uint32_t);  // enemyId
                offset += sizeof(uint8_t);   // enemyType
                offset += sizeof(float) * 2; // position
                offset += sizeof(int32_t);   // health
            }

            // Read progression
            info.currentLevel = readUint32(fullData, offset);
            info.currentWave = readUint32(fullData, offset);
            offset += sizeof(uint32_t); // totalWaves
            offset += sizeof(uint32_t); // enemiesDefeated
            info.totalScore = readUint32(fullData, offset);
            info.playTimeSeconds = readFloat(fullData, offset);

            info.isValid = true;
        } catch (...) {
            // Failed to read additional info, but header was valid
            info.isValid = true;
        }
    }

    return info;
}

std::vector<uint8_t> RTypeSaveManager::serialize(const RTypeGameState& state) const {
    std::vector<uint8_t> data;
    data.reserve(1024);  // Pre-allocate reasonable size

    // Create a copy of state to update header
    RTypeGameState stateCopy = state;
    stateCopy.updateTimestamp();
    stateCopy.header.checksum = stateCopy.calculateChecksum();

    // Write header
    writeUint32(data, stateCopy.header.magic);
    writeUint32(data, stateCopy.header.version);
    writeUint64(data, stateCopy.header.timestamp);
    writeUint32(data, stateCopy.header.checksum);
    writeUint32(data, 0);  // dataSize placeholder - will be updated

    size_t dataSizeOffset = data.size() - sizeof(uint32_t);
    size_t dataStartOffset = data.size();

    // Write save name
    writeString(data, stateCopy.saveName);

    // Write players
    writeUint32(data, static_cast<uint32_t>(stateCopy.players.size()));
    for (const auto& player : stateCopy.players) {
        writeUint32(data, player.playerId);
        writeFloat(data, player.positionX);
        writeFloat(data, player.positionY);
        writeFloat(data, player.rotation);
        writeInt32(data, player.health);
        writeInt32(data, player.maxHealth);
        writeInt32(data, player.lives);
        writeUint32(data, player.score);
        writeUint8(data, static_cast<uint8_t>(player.activePowerUp));
        writeFloat(data, player.powerUpTimeRemaining);
        writeUint32(data, player.weaponLevel);
    }

    // Write enemies
    writeUint32(data, static_cast<uint32_t>(stateCopy.enemies.size()));
    for (const auto& enemy : stateCopy.enemies) {
        writeUint32(data, enemy.enemyId);
        writeUint8(data, enemy.enemyType);
        writeFloat(data, enemy.positionX);
        writeFloat(data, enemy.positionY);
        writeInt32(data, enemy.health);
    }

    // Write progression
    writeUint32(data, stateCopy.progression.currentLevel);
    writeUint32(data, stateCopy.progression.currentWave);
    writeUint32(data, stateCopy.progression.totalWaves);
    writeUint32(data, stateCopy.progression.enemiesDefeated);
    writeUint32(data, stateCopy.progression.totalScore);
    writeFloat(data, stateCopy.progression.playTimeSeconds);

    // Write checkpoint
    writeUint32(data, stateCopy.progression.lastCheckpoint.checkpointId);
    writeUint32(data, stateCopy.progression.lastCheckpoint.waveNumber);
    writeFloat(data, stateCopy.progression.lastCheckpoint.waveProgress);

    // Write difficulty
    writeString(data, stateCopy.difficulty.difficultyLevel);
    writeFloat(data, stateCopy.difficulty.enemyHealthMultiplier);
    writeFloat(data, stateCopy.difficulty.enemySpeedMultiplier);
    writeFloat(data, stateCopy.difficulty.playerDamageMultiplier);
    writeUint32(data, stateCopy.difficulty.startingLives);

    // Update data size in header
    uint32_t dataSize = static_cast<uint32_t>(data.size() - dataStartOffset);
    std::memcpy(data.data() + dataSizeOffset, &dataSize, sizeof(dataSize));

    return data;
}

std::optional<RTypeGameState> RTypeSaveManager::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(SaveHeader)) {
        _lastResult = SaveResult::FileCorrupted;
        _lastError = "Save file too small";
        return std::nullopt;
    }

    RTypeGameState state;
    size_t offset = 0;

    try {
        // Read header
        state.header.magic = readUint32(data, offset);
        if (state.header.magic != SAVE_MAGIC_NUMBER) {
            _lastResult = SaveResult::FileCorrupted;
            _lastError = "Invalid save file magic number";
            return std::nullopt;
        }

        state.header.version = readUint32(data, offset);
        state.header.timestamp = readUint64(data, offset);
        state.header.checksum = readUint32(data, offset);
        state.header.dataSize = readUint32(data, offset);

        // Read save name
        state.saveName = readString(data, offset);

        // Read players
        uint32_t playerCount = readUint32(data, offset);
        state.players.reserve(playerCount);
        for (uint32_t i = 0; i < playerCount; ++i) {
            PlayerState player;
            player.playerId = readUint32(data, offset);
            player.positionX = readFloat(data, offset);
            player.positionY = readFloat(data, offset);
            player.rotation = readFloat(data, offset);
            player.health = readInt32(data, offset);
            player.maxHealth = readInt32(data, offset);
            player.lives = readInt32(data, offset);
            player.score = readUint32(data, offset);
            player.activePowerUp = static_cast<PowerUpType>(readUint8(data, offset));
            player.powerUpTimeRemaining = readFloat(data, offset);
            player.weaponLevel = readUint32(data, offset);
            state.players.push_back(player);
        }

        // Read enemies
        uint32_t enemyCount = readUint32(data, offset);
        state.enemies.reserve(enemyCount);
        for (uint32_t i = 0; i < enemyCount; ++i) {
            EnemyState enemy;
            enemy.enemyId = readUint32(data, offset);
            enemy.enemyType = readUint8(data, offset);
            enemy.positionX = readFloat(data, offset);
            enemy.positionY = readFloat(data, offset);
            enemy.health = readInt32(data, offset);
            state.enemies.push_back(enemy);
        }

        // Read progression
        state.progression.currentLevel = readUint32(data, offset);
        state.progression.currentWave = readUint32(data, offset);
        state.progression.totalWaves = readUint32(data, offset);
        state.progression.enemiesDefeated = readUint32(data, offset);
        state.progression.totalScore = readUint32(data, offset);
        state.progression.playTimeSeconds = readFloat(data, offset);

        // Read checkpoint
        state.progression.lastCheckpoint.checkpointId = readUint32(data, offset);
        state.progression.lastCheckpoint.waveNumber = readUint32(data, offset);
        state.progression.lastCheckpoint.waveProgress = readFloat(data, offset);

        // Read difficulty
        state.difficulty.difficultyLevel = readString(data, offset);
        state.difficulty.enemyHealthMultiplier = readFloat(data, offset);
        state.difficulty.enemySpeedMultiplier = readFloat(data, offset);
        state.difficulty.playerDamageMultiplier = readFloat(data, offset);
        state.difficulty.startingLives = readUint32(data, offset);

        return state;

    } catch (const std::exception& e) {
        _lastResult = SaveResult::FileCorrupted;
        _lastError = std::string("Failed to deserialize save: ") + e.what();
        return std::nullopt;
    }
}

// Serialization helpers using little-endian format
void RTypeSaveManager::writeUint8(std::vector<uint8_t>& buffer, uint8_t value) const {
    buffer.push_back(value);
}

void RTypeSaveManager::writeUint16(std::vector<uint8_t>& buffer, uint16_t value) const {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void RTypeSaveManager::writeUint32(std::vector<uint8_t>& buffer, uint32_t value) const {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void RTypeSaveManager::writeUint64(std::vector<uint8_t>& buffer, uint64_t value) const {
    for (int i = 0; i < 8; ++i) {
        buffer.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
    }
}

void RTypeSaveManager::writeInt32(std::vector<uint8_t>& buffer, int32_t value) const {
    writeUint32(buffer, static_cast<uint32_t>(value));
}

void RTypeSaveManager::writeFloat(std::vector<uint8_t>& buffer, float value) const {
    uint32_t intVal;
    std::memcpy(&intVal, &value, sizeof(float));
    writeUint32(buffer, intVal);
}

void RTypeSaveManager::writeString(std::vector<uint8_t>& buffer, const std::string& value) const {
    writeUint32(buffer, static_cast<uint32_t>(value.size()));
    buffer.insert(buffer.end(), value.begin(), value.end());
}

uint8_t RTypeSaveManager::readUint8(const std::vector<uint8_t>& buffer, size_t& offset) const {
    if (offset >= buffer.size()) {
        throw std::out_of_range("Buffer overflow reading uint8");
    }
    return buffer[offset++];
}

uint16_t RTypeSaveManager::readUint16(const std::vector<uint8_t>& buffer, size_t& offset) const {
    if (offset + 2 > buffer.size()) {
        throw std::out_of_range("Buffer overflow reading uint16");
    }
    uint16_t value = static_cast<uint16_t>(buffer[offset]) |
                     (static_cast<uint16_t>(buffer[offset + 1]) << 8);
    offset += 2;
    return value;
}

uint32_t RTypeSaveManager::readUint32(const std::vector<uint8_t>& buffer, size_t& offset) const {
    if (offset + 4 > buffer.size()) {
        throw std::out_of_range("Buffer overflow reading uint32");
    }
    uint32_t value = static_cast<uint32_t>(buffer[offset]) |
                     (static_cast<uint32_t>(buffer[offset + 1]) << 8) |
                     (static_cast<uint32_t>(buffer[offset + 2]) << 16) |
                     (static_cast<uint32_t>(buffer[offset + 3]) << 24);
    offset += 4;
    return value;
}

uint64_t RTypeSaveManager::readUint64(const std::vector<uint8_t>& buffer, size_t& offset) const {
    if (offset + 8 > buffer.size()) {
        throw std::out_of_range("Buffer overflow reading uint64");
    }
    uint64_t value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= static_cast<uint64_t>(buffer[offset + i]) << (i * 8);
    }
    offset += 8;
    return value;
}

int32_t RTypeSaveManager::readInt32(const std::vector<uint8_t>& buffer, size_t& offset) const {
    return static_cast<int32_t>(readUint32(buffer, offset));
}

float RTypeSaveManager::readFloat(const std::vector<uint8_t>& buffer, size_t& offset) const {
    uint32_t intVal = readUint32(buffer, offset);
    float value;
    std::memcpy(&value, &intVal, sizeof(float));
    return value;
}

std::string RTypeSaveManager::readString(const std::vector<uint8_t>& buffer, size_t& offset) const {
    uint32_t length = readUint32(buffer, offset);
    if (offset + length > buffer.size()) {
        throw std::out_of_range("Buffer overflow reading string");
    }
    std::string value(buffer.begin() + offset, buffer.begin() + offset + length);
    offset += length;
    return value;
}

}  // namespace rtype::game::config
