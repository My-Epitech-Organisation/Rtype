/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ProjectileSystem - Handles projectile movement and behavior implementation
*/

#include "ProjectileSystem.hpp"

namespace rtype::games::rtype::shared {

void ProjectileSystem::update(ECS::Registry& registry, float deltaTime) {
    auto view = registry.view<ProjectileTag, Position, VelocityComponent>();

    view.each([deltaTime](ECS::Entity /*entity*/, const ProjectileTag& /*tag*/,
                          Position& position,
                          const VelocityComponent& velocity) {
        position.x += velocity.vx * deltaTime;
        position.y += velocity.vy * deltaTime;
    });
}

}  // namespace rtype::games::rtype::shared
