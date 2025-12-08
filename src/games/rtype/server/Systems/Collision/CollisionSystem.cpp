/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CollisionSystem - Server-side AABB collision handling
*/

#include "CollisionSystem.hpp"

#include <vector>

#include "../../../shared/Components.hpp"
#include "../../../shared/Systems/Collision/AABB.hpp"

namespace rtype::games::rtype::server {

using shared::BoundingBoxComponent;
using shared::DestroyTag;
using shared::EnemyTag;
using shared::PlayerTag;
using shared::ProjectileTag;
using shared::TransformComponent;
using shared::collision::overlaps;

CollisionSystem::CollisionSystem() : ASystem("CollisionSystem") {}

void CollisionSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    // Collect projectiles
    std::vector<ECS::Entity> projectiles;
    auto projView = registry.view<TransformComponent, BoundingBoxComponent,
                                  ProjectileTag>();
    projView.each([&projectiles](ECS::Entity entity, auto&, auto&, auto&) {
        projectiles.push_back(entity);
    });

    // Enemy and player views for collision checks
    auto enemyView = registry.view<TransformComponent, BoundingBoxComponent,
                                   EnemyTag>();
    auto playerView = registry.view<TransformComponent, BoundingBoxComponent,
                                    PlayerTag>();

    for (ECS::Entity projectile : projectiles) {
        auto& projTransform = registry.getComponent<TransformComponent>(projectile);
        auto& projBox = registry.getComponent<BoundingBoxComponent>(projectile);

        // Skip already marked projectiles
        const bool projectileDestroyed =
            registry.hasComponent<DestroyTag>(projectile);

        if (!projectileDestroyed) {
            enemyView.each([&](ECS::Entity enemy, const TransformComponent& enemyTransform,
                               const BoundingBoxComponent& enemyBox, auto&) {
                if (registry.hasComponent<DestroyTag>(enemy)) return;
                if (overlaps(projTransform, projBox, enemyTransform, enemyBox)) {
                    registry.emplaceComponent<DestroyTag>(enemy, DestroyTag{});
                    registry.emplaceComponent<DestroyTag>(projectile,
                                                          DestroyTag{});
                }
            });
        }

        // Re-check projectile status to avoid double marking
        if (registry.hasComponent<DestroyTag>(projectile)) {
            continue;
        }

        playerView.each([&](ECS::Entity player, const TransformComponent& playerTransform,
                            const BoundingBoxComponent& playerBox, auto&) {
            if (registry.hasComponent<DestroyTag>(player)) return;
            if (overlaps(projTransform, projBox, playerTransform, playerBox)) {
                registry.emplaceComponent<DestroyTag>(player, DestroyTag{});
                registry.emplaceComponent<DestroyTag>(projectile, DestroyTag{});
            }
        });
    }
}

}  // namespace rtype::games::rtype::server
