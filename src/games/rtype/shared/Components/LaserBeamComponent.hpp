/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LaserBeamComponent - Continuous laser beam weapon state and timing
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @enum LaserBeamState
 * @brief States for the continuous laser beam weapon
 */
enum class LaserBeamState : uint8_t {
    Inactive = 0,  ///< Not firing, ready to use
    Active,        ///< Currently firing beam
    Cooldown       ///< Forced cooldown after release or max duration
};

/**
 * @struct LaserBeamComponent
 * @brief Component for continuous laser beam weapon state and timing
 *
 * This component handles the laser beam state machine
 * (Inactive/Active/Cooldown). Collision and damage are handled separately via:
 * - BoundingBoxComponent: Collision dimensions
 * - DamageOnContactComponent: DPS damage with startup delay
 */
struct LaserBeamComponent {
    LaserBeamState state{LaserBeamState::Inactive};
    uint32_t ownerNetworkId{0};

    // Timing
    float activeTime{0.0F};        ///< Time beam has been active
    float maxDuration{3.0F};       ///< Maximum fire duration (seconds)
    float cooldownTime{0.0F};      ///< Current cooldown remaining
    float cooldownDuration{2.0F};  ///< Cooldown after release (seconds)

    // Animation (optional visual effect)
    float pulsePhase{0.0F};  ///< For pulsation visual effect
    float pulseSpeed{8.0F};  ///< Pulsation frequency

    LaserBeamComponent() = default;

    /**
     * @brief Check if laser can be fired
     * @return true if in Inactive state
     */
    [[nodiscard]] bool canFire() const noexcept {
        return state == LaserBeamState::Inactive;
    }

    /**
     * @brief Check if laser is currently firing
     * @return true if in Active state
     */
    [[nodiscard]] bool isActive() const noexcept {
        return state == LaserBeamState::Active;
    }

    /**
     * @brief Check if laser is cooling down
     * @return true if in Cooldown state
     */
    [[nodiscard]] bool isCoolingDown() const noexcept {
        return state == LaserBeamState::Cooldown;
    }

    // Note: State mutation methods (startFiring, stopFiring, forceStop, update)
    // have been moved to LaserBeamSystem for ECS purity.
    // This component is now data-only with read-only accessors.

    /**
     * @brief Get cooldown progress (0.0 = just started, 1.0 = ready)
     * @return Cooldown progress percentage
     */
    [[nodiscard]] float getCooldownProgress() const noexcept {
        if (state != LaserBeamState::Cooldown || cooldownDuration <= 0.0F) {
            return 1.0F;
        }
        return 1.0F - (cooldownTime / cooldownDuration);
    }

    /**
     * @brief Get active duration progress (0.0 = just started, 1.0 = max)
     * @return Active duration progress percentage
     */
    [[nodiscard]] float getDurationProgress() const noexcept {
        if (state != LaserBeamState::Active || maxDuration <= 0.0F) {
            return 0.0F;
        }
        return activeTime / maxDuration;
    }
};

}  // namespace rtype::games::rtype::shared
