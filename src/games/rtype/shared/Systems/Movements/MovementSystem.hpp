/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MovementSystem - Shared movement logic
*/

#pragma once

#include "../../Components/TransformComponent.hpp"
#include "../../Components/VelocityComponent.hpp"
#include "../../../engine/ISystem.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class MovementSystem
 * @brief System that updates entity positions based on velocity
 *
 * This is a shared system used by both client and server.
 * It applies velocity to transform each frame.
 */
class MovementSystem : public ISystem {
   public:
    MovementSystem() = default;

    /**
     * @brief Update all entities with Transform and Velocity components
     * @param registry ECS registry containing entities
     * @param deltaTime Time elapsed since last update
     */
    void update(ECS::Registry& registry, float deltaTime) override;

    [[nodiscard]] const std::string getName() const noexcept override {
        return "MovementSystem";
    }
};

/**
 * @brief Standalone function to update a single entity's movement
 * @param transform Transform component to update
 * @param velocity Velocity component to read from
 * @param deltaTime Time elapsed since last update
 *
 * Useful for cases where you need to update a single entity
 * without going through the full system.
 */
void updateMovement(TransformComponent& transform,
                    const VelocityComponent& velocity, float deltaTime);

}  // namespace rtype::games::rtype::shared
