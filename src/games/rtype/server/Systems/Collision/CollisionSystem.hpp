/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CollisionSystem - Server-side collision handling using QuadTree + AABB
*/

#pragma once

#include <memory>

#include <rtype/engine.hpp>

#include "../../../shared/Systems/Collision/QuadTreeSystem.hpp"

namespace rtype::games::rtype::server {

/**
 * @brief Detects projectile collisions against enemies and players using
 * QuadTree spatial partitioning + AABB fine collision detection.
 *
 * Uses the QuadTree for broad-phase collision detection (O(n log n))
 * and AABB overlap checks for narrow-phase collision validation.
 */
class CollisionSystem : public ::rtype::engine::ASystem {
   public:
    /**
     * @brief Construct CollisionSystem with world bounds
     * @param worldWidth Width of the game world (default: 1920)
     * @param worldHeight Height of the game world (default: 1080)
     */
    explicit CollisionSystem(float worldWidth = 1920.0F,
                             float worldHeight = 1080.0F);

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    /**
     * @brief Handle collision between a projectile and a target entity
     * @param registry ECS registry
     * @param projectile The projectile entity
     * @param target The target entity (enemy or player)
     * @param isTargetPlayer True if target is a player, false if enemy
     */
    void handleProjectileCollision(ECS::Registry& registry,
                                   ECS::Entity projectile, ECS::Entity target,
                                   bool isTargetPlayer);

    std::unique_ptr<shared::QuadTreeSystem> _quadTreeSystem;
};

}  // namespace rtype::games::rtype::server
