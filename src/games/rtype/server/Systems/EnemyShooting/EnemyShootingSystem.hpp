/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EnemyShootingSystem - drives enemy projectile fire
*/

#pragma once

#include <functional>
#include <vector>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

namespace rtype::games::rtype::server {

/**
 * @class EnemyShootingSystem
 * @brief Server-only system that orders enemies to shoot at players.
 */
class EnemyShootingSystem : public ::rtype::engine::ASystem {
   public:
    using ShootCallback = std::function<uint32_t(
        ECS::Registry&, ECS::Entity, uint32_t, float, float, float, float)>;

    explicit EnemyShootingSystem(ShootCallback shootCb, float defaultTargetOffset = 300.0F)
        : ASystem("EnemyShootingSystem"),
          _shootCb(std::move(shootCb)),
          _defaultTargetOffset(defaultTargetOffset) {}

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    struct PlayerInfo {
        uint32_t networkId;
        float x;
        float y;
    };

    ShootCallback _shootCb;
    float _defaultTargetOffset;
    std::vector<PlayerInfo> _playerCache;
};

}  // namespace rtype::games::rtype::server
