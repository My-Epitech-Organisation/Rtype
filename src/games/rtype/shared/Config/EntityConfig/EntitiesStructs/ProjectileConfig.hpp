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
    std::string id;
    std::string spriteSheet;

    int32_t damage = 10;
    float speed = 300.0F;
    float lifetime = 5.0F;
    float hitboxWidth = 8.0F;
    float hitboxHeight = 4.0F;

    bool piercing = false;
    int32_t maxHits = 1;
    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && damage > 0 && speed > 0;
    }
};

}  // namespace rtype::games::rtype::shared
