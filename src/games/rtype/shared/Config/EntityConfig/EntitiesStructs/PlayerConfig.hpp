/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PlayerConfig - Player entity configuration structure
*/

#pragma once

#include <cstdint>
#include <string>

namespace rtype::games::rtype::shared {

/**
 * @struct PlayerConfig
 * @brief Configuration for player ships
 */
struct PlayerConfig {
    std::string id;
    std::string name;
    std::string spriteSheet;

    int32_t health = 100;
    float speed = 200.0F;
    float fireRate = 5.0F;

    float hitboxWidth = 32.0F;
    float hitboxHeight = 16.0F;

    std::string defaultProjectile = "basic_bullet";

    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && health > 0 && speed > 0;
    }
};

}  // namespace rtype::games::rtype::shared
