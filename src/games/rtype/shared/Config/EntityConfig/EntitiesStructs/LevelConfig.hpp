/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LevelConfig - Level configuration structure
*/

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "MapElementConfig.hpp"
#include "WaveConfig.hpp"

namespace rtype::games::rtype::shared {

/**
 * @struct LevelConfig
 * @brief Configuration for a complete level
 */
struct LevelConfig {
    std::string id;
    std::string name;
    std::string backgroundPath;

    float scrollSpeed = 50.0F;
    std::vector<WaveConfig> waves;

    // Map configuration (obstacles, tiles, starfield)
    MapConfig map;

    // Boss (optional)
    std::optional<std::string> bossId;

    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && !waves.empty();
    }
};

}  // namespace rtype::games::rtype::shared
