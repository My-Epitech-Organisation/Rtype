/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LifetimeSystem - Manages entity lifecycle based on lifetime implementation
*/

#include "LifetimeSystem.hpp"

namespace rtype::games::rtype::shared {

void LifetimeSystem::update(ECS::Registry& registry, float deltaTime) {
    auto view = registry.view<LifetimeComponent>();

    view.each([deltaTime, &registry](auto entity, LifetimeComponent& lifetime) {
        lifetime.remainingTime -= deltaTime;
        if (lifetime.remainingTime <= 0.0F) {
            registry.killEntity(entity);
        }
    });
}

}  // namespace rtype::games::rtype::shared
