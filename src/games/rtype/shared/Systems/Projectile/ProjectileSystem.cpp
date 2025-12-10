/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ProjectileSystem - Handles projectile movement and behavior
*/

#include "ProjectileSystem.hpp"

namespace rtype::games::rtype::shared {

void ProjectileSystem::update(ECS::Registry& registry, float deltaTime) {
    if (deltaTime < 0) {
        return;
    }

    auto view = registry.view<Position, VelocityComponent, ProjectileTag>();

    view.each([deltaTime](ECS::Entity /*entity*/, Position& position,
                          const VelocityComponent& velocity,
                          const ProjectileTag& /*tag*/) {
        position.x += velocity.vx * deltaTime;
        position.y += velocity.vy * deltaTime;
    });
}

}  // namespace rtype::games::rtype::shared
