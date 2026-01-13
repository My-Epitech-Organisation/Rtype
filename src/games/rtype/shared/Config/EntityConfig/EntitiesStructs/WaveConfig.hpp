/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** WaveConfig - Wave configuration structure
*/

#pragma once

#include <cstdint>
#include <optional>
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
        std::optional<float> x;
        std::optional<float> y;
        float delay = 0.0F;
        int32_t count = 1;

        [[nodiscard]] bool hasFixedX() const noexcept { return x.has_value(); }
        [[nodiscard]] bool hasFixedY() const noexcept { return y.has_value(); }
    };

    struct PowerUpEntry {
        std::string powerUpId;
        std::optional<float> x;
        std::optional<float> y;
        float delay = 0.0F;

        [[nodiscard]] bool hasFixedX() const noexcept { return x.has_value(); }
        [[nodiscard]] bool hasFixedY() const noexcept { return y.has_value(); }
    };

    std::vector<SpawnEntry> spawns;
    std::vector<PowerUpEntry> powerups;

    [[nodiscard]] bool isValid() const noexcept {
        return waveNumber > 0 && (!spawns.empty() || !powerups.empty());
    }
};

}  // namespace rtype::games::rtype::shared
