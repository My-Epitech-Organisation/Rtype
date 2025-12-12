/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EnemyShootingSystem - implementation
*/

#include "EnemyShootingSystem.hpp"

#include <limits>

#include "../../shared/Components.hpp"

namespace rtype::games::rtype::server {

using shared::AIBehavior;
using shared::AIComponent;
using shared::EnemyTag;
using shared::NetworkIdComponent;
using shared::PlayerTag;
using shared::ShootCooldownComponent;
using shared::TransformComponent;

void EnemyShootingSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    if (!_shootCb) {
        return;
    }

    struct PlayerInfo {
        uint32_t networkId;
        float x;
        float y;
    };
    std::vector<PlayerInfo> players;
    auto playerView =
        registry.view<PlayerTag, TransformComponent, NetworkIdComponent>();
    playerView.each([&players](ECS::Entity /*entity*/, const PlayerTag& /*tag*/,
                               const TransformComponent& transform,
                               const NetworkIdComponent& net) {
        players.push_back(PlayerInfo{net.networkId, transform.x, transform.y});
    });

    auto enemyView =
        registry.view<EnemyTag, TransformComponent, NetworkIdComponent,
                      ShootCooldownComponent>();
    enemyView.each([this, &registry, &players](ECS::Entity entity,
                                               const EnemyTag& /*tag*/,
                                               const TransformComponent& tf,
                                               const NetworkIdComponent& net,
                                               ShootCooldownComponent& cd) {
        if (!cd.canShoot()) {
            return;
        }

        float targetX = tf.x - 300.0F;
        float targetY = tf.y;
        if (registry.hasComponent<AIComponent>(entity)) {
            const auto& ai = registry.getComponent<AIComponent>(entity);
            if (ai.behavior == AIBehavior::Chase) {
                float bestDist2 = std::numeric_limits<float>::max();
                for (const auto& p : players) {
                    float dx = p.x - tf.x;
                    float dy = p.y - tf.y;
                    float d2 = dx * dx + dy * dy;
                    if (d2 < bestDist2) {
                        bestDist2 = d2;
                        targetX = p.x;
                        targetY = p.y;
                    }
                }
            }
        }

        _shootCb(registry, entity, net.networkId, tf.x, tf.y, targetX, targetY);
        cd.triggerCooldown();
    });
}

}  // namespace rtype::games::rtype::server
