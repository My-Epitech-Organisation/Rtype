/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CollisionSystem - Server-side collision handling using QuadTree + AABB
*/

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_set>
#include <utility>

#include <rtype/engine.hpp>

#include "../../../shared/Components/DamageOnContactComponent.hpp"
#include "../../../shared/Components/PowerUpComponent.hpp"
#include "../../../shared/Systems/Collision/QuadTreeSystem.hpp"

namespace rtype::games::rtype::server {

/**
 * @brief Detects projectile collisions against enemies and players using
 * QuadTree spatial partitioning + AABB fine collision detection.
 *
 * Uses the QuadTree for broad-phase collision detection (O(n log n))
 * and AABB overlap checks for narrow-phase collision validation.
 *
 * Uses CommandBuffer pour différer les modifications d'entités durant
 * l'itération.
 */
class CollisionSystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    /**
     * @brief Construct CollisionSystem with event emitter and world bounds
     * @param emitter Function to emit game events (e.g., health changes)
     * @param worldWidth Width of the game world (default: 1920)
     * @param worldHeight Height of the game world (default: 1080)
     */
    explicit CollisionSystem(EventEmitter emitter, float worldWidth = 1920.0F,
                             float worldHeight = 1080.0F);

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    /**
     * @brief Handle collision between a projectile and a target entity
     * @param registry ECS registry
     * @param cmdBuffer Command buffer pour différer les modifications
     * @param projectile The projectile entity
     * @param target The target entity (enemy or player)
     * @param isTargetPlayer True if target is a player, false if enemy
     */
    void handleProjectileCollision(ECS::Registry& registry,
                                   ECS::CommandBuffer& cmdBuffer,
                                   ECS::Entity projectile, ECS::Entity target,
                                   bool isTargetPlayer);

    /**
     * @brief Handle collision between an enemy and a player
     * @param registry ECS registry
     * @param cmdBuffer Command buffer
     * @param enemy The enemy entity
     * @param player The player entity
     */
    void handleEnemyPlayerCollision(ECS::Registry& registry,
                                    ECS::CommandBuffer& cmdBuffer,
                                    ECS::Entity enemy, ECS::Entity player);

    void handlePickupCollision(ECS::Registry& registry,
                               ECS::CommandBuffer& cmdBuffer,
                               ECS::Entity player, ECS::Entity pickup);

    void handleObstacleCollision(ECS::Registry& registry,
                                 ECS::CommandBuffer& cmdBuffer,
                                 ECS::Entity obstacle, ECS::Entity other,
                                 bool otherIsPlayer);

    /**
     * @brief Handle collision between a laser beam and an enemy
     * @param registry ECS registry
     * @param cmdBuffer Command buffer
     * @param laser The laser beam entity
     * @param enemy The enemy entity
     * @param deltaTime Frame delta time for DPS calculation
     */
    void handleLaserEnemyCollision(ECS::Registry& registry,
                                   ECS::CommandBuffer& cmdBuffer,
                                   ECS::Entity laser, ECS::Entity enemy,
                                   float deltaTime);

    /**
     * @brief Handle player picking up an orphan Force Pod
     * @param registry ECS registry
     * @param cmdBuffer Command buffer
     * @param forcePod The orphan Force Pod entity
     * @param player The player entity
     */
    void handleOrphanForcePodPickup(ECS::Registry& registry,
                                    ECS::CommandBuffer& cmdBuffer,
                                    ECS::Entity forcePod, ECS::Entity player);

    EventEmitter _emitEvent;
    std::unique_ptr<shared::QuadTreeSystem> _quadTreeSystem;

    /// Tracks laser-enemy pairs damaged this frame to prevent double hits
    std::unordered_set<uint64_t> _laserDamagedThisFrame;

    /// Tracks obstacle-entity pairs that have collided to prevent repeated
    /// damage
    std::unordered_set<uint64_t> _obstacleCollidedThisFrame;
};

}  // namespace rtype::games::rtype::server
