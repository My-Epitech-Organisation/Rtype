/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargeShotVisualComponent - Visual components for charge shot system
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_CHARGESHOTVISUALCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_CHARGESHOTVISUALCOMPONENT_HPP_

#include <cstdint>
#include <tuple>

#include "games/rtype/shared/Components/CooldownComponent.hpp"

namespace rtype::games::rtype::client {

/**
 * @struct ShootInputState
 * @brief Singleton for tracking shoot button state
 */
struct ShootInputState {
    bool isPressed = false;
};

/**
 * @struct ChargeShotInputState
 * @brief Singleton for tracking charge shot button state
 */
struct ChargeShotInputState {
    bool isPressed = false;
    bool shouldFireShot = false;
    shared::ChargeLevel releasedChargeLevel = shared::ChargeLevel::None;
};

/**
 * @struct ChargeShotVisual
 * @brief Visual state component for charge shot effect on player ship
 */
struct ChargeShotVisual {
    float glowIntensity = 0.0F;
    float shakeIntensity = 0.0F;
    float shakeTimer = 0.0F;
    bool shouldShake = false;
    bool wasCharging = false;
    bool isChargingShake = false;
    float chargeShakeIntensity = 0.0F;

    static constexpr uint8_t kLevel1Color[3] = {100, 150, 255};   // Blue glow
    static constexpr uint8_t kLevel2Color[3] = {255, 200, 100};   // Orange glow
    static constexpr uint8_t kLevel3Color[3] = {255, 100, 100};   // Red glow

    static constexpr float kMaxShakeIntensity = 8.0F;
    static constexpr float kShakeDuration = 0.3F;
    static constexpr float kLevel1ShakeIntensity = 1.0F;
    static constexpr float kLevel2ShakeIntensity = 2.5F;
    static constexpr float kLevel3ShakeIntensity = 5.0F;

    ChargeShotVisual() = default;

    /**
     * @brief Get glow color for a charge level
     * @param level The charge level
     * @return Tuple of (r, g, b)
     */
    [[nodiscard]] static std::tuple<uint8_t, uint8_t, uint8_t> getGlowColor(
        shared::ChargeLevel level) noexcept {
        switch (level) {
            case shared::ChargeLevel::Level1:
                return {kLevel1Color[0], kLevel1Color[1], kLevel1Color[2]};
            case shared::ChargeLevel::Level2:
                return {kLevel2Color[0], kLevel2Color[1], kLevel2Color[2]};
            case shared::ChargeLevel::Level3:
                return {kLevel3Color[0], kLevel3Color[1], kLevel3Color[2]};
            default:
                return {255, 255, 255};
        }
    }

    /**
     * @brief Update glow intensity and charge shake based on charge level
     * @param level Current charge level
     * @param isCharging Whether currently charging
     */
    void updateGlow(shared::ChargeLevel level, bool isCharging = false) noexcept {
        switch (level) {
            case shared::ChargeLevel::Level1:
                glowIntensity = 0.3F;
                chargeShakeIntensity = kLevel1ShakeIntensity;
                break;
            case shared::ChargeLevel::Level2:
                glowIntensity = 0.6F;
                chargeShakeIntensity = kLevel2ShakeIntensity;
                break;
            case shared::ChargeLevel::Level3:
                glowIntensity = 1.0F;
                chargeShakeIntensity = kLevel3ShakeIntensity;
                break;
            default:
                glowIntensity = 0.0F;
                chargeShakeIntensity = 0.0F;
                break;
        }
        isChargingShake = isCharging && (level != shared::ChargeLevel::None);
    }

    /**
     * @brief Update shake effect timer
     * @param dt Delta time
     */
    void updateShake(float dt) noexcept {
        if (shouldShake && shakeTimer > 0.0F) {
            shakeTimer -= dt;
            if (shakeTimer <= 0.0F) {
                shouldShake = false;
                shakeIntensity = 0.0F;
            }
        }
    }

    /**
     * @brief Get current effective shake intensity
     * @return Current shake intensity (release shake or charge shake)
     */
    [[nodiscard]] float getEffectiveShakeIntensity() const noexcept {
        if (shouldShake) {
            return shakeIntensity;
        }
        if (isChargingShake) {
            return chargeShakeIntensity;
        }
        return 0.0F;
    }

    /**
     * @brief Check if any shake should be applied
     * @return true if shaking
     */
    [[nodiscard]] bool isShaking() const noexcept {
        return shouldShake || isChargingShake;
    }

    /**
     * @brief Trigger max charge screen shake
     */
    void triggerMaxChargeShake() noexcept {
        shouldShake = true;
        shakeIntensity = kMaxShakeIntensity;
        shakeTimer = kShakeDuration;
    }

    /**
     * @brief Reset visual state
     */
    void reset() noexcept {
        glowIntensity = 0.0F;
        isChargingShake = false;
        chargeShakeIntensity = 0.0F;
    }
};

/**
 * @struct ChargeBarUI
 * @brief UI component for displaying charge level progress bar
 */
struct ChargeBarUI {
    float chargePercent = 0.0F;
    float displayPercent = 0.0F;
    float smoothingSpeed = 5.0F;
    bool isVisible = true;

    float barWidth = 100.0F;
    float barHeight = 10.0F;
    float offsetX = -50.0F;
    float offsetY = -40.0F;

    ChargeBarUI() = default;

    /**
     * @brief Set target charge percent
     * @param percent Target percent (0.0 - 1.0)
     */
    void setChargePercent(float percent) noexcept {
        chargePercent = percent;
    }

    /**
     * @brief Update smooth display value
     * @param dt Delta time
     */
    void update(float dt) noexcept {
        float diff = chargePercent - displayPercent;
        displayPercent += diff * smoothingSpeed * dt;

        if (displayPercent < 0.0F) displayPercent = 0.0F;
        if (displayPercent > 1.0F) displayPercent = 1.0F;
    }

    /**
     * @brief Get the color for current charge level
     * @return Tuple of (r, g, b)
     */
    [[nodiscard]] std::tuple<uint8_t, uint8_t, uint8_t> getBarColor()
        const noexcept {
        if (chargePercent >= 0.9F) {
            return {255, 100, 100};  // Red for max
        } else if (chargePercent >= 0.6F) {
            return {255, 200, 100};  // Orange
        } else if (chargePercent >= 0.3F) {
            return {100, 150, 255};  // Blue
        }
        return {128, 128, 128};  // Gray when empty
    }
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_CHARGESHOTVISUALCOMPONENT_HPP_
