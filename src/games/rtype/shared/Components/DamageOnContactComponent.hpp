/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DamageOnContactComponent - Deal damage when colliding
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @struct DamageOnContactComponent
 * @brief Deals damage when entity collides with targets
 *
 * Supports two modes:
 * - Instant damage: Fixed damage per collision (projectiles, obstacles)
 * - DPS mode: Continuous damage per second (laser beams, fire)
 */
struct DamageOnContactComponent {
    // Instant damage mode
    int32_t damage{10};  ///< Fixed damage per hit (when isDPS=false)

    // DPS mode (continuous damage)
    float damagePerSecond{0.0F};  ///< Damage per second (when isDPS=true)
    bool isDPS{false};            ///< Enable DPS mode instead of instant

    // Behavior
    bool destroySelf{false};  ///< Destroy this entity after dealing damage

    // Owner tracking (to prevent friendly fire)
    uint32_t ownerNetworkId{0};  ///< Network ID of owner (0 = no owner)

    // Startup delay (for weapons with charge-up animation)
    float startupDelay{0.0F};  ///< Delay before damage becomes active
    float activeTime{0.0F};    ///< Time since activation (for startup check)

    /**
     * @brief Check if damage is currently active (past startup delay)
     */
    [[nodiscard]] bool isActive() const noexcept {
        return !isDPS || activeTime >= startupDelay;
    }

    /**
     * @brief Calculate damage for this frame
     * @param deltaTime Frame delta time (only used for DPS mode)
     * @return Damage to apply (minimum 1 for DPS mode)
     */
    [[nodiscard]] int32_t calculateDamage(float deltaTime) const noexcept {
        if (!isDPS) {
            return damage;
        }
        int32_t dpsDamage = static_cast<int32_t>(damagePerSecond * deltaTime);
        return dpsDamage > 0 ? dpsDamage : 1;  // Minimum 1 damage per frame
    }
};

}  // namespace rtype::games::rtype::shared
