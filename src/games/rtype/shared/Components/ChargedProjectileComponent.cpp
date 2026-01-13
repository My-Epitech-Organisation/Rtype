/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargedProjectileComponent - Implementation of charged projectile component
*/

#include "Components/ChargedProjectileComponent.hpp"

namespace rtype::games::rtype::shared {

ChargedProjectileComponent::ChargedProjectileComponent(ChargeLevel chargeLevel)
    : level(chargeLevel),
      damage(ChargeComponent::getDamageForLevel(chargeLevel)),
      maxPierceCount(ChargeComponent::getPierceCountForLevel(chargeLevel)) {}

bool ChargedProjectileComponent::canHitEntity(
    uint32_t entityNetworkId) const noexcept {
    return hitEntities.find(entityNetworkId) == hitEntities.end();
}

bool ChargedProjectileComponent::registerHit(
    uint32_t entityNetworkId) noexcept {
    hitEntities.insert(entityNetworkId);
    currentPierceCount++;
    return currentPierceCount > maxPierceCount;
}

void ChargedProjectileComponent::updateAnimation(float deltaTime) noexcept {
    animationTimer += deltaTime;

    if (isSpawning) {
        if (animationTimer >= kSpawnFrameDuration) {
            animationTimer = 0.0F;
            currentFrame++;
            if (currentFrame > kSpawnEndFrame) {
                isSpawning = false;
                isLooping = true;
                isReversing = false;
                currentFrame = kLoopStartFrame;
            }
        }
    } else if (isLooping) {
        if (animationTimer >= kLoopFrameDuration) {
            animationTimer = 0.0F;
            if (!isReversing) {
                currentFrame++;
                if (currentFrame >= kLoopEndFrame) {
                    isReversing = true;
                }
            } else {
                currentFrame--;
                if (currentFrame <= kLoopStartFrame) {
                    isReversing = false;
                }
            }
        }
    }
}

float ChargedProjectileComponent::getSizeMultiplier() const noexcept {
    switch (level) {
        case ChargeLevel::Level1:
            return kLevel1Size;
        case ChargeLevel::Level2:
            return kLevel2Size;
        case ChargeLevel::Level3:
            return kLevel3Size;
        default:
            return 1.0F;
    }
}

int32_t ChargedProjectileComponent::getFrame() const noexcept {
    return currentFrame;
}

}  // namespace rtype::games::rtype::shared
