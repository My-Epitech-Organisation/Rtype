/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargedProjectileComponent - Component for charged shot projectiles
*/

#pragma once

#include <cstdint>
#include <unordered_set>

#include "CooldownComponent.hpp"

namespace rtype::games::rtype::shared {

/**
 * @struct ChargedProjectileComponent
 * @brief Component for charged shot projectiles with pierce tracking
 *
 * Tracks:
 * - Charge level (determines damage and pierce count)
 * - List of already-hit entities (to prevent multi-hit on same enemy)
 * - Animation state for the charged shot visual
 */
struct ChargedProjectileComponent {
    ChargeLevel level = ChargeLevel::None;
    int32_t damage = 0;
    int32_t maxPierceCount = 0;
    int32_t currentPierceCount = 0;
    std::unordered_set<uint32_t> hitEntities;

    float animationTimer = 0.0F;
    int32_t currentFrame = 0;
    bool isSpawning = true;
    bool isLooping = false;
    bool isReversing = false;  // For ping-pong animation

    static constexpr int32_t kTotalFrames = 10;
    static constexpr int32_t kSpawnEndFrame = 5;   // Spawn goes 0->5
    static constexpr int32_t kLoopStartFrame = 0;  // Loop uses all frames
    static constexpr int32_t kLoopEndFrame = 9;
    static constexpr float kSpawnFrameDuration = 0.04F;  // Fast spawn
    static constexpr float kLoopFrameDuration = 0.06F;   // Smooth loop

    static constexpr float kLevel1Size = 1.0F;
    static constexpr float kLevel2Size = 1.5F;
    static constexpr float kLevel3Size = 2.0F;

    ChargedProjectileComponent() = default;

    /**
     * @brief Construct with specific charge level
     * @param chargeLevel The charge level
     */
    explicit ChargedProjectileComponent(ChargeLevel chargeLevel)
        : level(chargeLevel),
          damage(ChargeComponent::getDamageForLevel(chargeLevel)),
          maxPierceCount(ChargeComponent::getPierceCountForLevel(chargeLevel)) {
    }

    /**
     * @brief Check if projectile can hit a specific entity
     * @param entityNetworkId Network ID of the entity
     * @return true if entity hasn't been hit yet
     */
    [[nodiscard]] bool canHitEntity(uint32_t entityNetworkId) const noexcept {
        return hitEntities.find(entityNetworkId) == hitEntities.end();
    }

    /**
     * @brief Register a hit on an entity
     * @param entityNetworkId Network ID of the hit entity
     * @return true if projectile should be destroyed (no more pierces)
     */
    bool registerHit(uint32_t entityNetworkId) noexcept {
        hitEntities.insert(entityNetworkId);
        currentPierceCount++;
        return currentPierceCount > maxPierceCount;
    }

    /**
     * @brief Update animation state
     * @param deltaTime Time elapsed since last update
     */
    void updateAnimation(float deltaTime) noexcept {
        animationTimer += deltaTime;

        if (isSpawning) {
            // Spawn animation: quick expansion 0->5
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
            // Loop animation: ping-pong through all frames (0->9->0->9...)
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

    /**
     * @brief Get projectile size multiplier based on charge level
     * @return Size multiplier
     */
    [[nodiscard]] float getSizeMultiplier() const noexcept {
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

    /**
     * @brief Get current animation frame (0-based)
     * @return Current frame index
     */
    [[nodiscard]] int32_t getFrame() const noexcept { return currentFrame; }
};

/**
 * @struct ChargedProjectileTag
 * @brief Tag for charged shot projectiles
 */
struct ChargedProjectileTag {};

}  // namespace rtype::games::rtype::shared
