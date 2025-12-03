/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MovementSystem - Shared movement logic implementation
*/

#include "MovementSystem.hpp"

namespace rtype::games::rtype::shared {

void MovementSystem::update(ECS::Registry& registry, float deltaTime) {
    auto view = registry.view<TransformComponent, VelocityComponent>();

    view.each([deltaTime](ECS::Entity /*entity*/, TransformComponent& transform,
                          const VelocityComponent& velocity) {
        updateMovement(transform, velocity, deltaTime);
    });
}

void updateMovement(TransformComponent& transform,
                    const VelocityComponent& velocity, float deltaTime) {
    transform.x += velocity.vx * deltaTime;
    transform.y += velocity.vy * deltaTime;
}

}  // namespace rtype::games::rtype::shared
