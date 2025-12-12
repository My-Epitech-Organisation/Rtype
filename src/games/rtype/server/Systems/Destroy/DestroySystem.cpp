/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DestroySystem - Destroys entities marked with DestroyTag
*/

#include "DestroySystem.hpp"

#include <vector>

#include <rtype/common.hpp>

#include "../../../shared/Components.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::games::rtype::server {

using shared::DestroyTag;
using shared::EnemyTag;
using shared::EntityType;
using shared::NetworkIdComponent;
using shared::ObstacleTag;
using shared::ProjectileTag;

DestroySystem::DestroySystem(EventEmitter emitter,
                             EnemyCountUpdater enemyCountDecrementer)
    : ASystem("DestroySystem"),
      _emitEvent(std::move(emitter)),
      _decrementEnemyCount(std::move(enemyCountDecrementer)) {}

void DestroySystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    std::vector<ECS::Entity> toDestroy;
    auto view = registry.view<DestroyTag>();

    view.each([&toDestroy](ECS::Entity entity, const DestroyTag& /*tag*/) {
        toDestroy.push_back(entity);
    });

    for (ECS::Entity entity : toDestroy) {
        NetworkIdComponent netIdComp{};
        bool hasNetworkId = false;
        bool isEnemy = false;
        bool isProjectile = false;
        bool isObstacle = false;

        if (registry.hasComponent<NetworkIdComponent>(entity)) {
            netIdComp = registry.getComponent<NetworkIdComponent>(entity);
            hasNetworkId = true;
        }
        if (registry.hasComponent<EnemyTag>(entity)) {
            isEnemy = true;
        }
        if (registry.hasComponent<ProjectileTag>(entity)) {
            isProjectile = true;
        }
        if (registry.hasComponent<ObstacleTag>(entity)) {
            isObstacle = true;
        }
        if (isEnemy) {
            _decrementEnemyCount();
        }
        if (hasNetworkId && netIdComp.isValid()) {
            engine::GameEvent event{};
            event.type = engine::GameEventType::EntityDestroyed;
            event.entityNetworkId = netIdComp.networkId;
            if (isEnemy) {
                event.entityType = static_cast<uint8_t>(::rtype::network::EntityType::Bydos);
            } else if (isProjectile) {
                event.entityType = static_cast<uint8_t>(::rtype::network::EntityType::Missile);
            } else if (isObstacle) {
                event.entityType = static_cast<uint8_t>(::rtype::network::EntityType::Obstacle);
            } else {
                event.entityType = static_cast<uint8_t>(::rtype::network::EntityType::Player);
            }
            _emitEvent(event);
        } else {
            ::rtype::Logger::instance().warning(
                "DestroySystem: Entity destroyed without valid NetworkId - "
                "clients will not be notified");
        }
        registry.killEntity(entity);
    }
}

}  // namespace rtype::games::rtype::server
