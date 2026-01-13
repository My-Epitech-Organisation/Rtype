/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargeShotVisualComponent - Implementation of visual components for charge
** shot system
*/

#include "games/rtype/client/Components/ChargeShotVisualComponent.hpp"

namespace rtype::games::rtype::client {

// ChargeShotVisual implementation

std::tuple<uint8_t, uint8_t, uint8_t> ChargeShotVisual::getGlowColor(
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

void ChargeShotVisual::updateGlow(shared::ChargeLevel level,
                                  bool isCharging) noexcept {
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

void ChargeShotVisual::updateShake(float dt) noexcept {
    if (shouldShake && shakeTimer > 0.0F) {
        shakeTimer -= dt;
        if (shakeTimer <= 0.0F) {
            shouldShake = false;
            shakeIntensity = 0.0F;
        }
    }
}

float ChargeShotVisual::getEffectiveShakeIntensity() const noexcept {
    if (shouldShake) {
        return shakeIntensity;
    }
    if (isChargingShake) {
        return chargeShakeIntensity;
    }
    return 0.0F;
}

bool ChargeShotVisual::isShaking() const noexcept {
    return shouldShake || isChargingShake;
}

void ChargeShotVisual::triggerMaxChargeShake() noexcept {
    shouldShake = true;
    shakeIntensity = kMaxShakeIntensity;
    shakeTimer = kShakeDuration;
}

void ChargeShotVisual::reset() noexcept {
    glowIntensity = 0.0F;
    isChargingShake = false;
    chargeShakeIntensity = 0.0F;
}

void ChargeBarUI::setChargePercent(float percent) noexcept {
    chargePercent = percent;
}

void ChargeBarUI::update(float dt) noexcept {
    float diff = chargePercent - displayPercent;
    displayPercent += diff * smoothingSpeed * dt;

    if (displayPercent < 0.0F) displayPercent = 0.0F;
    if (displayPercent > 1.0F) displayPercent = 1.0F;
}

std::tuple<uint8_t, uint8_t, uint8_t> ChargeBarUI::getBarColor()
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

}  // namespace rtype::games::rtype::client
