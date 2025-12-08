/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameStateSerializer - Implementation
*/

#include "GameStateSerializer.hpp"

#include <cstring>

namespace rtype::game::config {

std::vector<uint8_t> GameStateSerializer::serialize(
    const RTypeGameState& state) {
    auto data = std::make_shared<std::vector<uint8_t>>();
    data->reserve(1024);

    RTypeGameState stateCopy = state;
    stateCopy.updateTimestamp();
    stateCopy.header.checksum = stateCopy.calculateChecksum();

    BinarySerializer::writeUint32(data, stateCopy.header.magic);
    BinarySerializer::writeUint32(data, stateCopy.header.version);
    BinarySerializer::writeUint64(data, stateCopy.header.timestamp);
    BinarySerializer::writeUint32(data, stateCopy.header.checksum);
    BinarySerializer::writeUint32(data, 0);  // dataSize placeholder

    size_t dataSizeOffset = data->size() - sizeof(uint32_t);
    size_t dataStartOffset = data->size();

    BinarySerializer::writeString(data, stateCopy.saveName);
    serializePlayers(data, stateCopy.players);
    serializeEnemies(data, stateCopy.enemies);
    serializeProgression(data, stateCopy.progression);
    serializeDifficulty(data, stateCopy.difficulty);
    uint32_t dataSize = static_cast<uint32_t>(data->size() - dataStartOffset);
    std::memcpy(data->data() + dataSizeOffset, &dataSize, sizeof(dataSize));

    return *data;
}

std::pair<std::optional<RTypeGameState>, std::optional<std::string>>
GameStateSerializer::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(SaveHeader)) {
        return {std::nullopt, std::string("Save file too small")};
    }

    RTypeGameState state;
    auto offset = std::make_shared<size_t>(0);
    auto dataPtr = std::make_shared<const std::vector<uint8_t>>(data);

    try {
        state.header.magic = BinarySerializer::readUint32(dataPtr, offset);
        if (state.header.magic != SAVE_MAGIC_NUMBER) {
            return {std::nullopt,
                    std::string("Invalid save file magic number")};
        }

        state.header.version = BinarySerializer::readUint32(dataPtr, offset);
        state.header.timestamp = BinarySerializer::readUint64(dataPtr, offset);
        state.header.checksum = BinarySerializer::readUint32(dataPtr, offset);
        state.header.dataSize = BinarySerializer::readUint32(dataPtr, offset);
        state.saveName = BinarySerializer::readString(dataPtr, offset);

        auto players = std::make_shared<std::vector<PlayerState>>();
        auto enemies = std::make_shared<std::vector<EnemyState>>();
        auto progression = std::make_shared<ProgressionData>();
        auto difficulty = std::make_shared<DifficultySnapshot>();

        deserializePlayers(dataPtr, offset, players);
        deserializeEnemies(dataPtr, offset, enemies);
        deserializeProgression(dataPtr, offset, progression);
        deserializeDifficulty(dataPtr, offset, difficulty);

        state.players = *players;
        state.enemies = *enemies;
        state.progression = *progression;
        state.difficulty = *difficulty;

        return {state, std::nullopt};
    } catch (const std::exception& e) {
        return {std::nullopt,
                std::string("Failed to deserialize save: ") + std::string(e.what())};
    }
}

void GameStateSerializer::serializePlayers(
    std::shared_ptr<std::vector<uint8_t>> data,
    const std::vector<PlayerState>& players) {
    BinarySerializer::writeUint32(data, static_cast<uint32_t>(players.size()));
    for (const auto& player : players) {
        BinarySerializer::writeUint32(data, player.playerId);
        BinarySerializer::writeFloat(data, player.positionX);
        BinarySerializer::writeFloat(data, player.positionY);
        BinarySerializer::writeFloat(data, player.rotation);
        BinarySerializer::writeInt32(data, player.health);
        BinarySerializer::writeInt32(data, player.maxHealth);
        BinarySerializer::writeInt32(data, player.lives);
        BinarySerializer::writeUint32(data, player.score);
        BinarySerializer::writeUint8(
            data, static_cast<uint8_t>(player.activePowerUp));
        BinarySerializer::writeFloat(data, player.powerUpTimeRemaining);
        BinarySerializer::writeUint32(data, player.weaponLevel);
    }
}

void GameStateSerializer::serializeEnemies(
    std::shared_ptr<std::vector<uint8_t>> data,
    const std::vector<EnemyState>& enemies) {
    BinarySerializer::writeUint32(data, static_cast<uint32_t>(enemies.size()));
    for (const auto& enemy : enemies) {
        BinarySerializer::writeUint32(data, enemy.enemyId);
        BinarySerializer::writeUint8(data, enemy.enemyType);
        BinarySerializer::writeFloat(data, enemy.positionX);
        BinarySerializer::writeFloat(data, enemy.positionY);
        BinarySerializer::writeInt32(data, enemy.health);
    }
}

