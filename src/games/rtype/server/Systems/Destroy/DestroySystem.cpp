/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DestroySystem - Destroys entities marked with DestroyTag
*/

#include "DestroySystem.hpp"

#include <vector>

#include "../../../shared/Components.hpp"

namespace rtype::games::rtype::server {

using shared::DestroyTag;
using shared::EnemyTag;
using shared::EntityType;
using shared::NetworkIdComponent;

DestroySystem::DestroySystem(EventEmitter emitter,
                             EnemyCountUpdater enemyCountDecrementer)
    : _emitEvent(std::move(emitter)),
      _decrementEnemyCount(std::move(enemyCountDecrementer)) {}

void DestroySystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    std::vector<ECS::Entity> toDestroy;
    auto view = registry.view<DestroyTag>();

    view.each([&toDestroy](ECS::Entity entity, const DestroyTag& /*tag*/) {
        toDestroy.push_back(entity);
    });

    for (ECS::Entity entity : toDestroy) {
        uint32_t networkId = 0;
        bool isEnemy = false;

        if (registry.hasComponent<NetworkIdComponent>(entity)) {
            networkId =
                registry.getComponent<NetworkIdComponent>(entity).networkId;
        }
        if (registry.hasComponent<EnemyTag>(entity)) {
            isEnemy = true;
        }
        if (isEnemy) {
            _decrementEnemyCount();
        }
        if (networkId != 0) {
            engine::GameEvent event{};
            event.type = engine::GameEventType::EntityDestroyed;
            event.entityNetworkId = networkId;
            event.entityType = isEnemy
                                   ? static_cast<uint8_t>(EntityType::Enemy)
                                   : static_cast<uint8_t>(EntityType::Player);
            _emitEvent(event);
        }
        registry.killEntity(entity);
    }
}

}  // namespace rtype::games::rtype::server
