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
    using ShootCallback = std::function<uint32_t(ECS::Registry&, ECS::Entity,
                                                uint32_t, float, float, float,
                                                float)>;

    explicit EnemyShootingSystem(ShootCallback shootCb)
        : ASystem("EnemyShootingSystem"), _shootCb(std::move(shootCb)) {}

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    ShootCallback _shootCb;
};

}  // namespace rtype::games::rtype::server
