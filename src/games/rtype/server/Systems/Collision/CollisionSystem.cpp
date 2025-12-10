/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CollisionSystem - Server-side AABB collision handling
*/

#include "CollisionSystem.hpp"

#include <cstdint>
#include <utility>
#include <vector>

#include "../../../shared/Components.hpp"
#include "../../../shared/Systems/Collision/AABB.hpp"

namespace rtype::games::rtype::server {

using shared::BoundingBoxComponent;
using shared::DestroyTag;
using shared::EnemyTag;
using shared::EntityType;
using shared::HealthComponent;
using shared::NetworkIdComponent;
using shared::PlayerTag;
using shared::ProjectileTag;
using shared::TransformComponent;
using shared::collision::overlaps;

namespace {
constexpr int32_t kPlayerCollisionDamage = 1;
}

CollisionSystem::CollisionSystem(EventEmitter emitter)
    : ASystem("CollisionSystem"), _emitEvent(std::move(emitter)) {}

void CollisionSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    std::vector<ECS::Entity> projectiles;
    auto projView =
        registry
            .view<TransformComponent, BoundingBoxComponent, ProjectileTag>();
    projView.each([&projectiles](ECS::Entity entity, auto&, auto&, auto&) {
        projectiles.push_back(entity);
    });

    auto enemyView =
        registry.view<TransformComponent, BoundingBoxComponent, EnemyTag>();
    auto playerView =
        registry.view<TransformComponent, BoundingBoxComponent, PlayerTag>();

    for (ECS::Entity projectile : projectiles) {
        auto& projTransform =
            registry.getComponent<TransformComponent>(projectile);
        auto& projBox = registry.getComponent<BoundingBoxComponent>(projectile);

        const bool projectileDestroyed =
            registry.hasComponent<DestroyTag>(projectile);

        if (!projectileDestroyed) {
            enemyView.each([&](ECS::Entity enemy,
                               const TransformComponent& enemyTransform,
                               const BoundingBoxComponent& enemyBox, auto&) {
                if (registry.hasComponent<DestroyTag>(enemy)) return;
                if (overlaps(projTransform, projBox, enemyTransform,
                             enemyBox)) {
                    registry.emplaceComponent<DestroyTag>(enemy, DestroyTag{});
                    registry.emplaceComponent<DestroyTag>(projectile,
                                                          DestroyTag{});
                }
            });
        }

        if (registry.hasComponent<DestroyTag>(projectile)) {
            continue;
        }

        playerView.each([&](ECS::Entity player,
                            const TransformComponent& playerTransform,
                            const BoundingBoxComponent& playerBox, auto&) {
            if (registry.hasComponent<DestroyTag>(player)) return;
            if (!overlaps(projTransform, projBox, playerTransform, playerBox)) {
                return;
            }

            bool destroyedProjectile = false;
            if (registry.hasComponent<HealthComponent>(player)) {
                auto& health = registry.getComponent<HealthComponent>(player);
                if (health.current > 0) {
                    health.takeDamage(kPlayerCollisionDamage);
                }

                const bool isDead = !health.isAlive();

                if (_emitEvent &&
                    registry.hasComponent<NetworkIdComponent>(player)) {
                    const auto& netId =
                        registry.getComponent<NetworkIdComponent>(player);
                    if (netId.isValid()) {
                        engine::GameEvent event{};
                        event.type = engine::GameEventType::EntityHealthChanged;
                        event.entityNetworkId = netId.networkId;
                        event.entityType =
                            static_cast<uint8_t>(EntityType::Player);
                        event.healthCurrent = health.current;
                        event.healthMax = health.max;
                        _emitEvent(event);
                    }
                }

                if (isDead) {
                    registry.emplaceComponent<DestroyTag>(player, DestroyTag{});
                }
                destroyedProjectile = true;
            } else {
                registry.emplaceComponent<DestroyTag>(player, DestroyTag{});
                destroyedProjectile = true;
            }

            if (destroyedProjectile) {
                registry.emplaceComponent<DestroyTag>(projectile, DestroyTag{});
            }
        });
    }
}

}  // namespace rtype::games::rtype::server
