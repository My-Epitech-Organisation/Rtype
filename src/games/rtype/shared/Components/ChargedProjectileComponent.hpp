/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargedProjectileComponent - Component for charged shot projectiles
*/

#ifndef SRC_GAMES_RTYPE_SHARED_COMPONENTS_CHARGEDPROJECTILECOMPONENT_HPP_
#define SRC_GAMES_RTYPE_SHARED_COMPONENTS_CHARGEDPROJECTILECOMPONENT_HPP_

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
    explicit ChargedProjectileComponent(ChargeLevel chargeLevel);

    /**
     * @brief Check if projectile can hit a specific entity
     * @param entityNetworkId Network ID of the entity
     * @return true if entity hasn't been hit yet
     */
    [[nodiscard]] bool canHitEntity(uint32_t entityNetworkId) const noexcept;

    /**
     * @brief Register a hit on an entity
     * @param entityNetworkId Network ID of the hit entity
     * @return true if projectile should be destroyed (no more pierces)
     */
    bool registerHit(uint32_t entityNetworkId) noexcept;

    /**
     * @brief Update animation state
     * @param deltaTime Time elapsed since last update
     */
    void updateAnimation(float deltaTime) noexcept;

    /**
     * @brief Get projectile size multiplier based on charge level
     * @return Size multiplier
     */
    [[nodiscard]] float getSizeMultiplier() const noexcept;

    /**
     * @brief Get current animation frame (0-based)
     * @return Current frame index
     */
    [[nodiscard]] int32_t getFrame() const noexcept;
};

/**
 * @struct ChargedProjectileTag
 * @brief Tag for charged shot projectiles
 */
struct ChargedProjectileTag {};

}  // namespace rtype::games::rtype::shared

#endif  // SRC_GAMES_RTYPE_SHARED_COMPONENTS_CHARGEDPROJECTILECOMPONENT_HPP_
