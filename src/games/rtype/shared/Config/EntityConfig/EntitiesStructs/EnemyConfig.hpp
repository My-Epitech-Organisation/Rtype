/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EnemyConfig - Enemy entity configuration structure
*/

#pragma once

#include <cstdint>
#include <string>

#include "Components/AIComponent.hpp"

namespace rtype::games::rtype::shared {

/**
 * @struct EnemyConfig
 * @brief Configuration for an enemy type loaded from config files
 */
struct EnemyConfig {
    std::string id;    // Unique identifier (e.g., "basic_enemy", "boss_1")
    std::string name;  // Display name
    std::string spriteSheet;  // Path to sprite sheet

    // Stats
    int32_t health = 100;
    int32_t damage = 10;
    int32_t scoreValue = 100;

    // Movement
    AIBehavior behavior = AIBehavior::MoveLeft;
    float speed = 100.0F;

    // Hitbox
    float hitboxWidth = 32.0F;
    float hitboxHeight = 32.0F;

    // Shooting (optional)
    bool canShoot = false;
    float fireRate = 1.0F;       // Shots per second
    std::string projectileType;  // Reference to projectile config

    /**
     * @brief Validate the enemy configuration
     * @return true if configuration is valid
     */
    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && health > 0 &&
               (speed >= 0 || behavior == AIBehavior::Stationary);
    }
};

}  // namespace rtype::games::rtype::shared
