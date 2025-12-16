/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MovementSystem - Shared movement logic implementation
*/

#include "MovementSystem.hpp"

namespace rtype::games::rtype::shared {

void MovementSystem::update(ECS::Registry& registry, float deltaTime) {
    const size_t entityCount = registry.countComponents<TransformComponent>();

    if (entityCount >= PARALLEL_THRESHOLD) {
        auto view = registry.parallelView<TransformComponent, VelocityComponent>();
        view.each([deltaTime](ECS::Entity /*entity*/, TransformComponent& transform,
                              const VelocityComponent& velocity) {
            transform.x += velocity.vx * deltaTime;
            transform.y += velocity.vy * deltaTime;
        });
    } else {
        auto view = registry.view<TransformComponent, VelocityComponent>();
        view.each([deltaTime](ECS::Entity /*entity*/, TransformComponent& transform,
                              const VelocityComponent& velocity) {
            transform.x += velocity.vx * deltaTime;
            transform.y += velocity.vy * deltaTime;
        });
    }
}

}  // namespace rtype::games::rtype::shared
