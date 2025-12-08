/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CollisionSystem - Server-side AABB collision handling
*/

#pragma once

#include <rtype/engine.hpp>

namespace rtype::games::rtype::server {

/**
 * @brief Detects projectile collisions against enemies and players using AABB and marks entities for destruction.
 */
class CollisionSystem : public ::rtype::engine::ASystem {
   public:
    CollisionSystem();

    void update(ECS::Registry& registry, float deltaTime) override;
};

}  // namespace rtype::games::rtype::server
