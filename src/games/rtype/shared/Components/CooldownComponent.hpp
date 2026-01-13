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
 * @enum ChargeLevel
 * @brief Represents charge levels for charged attacks
 */
enum class ChargeLevel { None, Level1, Level2, Level3 };

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
    float currentCharge = 0.0F;  ///< Current charge level (0.0 - 1.0)
    float chargeRate = 0.5F;     ///< Charge rate per second
    float maxCharge = 1.0F;
    bool isCharging = false;
    bool wasCharging = false;         ///< Track previous charging state
    float minChargeThreshold = 0.0F;  ///< Minimum charge for powered shot
    ChargeLevel currentLevel = ChargeLevel::None;  ///< Current charge level

    static constexpr float kLevel1Threshold = 0.3F;
    static constexpr float kLevel2Threshold = 0.6F;
    static constexpr float kLevel3Threshold = 0.9F;

    static constexpr int32_t kLevel1Damage = 20;
    static constexpr int32_t kLevel2Damage = 40;
    static constexpr int32_t kLevel3Damage = 80;

    static constexpr int32_t kLevel1Pierce = 1;
    static constexpr int32_t kLevel2Pierce = 2;
    static constexpr int32_t kLevel3Pierce = 4;

    ChargeComponent() = default;

    /**
     * @brief Construct with specific charge rate
     * @param rate Charge rate per second
     */
    explicit ChargeComponent(float rate) : chargeRate(rate) {}

    /**
     * @brief Start charging
     */
    void startCharging() noexcept {
        isCharging = true;
        wasCharging = true;
    }

    /**
     * @brief Stop charging and return charge level enum
     * @return Current charge level when released
     */
    [[nodiscard]] ChargeLevel release() noexcept {
        isCharging = false;
        wasCharging = false;
        ChargeLevel level = currentLevel;
        currentCharge = 0.0F;
        currentLevel = ChargeLevel::None;
        return level;
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
            if (currentCharge >= kLevel3Threshold) {
                currentLevel = ChargeLevel::Level3;
            } else if (currentCharge >= kLevel2Threshold) {
                currentLevel = ChargeLevel::Level2;
            } else if (currentCharge >= kLevel1Threshold) {
                currentLevel = ChargeLevel::Level1;
            } else {
                currentLevel = ChargeLevel::None;
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

    /**
     * @brief Get damage value for a charge level
     * @param level The charge level
     * @return Damage value
     */
    [[nodiscard]] static int32_t getDamageForLevel(ChargeLevel level) noexcept {
        switch (level) {
            case ChargeLevel::Level1:
                return kLevel1Damage;
            case ChargeLevel::Level2:
                return kLevel2Damage;
            case ChargeLevel::Level3:
                return kLevel3Damage;
            default:
                return 0;
        }
    }

    /**
     * @brief Get pierce count for a charge level
     * @param level The charge level
     * @return Pierce count
     */
    [[nodiscard]] static int32_t getPierceCountForLevel(
        ChargeLevel level) noexcept {
        switch (level) {
            case ChargeLevel::Level1:
                return kLevel1Pierce;
            case ChargeLevel::Level2:
                return kLevel2Pierce;
            case ChargeLevel::Level3:
                return kLevel3Pierce;
            default:
                return 0;
        }
    }
};

}  // namespace rtype::games::rtype::shared
