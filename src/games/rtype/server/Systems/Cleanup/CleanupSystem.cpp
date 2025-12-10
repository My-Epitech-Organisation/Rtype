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
using shared::TransformComponent;

CleanupSystem::CleanupSystem(EventEmitter emitter, CleanupConfig config)
    : ASystem("CleanupSystem"),
      _emitEvent(std::move(emitter)),
      _config(config) {}

void CleanupSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    auto view = registry.view<TransformComponent, EnemyTag>();

    view.each([this, &registry](ECS::Entity entity,
                                const TransformComponent& transform,
                                const EnemyTag& /*tag*/) {
        bool outOfBounds = transform.x < _config.leftBoundary ||
                           transform.x > _config.rightBoundary ||
                           transform.y < _config.topBoundary ||
                           transform.y > _config.bottomBoundary;

        if (outOfBounds && !registry.hasComponent<DestroyTag>(entity)) {
            LOG_DEBUG("[CleanupSystem] Entity " << entity.id << " out of bounds at ("
                      << transform.x << ", " << transform.y << ")");
            registry.emplaceComponent<DestroyTag>(entity, DestroyTag{});
        }
    });
}

}  // namespace rtype::games::rtype::server
