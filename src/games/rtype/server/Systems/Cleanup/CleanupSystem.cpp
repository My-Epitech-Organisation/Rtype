/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CleanupSystem - Marks out-of-bounds entities for destruction
*/

#include "CleanupSystem.hpp"

#include "../../../shared/Components.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

using shared::DestroyTag;
using shared::EnemyTag;
using shared::ProjectileTag;
using shared::TransformComponent;

CleanupSystem::CleanupSystem(EventEmitter emitter, CleanupConfig config)
    : ASystem("CleanupSystem"),
      _emitEvent(std::move(emitter)),
      _config(config) {}

void CleanupSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    auto isOutOfBounds = [this](const TransformComponent& transform) {
        return transform.x < _config.leftBoundary ||
               transform.x > _config.rightBoundary ||
               transform.y < _config.topBoundary ||
               transform.y > _config.bottomBoundary;
    };

    auto enemyView = registry.view<TransformComponent, EnemyTag>();

    enemyView.each([this, &registry, &isOutOfBounds](ECS::Entity entity,
                                const TransformComponent& transform,
                                const EnemyTag& /*tag*/) {
        if (isOutOfBounds(transform) && !registry.hasComponent<DestroyTag>(entity)) {
            LOG_DEBUG("[CleanupSystem] Enemy "
                      << entity.id << " escaped out of bounds at ("
                      << transform.x << ", " << transform.y
                      << ") - Damaging all players");
            auto playerView =
                registry.view<shared::PlayerTag, shared::HealthComponent,
                              shared::NetworkIdComponent>();
            playerView.each(
                [this, &registry](ECS::Entity playerEntity,
                                  const shared::PlayerTag&,
                                  shared::HealthComponent& health,
                                  const shared::NetworkIdComponent& netId) {
                    int32_t oldHealth = health.current;
                    health.current -= 30;
                    if (health.current < 0) {
                        health.current = 0;
                    }

                    LOG_INFO("[CleanupSystem] Player "
                             << netId.networkId
                             << " took 30 damage (enemy escaped): " << oldHealth
                             << " -> " << health.current);
                    engine::GameEvent event{};
                    event.type = engine::GameEventType::EntityHealthChanged;
                    event.entityNetworkId = netId.networkId;
                    event.healthCurrent = health.current;
                    event.healthMax = health.max;
                    _emitEvent(event);

                    if (health.current <= 0 &&
                        !registry.hasComponent<DestroyTag>(playerEntity)) {
                        registry.emplaceComponent<DestroyTag>(playerEntity,
                                                              DestroyTag{});
                    }
                });

            registry.emplaceComponent<DestroyTag>(entity, DestroyTag{});
        }
    });

    auto projectileView = registry.view<TransformComponent, ProjectileTag>();

    projectileView.each([this, &registry, &isOutOfBounds](ECS::Entity entity,
                                const TransformComponent& transform,
                                const ProjectileTag& /*tag*/) {
        if (isOutOfBounds(transform) && !registry.hasComponent<DestroyTag>(entity)) {
            registry.emplaceComponent<DestroyTag>(entity, DestroyTag{});
        }
    });
}

}  // namespace rtype::games::rtype::server
