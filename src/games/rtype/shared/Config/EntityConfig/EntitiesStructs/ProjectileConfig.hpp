/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ProjectileConfig - Projectile entity configuration structure
*/

#pragma once

#include <cstdint>
#include <string>

namespace rtype::games::rtype::shared {

/**
 * @struct ProjectileConfig
 * @brief Configuration for a projectile type
 */
struct ProjectileConfig {
    std::string id;           // Unique identifier
    std::string spriteSheet;  // Path to sprite sheet

    int32_t damage = 10;
    float speed = 300.0F;
    float lifetime = 5.0F;  // Seconds before auto-destroy

    // Hitbox
    float hitboxWidth = 8.0F;
    float hitboxHeight = 4.0F;

    // Effects
    bool piercing = false;  // Can hit multiple enemies
    int32_t maxHits = 1;    // Max enemies hit (if piercing)

    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && damage > 0 && speed > 0;
    }
};

}  // namespace rtype::games::rtype::shared
