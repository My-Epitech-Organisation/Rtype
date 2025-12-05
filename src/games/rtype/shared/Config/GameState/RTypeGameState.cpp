/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RTypeGameState - Implementation
*/

#include "RTypeGameState.hpp"

#include <chrono>
#include <cstring>

namespace rtype::game::config {

RTypeGameState RTypeGameState::createNew() {
    RTypeGameState state;
    state.updateTimestamp();

    PlayerState player;
    player.playerId = 1;
    player.health = 100;
    player.maxHealth = 100;
    player.lives = 3;
    state.players.push_back(player);

    return state;
}

void RTypeGameState::updateTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    header.timestamp = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(epoch).count());
}

uint32_t RTypeGameState::calculateChecksum() const {
    constexpr uint32_t FNV_OFFSET_BASIS = 2166136261U;
    constexpr uint32_t FNV_PRIME = 16777619U;

    uint32_t hash = FNV_OFFSET_BASIS;

    auto hashValue = [&hash](const void* data, size_t size) {
        const auto* bytes = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < size; ++i) {
            hash ^= bytes[i];
            hash *= FNV_PRIME;
        }
    };

    for (const auto& player : players) {
        hashValue(&player.playerId, sizeof(player.playerId));
        hashValue(&player.positionX, sizeof(player.positionX));
        hashValue(&player.positionY, sizeof(player.positionY));
        hashValue(&player.health, sizeof(player.health));
        hashValue(&player.score, sizeof(player.score));
        hashValue(&player.lives, sizeof(player.lives));
    }

    hashValue(&progression.currentLevel, sizeof(progression.currentLevel));
    hashValue(&progression.currentWave, sizeof(progression.currentWave));
    hashValue(&progression.totalScore, sizeof(progression.totalScore));
    hashValue(&progression.enemiesDefeated,
              sizeof(progression.enemiesDefeated));

    for (char c : difficulty.difficultyLevel) {
        hash ^= static_cast<uint32_t>(c);
        hash *= FNV_PRIME;
    }

    return hash;
}

bool RTypeGameState::isValid() const {
    if (header.magic != SAVE_MAGIC_NUMBER) {
        return false;
    }
    if (header.version > SAVE_FORMAT_VERSION) {
        return false;
    }
    if (players.empty()) {
        return false;
    }
    for (const auto& player : players) {
        if (player.health < 0 || player.health > player.maxHealth) {
            return false;
        }
        if (player.lives < 0) {
            return false;
        }
    }
    if (progression.currentLevel == 0) {
        return false;
    }
    if (progression.currentWave == 0) {
        return false;
    }

    return true;
}

}  // namespace rtype::game::config
