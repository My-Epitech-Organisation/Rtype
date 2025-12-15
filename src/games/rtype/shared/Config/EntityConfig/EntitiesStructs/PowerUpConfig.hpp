/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PowerUpConfig - Power-up entity configuration structure
*/

#pragma once

#include <cstdint>
#include <string>

namespace rtype::games::rtype::shared {

/**
 * @struct PowerUpConfig
 * @brief Configuration for power-up items
 */
struct PowerUpConfig {
    std::string id;
    std::string name;
    std::string spriteSheet;

    enum class EffectType : uint8_t {
        Health,
        SpeedBoost,
        WeaponUpgrade,
        Shield,
        HealthBoost
    };
    EffectType effect = EffectType::Health;

    float duration = 0.0F;  // 0 = permanent (like health)
    int32_t value = 25;

    float hitboxWidth = 16.0F;
    float hitboxHeight = 16.0F;

    [[nodiscard]] bool isValid() const noexcept { return !id.empty(); }
};

}  // namespace rtype::games::rtype::shared
