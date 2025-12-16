/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ProjectileSystem - Handles projectile movement and behavior
*/

#include "ProjectileSystem.hpp"

namespace rtype::games::rtype::shared {

namespace {
    constexpr size_t PARALLEL_THRESHOLD = 200;
}

void ProjectileSystem::update(ECS::Registry& registry, float deltaTime) {
    if (deltaTime < 0) {
        return;
    }
    const size_t entityCount = registry.countComponents<ProjectileTag>();
    if (entityCount >= PARALLEL_THRESHOLD) {
        auto view = registry.parallelView<TransformComponent, VelocityComponent, ProjectileTag>();
        view.each([deltaTime](ECS::Entity /*entity*/, TransformComponent& position,
                              const VelocityComponent& velocity,
                              const ProjectileTag& /*tag*/) {
            position.x += velocity.vx * deltaTime;
            position.y += velocity.vy * deltaTime;
        });
    } else {
        auto view = registry.view<TransformComponent, VelocityComponent, ProjectileTag>();
        view.each([deltaTime](ECS::Entity /*entity*/, TransformComponent& position,
                              const VelocityComponent& velocity,
                              const ProjectileTag& /*tag*/) {
            position.x += velocity.vx * deltaTime;
            position.y += velocity.vy * deltaTime;
        });
    }
}

}  // namespace rtype::games::rtype::shared
