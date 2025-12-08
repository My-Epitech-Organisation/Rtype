/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** WaveConfig - Wave configuration structure
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace rtype::games::rtype::shared {

/**
 * @struct WaveConfig
 * @brief Configuration for an enemy wave
 */
struct WaveConfig {
    int32_t waveNumber = 1;
    float spawnDelay = 0.5F;

    struct SpawnEntry {
        std::string enemyId;
        float x = 800.0F;
        float y = 300.0F;
        float delay = 0.0F;
        int32_t count = 1;
    };

    std::vector<SpawnEntry> spawns;

    [[nodiscard]] bool isValid() const noexcept {
        return waveNumber > 0 && !spawns.empty();
    }
};

}  // namespace rtype::games::rtype::shared
