/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SaveInfoReader - Implementation
*/

#include "SaveInfoReader.hpp"

namespace rtype::game::config {

SaveInfo SaveInfoReader::readSaveInfo(const std::filesystem::path& filepath) {
    SaveInfo info;
    info.filename = filepath.stem().string();

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return info;
    }

    // Read just the header
    std::vector<uint8_t> headerData(sizeof(SaveHeader));
    file.read(reinterpret_cast<char*>(headerData.data()), sizeof(SaveHeader));

    if (!file ||
        file.gcount() != static_cast<std::streamsize>(sizeof(SaveHeader))) {
        return info;
    }

    size_t offset = 0;
    uint32_t magic = BinarySerializer::readUint32(headerData, offset);
    if (magic != SAVE_MAGIC_NUMBER) {
        return info;
    }

    info.version = BinarySerializer::readUint32(headerData, offset);
    info.timestamp = BinarySerializer::readUint64(headerData, offset);

    // Skip checksum and dataSize
    offset += 8;

    // Read full file for additional info
    file.seekg(0, std::ios::end);
    auto fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> fullData(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(fullData.data()), fileSize);
    file.close();

    // Try to read progression info
    offset = sizeof(SaveHeader);
    if (offset < fullData.size()) {
        try {
            readProgressionInfo(fullData, offset, info);
            info.isValid = true;
        } catch (...) {
            // Failed to read additional info, but header was valid
            info.isValid = true;
        }
    }

    return info;
}

void SaveInfoReader::readProgressionInfo(const std::vector<uint8_t>& data,
                                         size_t& offset, SaveInfo& info) {
    // Read save name
    info.saveName = BinarySerializer::readString(data, offset);

    // Skip player data
    uint32_t playerCount = BinarySerializer::readUint32(data, offset);
    for (uint32_t i = 0; i < playerCount && offset < data.size(); ++i) {
        offset += sizeof(uint32_t);     // playerId
        offset += sizeof(float) * 3;    // position, rotation
        offset += sizeof(int32_t) * 3;  // health, maxHealth, lives
        offset += sizeof(uint32_t);     // score
        offset += sizeof(uint8_t);      // powerUp
        offset += sizeof(float);        // powerUpTime
        offset += sizeof(uint32_t);     // weaponLevel
    }

    // Skip enemy data
    uint32_t enemyCount = BinarySerializer::readUint32(data, offset);
    for (uint32_t i = 0; i < enemyCount && offset < data.size(); ++i) {
        offset += sizeof(uint32_t);   // enemyId
        offset += sizeof(uint8_t);    // enemyType
        offset += sizeof(float) * 2;  // position
        offset += sizeof(int32_t);    // health
    }

    // Read progression
    info.currentLevel = BinarySerializer::readUint32(data, offset);
    info.currentWave = BinarySerializer::readUint32(data, offset);
    offset += sizeof(uint32_t);  // totalWaves
    offset += sizeof(uint32_t);  // enemiesDefeated
    info.totalScore = BinarySerializer::readUint32(data, offset);
    info.playTimeSeconds = BinarySerializer::readFloat(data, offset);
}

}  // namespace rtype::game::config
