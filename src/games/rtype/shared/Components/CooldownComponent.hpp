/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CooldownComponent - Manages action cooldowns to prevent spamming
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @struct ShootCooldownComponent
 * @brief Component managing shooting cooldown for entities
 *
 * Prevents rapid-fire spamming by enforcing a minimum time between shots.
 * The cooldown is applied per-entity and can vary based on weapon type.
 */
struct ShootCooldownComponent {
    float cooldownTime = 0.25F;
    float currentCooldown = 0.0F;
    uint8_t currentWeaponSlot = 0;

    ShootCooldownComponent() = default;

    /**
     * @brief Construct with specific cooldown time
     * @param cooldown Time between shots in seconds
     */
    explicit ShootCooldownComponent(float cooldown)
        : cooldownTime(cooldown), currentCooldown(0.0F) {}

    /**
     * @brief Check if entity can shoot
     * @return true if cooldown has elapsed
     */
    [[nodiscard]] bool canShoot() const noexcept {
        return currentCooldown <= 0.0F;
    }

    /**
     * @brief Trigger the cooldown after shooting
     */
    void triggerCooldown() noexcept { currentCooldown = cooldownTime; }

    /**
     * @brief Update cooldown timer
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime) noexcept {
        if (currentCooldown > 0.0F) {
            currentCooldown -= deltaTime;
            if (currentCooldown < 0.0F) {
                currentCooldown = 0.0F;
            }
        }
    }

    /**
     * @brief Reset cooldown to zero (e.g., for power-ups)
     */
    void reset() noexcept { currentCooldown = 0.0F; }

    /**
     * @brief Set new cooldown time (weapon change, power-up)
     * @param newCooldown New cooldown time in seconds
     */
    void setCooldownTime(float newCooldown) noexcept {
        cooldownTime = newCooldown;
    }

    /**
     * @brief Change weapon slot
     * @param slot New weapon slot index
     */
    void setWeaponSlot(uint8_t slot) noexcept { currentWeaponSlot = slot; }
};

/**
 * @struct ChargeComponent
 * @brief Component for charged attacks (hold to charge, release to fire)
 *
 * Used for mechanics where holding the shoot button charges a more
 * powerful attack.
 */
struct ChargeComponent {
    float currentCharge = 0.0F;      ///< Current charge level (0.0 - 1.0)
    float chargeRate = 0.5F;         ///< Charge rate per second
    float maxCharge = 1.0F;
    bool isCharging = false;
    float minChargeThreshold = 0.0F; ///< Minimum charge for powered shot

    ChargeComponent() = default;

    /**
     * @brief Construct with specific charge rate
     * @param rate Charge rate per second
     */
    explicit ChargeComponent(float rate) : chargeRate(rate) {}

    /**
     * @brief Start charging
     */
    void startCharging() noexcept { isCharging = true; }

    /**
     * @brief Stop charging and return charge level
     * @return Current charge level when released
     */
    [[nodiscard]] float release() noexcept {
        isCharging = false;
        float charge = currentCharge;
        currentCharge = 0.0F;
        return charge;
    }

    /**
     * @brief Update charge if currently charging
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime) noexcept {
        if (isCharging && currentCharge < maxCharge) {
            currentCharge += chargeRate * deltaTime;
            if (currentCharge > maxCharge) {
                currentCharge = maxCharge;
            }
        }
    }

    /**
     * @brief Check if charge exceeds threshold for powered shot
     * @return true if charge is above threshold
     */
    [[nodiscard]] bool isPoweredShot() const noexcept {
        return currentCharge >= minChargeThreshold;
    }

    /**
     * @brief Get charge percentage (0.0 - 1.0)
     * @return Charge percentage
     */
    [[nodiscard]] float getChargePercent() const noexcept {
        return currentCharge / maxCharge;
    }
};

}  // namespace rtype::games::rtype::shared

