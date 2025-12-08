/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DestroySystem - Server-side entity destruction
*/

#pragma once

#include <functional>

#include <rtype/engine.hpp>

namespace rtype::games::rtype::server {

/**
 * @class DestroySystem
 * @brief Server-only system that destroys entities marked with DestroyTag
 *
 * Emits destruction events for network synchronization.
 */
class DestroySystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;
    using EnemyCountUpdater = std::function<void()>;

    DestroySystem(EventEmitter emitter,
                  EnemyCountUpdater enemyCountDecrementer);

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    EventEmitter _emitEvent;
    EnemyCountUpdater _decrementEnemyCount;
};

}  // namespace rtype::games::rtype::server
