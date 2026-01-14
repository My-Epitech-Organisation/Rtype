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

    static constexpr uint8_t kLevel1Color[3] = {100, 150, 255};  // Blue glow
    static constexpr uint8_t kLevel2Color[3] = {255, 200, 100};  // Orange glow
    static constexpr uint8_t kLevel3Color[3] = {255, 100, 100};  // Red glow

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
        shared::ChargeLevel level) noexcept;

    /**
     * @brief Update glow intensity and charge shake based on charge level
     * @param level Current charge level
     * @param isCharging Whether currently charging
     */
    void updateGlow(shared::ChargeLevel level,
                    bool isCharging = false) noexcept;

    /**
     * @brief Update shake effect timer
     * @param dt Delta time
     */
    void updateShake(float dt) noexcept;

    /**
     * @brief Get current effective shake intensity
     * @return Current shake intensity (release shake or charge shake)
     */
    [[nodiscard]] float getEffectiveShakeIntensity() const noexcept;

    /**
     * @brief Check if any shake should be applied
     * @return true if shaking
     */
    [[nodiscard]] bool isShaking() const noexcept;

    /**
     * @brief Trigger max charge screen shake
     */
    void triggerMaxChargeShake() noexcept;

    /**
     * @brief Reset visual state
     */
    void reset() noexcept;
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
    void setChargePercent(float percent) noexcept;

    /**
     * @brief Update smooth display value
     * @param dt Delta time
     */
    void update(float dt) noexcept;

    /**
     * @brief Get the color for current charge level
     * @return Tuple of (r, g, b)
     */
    [[nodiscard]] std::tuple<uint8_t, uint8_t, uint8_t> getBarColor()
        const noexcept;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_CHARGESHOTVISUALCOMPONENT_HPP_
