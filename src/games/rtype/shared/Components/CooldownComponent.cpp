/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CooldownComponent - Implementation of cooldown and charge components
*/

#include "Components/CooldownComponent.hpp"

namespace rtype::games::rtype::shared {

// ShootCooldownComponent implementation

ShootCooldownComponent::ShootCooldownComponent(float cooldown)
    : cooldownTime(cooldown), currentCooldown(0.0F) {}

bool ShootCooldownComponent::canShoot() const noexcept {
    return currentCooldown <= 0.0F;
}

void ShootCooldownComponent::triggerCooldown() noexcept {
    currentCooldown = cooldownTime;
}

void ShootCooldownComponent::update(float deltaTime) noexcept {
    if (currentCooldown > 0.0F) {
        currentCooldown -= deltaTime;
        if (currentCooldown < 0.0F) {
            currentCooldown = 0.0F;
        }
    }
}

void ShootCooldownComponent::reset() noexcept { currentCooldown = 0.0F; }

void ShootCooldownComponent::setCooldownTime(float newCooldown) noexcept {
    cooldownTime = newCooldown;
}

void ShootCooldownComponent::setWeaponSlot(uint8_t slot) noexcept {
    currentWeaponSlot = slot;
}

ChargeComponent::ChargeComponent(float rate) : chargeRate(rate) {}

void ChargeComponent::startCharging() noexcept {
    isCharging = true;
    wasCharging = true;
}

ChargeLevel ChargeComponent::release() noexcept {
    isCharging = false;
    wasCharging = false;
    ChargeLevel level = currentLevel;
    currentCharge = 0.0F;
    currentLevel = ChargeLevel::None;
    return level;
}

void ChargeComponent::update(float deltaTime) noexcept {
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

bool ChargeComponent::isPoweredShot() const noexcept {
    return currentCharge >= minChargeThreshold;
}

float ChargeComponent::getChargePercent() const noexcept {
    return currentCharge / maxCharge;
}

int32_t ChargeComponent::getDamageForLevel(ChargeLevel level) noexcept {
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

int32_t ChargeComponent::getPierceCountForLevel(ChargeLevel level) noexcept {
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

}  // namespace rtype::games::rtype::shared
