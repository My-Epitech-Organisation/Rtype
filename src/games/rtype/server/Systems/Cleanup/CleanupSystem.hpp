/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CleanupSystem - Server-side out-of-bounds cleanup
*/

#pragma once

#include <functional>

#include <rtype/engine.hpp>

namespace rtype::games::rtype::server {

/**
 * @struct CleanupConfig
 * @brief Configuration for cleanup boundaries
 */
struct CleanupConfig {
    float leftBoundary = -100.0F;
    float rightBoundary = 900.0F;
    float topBoundary = -100.0F;
    float bottomBoundary = 700.0F;
};

/**
 * @class CleanupSystem
 * @brief Server-only system that marks out-of-bounds entities for destruction
 */
class CleanupSystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    CleanupSystem(EventEmitter emitter, CleanupConfig config);

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    EventEmitter _emitEvent;
    CleanupConfig _config;
};

}  // namespace rtype::games::rtype::server
