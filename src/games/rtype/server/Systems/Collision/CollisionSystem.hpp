/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CollisionSystem - Server-side collision handling using QuadTree + AABB
*/

#pragma once

#include <functional>
#include <memory>
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

    void handleForcePodCollision(ECS::Registry& registry,
                                 ECS::CommandBuffer& cmdBuffer,
                                 ECS::Entity forcePod, ECS::Entity target);

    EventEmitter _emitEvent;
    std::unique_ptr<shared::QuadTreeSystem> _quadTreeSystem;
};

}  // namespace rtype::games::rtype::server