void GameStateSerializer::serializeProgression(
    std::shared_ptr<std::vector<uint8_t>> data,
    const ProgressionData& progression) {
    BinarySerializer::writeUint32(data, progression.currentLevel);
    BinarySerializer::writeUint32(data, progression.currentWave);
    BinarySerializer::writeUint32(data, progression.totalWaves);
    BinarySerializer::writeUint32(data, progression.enemiesDefeated);
    BinarySerializer::writeUint32(data, progression.totalScore);
    BinarySerializer::writeFloat(data, progression.playTimeSeconds);
    BinarySerializer::writeUint32(data,
                                  progression.lastCheckpoint.checkpointId);
    BinarySerializer::writeUint32(data, progression.lastCheckpoint.waveNumber);
    BinarySerializer::writeFloat(data, progression.lastCheckpoint.waveProgress);
}

void GameStateSerializer::serializeDifficulty(
    std::shared_ptr<std::vector<uint8_t>> data,
    const DifficultySnapshot& difficulty) {
    BinarySerializer::writeString(data, difficulty.difficultyLevel);
    BinarySerializer::writeFloat(data, difficulty.enemyHealthMultiplier);
    BinarySerializer::writeFloat(data, difficulty.enemySpeedMultiplier);
    BinarySerializer::writeFloat(data, difficulty.playerDamageMultiplier);
    BinarySerializer::writeUint32(data, difficulty.startingLives);
}

void GameStateSerializer::deserializePlayers(
    std::shared_ptr<const std::vector<uint8_t>> data,
    std::shared_ptr<size_t> offset,
    std::shared_ptr<std::vector<PlayerState>> players) {
    uint32_t playerCount = BinarySerializer::readUint32(data, offset);
    players->reserve(playerCount);
    for (uint32_t i = 0; i < playerCount; ++i) {
        PlayerState player;
        player.playerId = BinarySerializer::readUint32(data, offset);
        player.positionX = BinarySerializer::readFloat(data, offset);
        player.positionY = BinarySerializer::readFloat(data, offset);
        player.rotation = BinarySerializer::readFloat(data, offset);
        player.health = BinarySerializer::readInt32(data, offset);
        player.maxHealth = BinarySerializer::readInt32(data, offset);
        player.lives = BinarySerializer::readInt32(data, offset);
        player.score = BinarySerializer::readUint32(data, offset);
        player.activePowerUp =
            static_cast<PowerUpType>(BinarySerializer::readUint8(data, offset));
        player.powerUpTimeRemaining = BinarySerializer::readFloat(data, offset);
        player.weaponLevel = BinarySerializer::readUint32(data, offset);
        players->push_back(player);
    }
}

void GameStateSerializer::deserializeEnemies(
    std::shared_ptr<const std::vector<uint8_t>> data,
    std::shared_ptr<size_t> offset,
    std::shared_ptr<std::vector<EnemyState>> enemies) {
    uint32_t enemyCount = BinarySerializer::readUint32(data, offset);
    enemies->reserve(enemyCount);
    for (uint32_t i = 0; i < enemyCount; ++i) {
        EnemyState enemy;
        enemy.enemyId = BinarySerializer::readUint32(data, offset);
        enemy.enemyType = BinarySerializer::readUint8(data, offset);
        enemy.positionX = BinarySerializer::readFloat(data, offset);
        enemy.positionY = BinarySerializer::readFloat(data, offset);
        enemy.health = BinarySerializer::readInt32(data, offset);
        enemies->push_back(enemy);
    }
}

void GameStateSerializer::deserializeProgression(
    std::shared_ptr<const std::vector<uint8_t>> data,
    std::shared_ptr<size_t> offset,
    std::shared_ptr<ProgressionData> progression) {
    progression->currentLevel = BinarySerializer::readUint32(data, offset);
    progression->currentWave = BinarySerializer::readUint32(data, offset);
    progression->totalWaves = BinarySerializer::readUint32(data, offset);
    progression->enemiesDefeated = BinarySerializer::readUint32(data, offset);
    progression->totalScore = BinarySerializer::readUint32(data, offset);
    progression->playTimeSeconds = BinarySerializer::readFloat(data, offset);
    progression->lastCheckpoint.checkpointId =
        BinarySerializer::readUint32(data, offset);
    progression->lastCheckpoint.waveNumber =
        BinarySerializer::readUint32(data, offset);
    progression->lastCheckpoint.waveProgress =
        BinarySerializer::readFloat(data, offset);
}

void GameStateSerializer::deserializeDifficulty(
    std::shared_ptr<const std::vector<uint8_t>> data,
    std::shared_ptr<size_t> offset,
    std::shared_ptr<DifficultySnapshot> difficulty) {
    difficulty->difficultyLevel = BinarySerializer::readString(data, offset);
    difficulty->enemyHealthMultiplier =
        BinarySerializer::readFloat(data, offset);
    difficulty->enemySpeedMultiplier =
        BinarySerializer::readFloat(data, offset);
    difficulty->playerDamageMultiplier =
        BinarySerializer::readFloat(data, offset);
    difficulty->startingLives = BinarySerializer::readUint32(data, offset);
}

}  // namespace rtype::game::config
