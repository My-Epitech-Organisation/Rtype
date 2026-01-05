/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MovementSystem - Shared movement logic
*/

#pragma once

#include <rtype/engine.hpp>

#include "../../Components/TransformComponent.hpp"
#include "../../Components/VelocityComponent.hpp"

namespace rtype::games::rtype::shared {
/**
 * @class MovementSystem
 * @brief System that updates entity positions based on velocity
 *
 * This is a shared system used by both client and server.
 * It applies velocity to transform each frame.
 */
class MovementSystem : public ::rtype::engine::ASystem {
   public:
    MovementSystem() : ASystem("MovementSystem") {}

    /**
     * @brief Update all entities with Transform and Velocity components
     * @param registry ECS registry containing entities
     * @param deltaTime Time elapsed since last update
     */
    void update(ECS::Registry& registry, float deltaTime) override;
};

}  // namespace rtype::games::rtype::shared
