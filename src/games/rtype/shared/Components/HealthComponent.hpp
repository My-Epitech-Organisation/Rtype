/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** HealthComponent - Entity health data
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @struct HealthComponent
 * @brief Component for entities that can take damage
 *
 * Used by players, enemies, and destructible objects.
 * When current reaches 0, entity should be destroyed.
 */
struct HealthComponent {
    int32_t current = 100;  ///< Current health points
    int32_t max = 100;      ///< Maximum health points

    /**
     * @brief Check if entity is alive
     * @return true if current health > 0
     */
    [[nodiscard]] bool isAlive() const noexcept { return current > 0; }

    /**
     * @brief Apply damage to entity
     * @param amount Damage amount (positive value)
     */
    void takeDamage(int32_t amount) noexcept {
        current = (current > amount) ? current - amount : 0;
    }

    /**
     * @brief Heal entity
     * @param amount Heal amount (positive value)
     */
    void heal(int32_t amount) noexcept {
        current = (current + amount > max) ? max : current + amount;
    }
};

}  // namespace rtype::games::rtype::shared
