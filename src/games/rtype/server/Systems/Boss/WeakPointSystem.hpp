/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** WeakPointSystem - Manages boss weak points
*/

#pragma once

#include <functional>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

namespace rtype::games::rtype::server {

/**
 * @class WeakPointSystem
 * @brief System that manages boss weak points
 *
 * Handles:
 * - Weak point position synchronization with parent boss
 * - Weak point destruction detection
 * - Bonus score emission on destruction
 * - Parent damage application when weak points are destroyed
 * - Disabling boss attacks when relevant weak points are destroyed
 */
class WeakPointSystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    /**
     * @brief Construct with event emitter
     * @param emitter Function to emit game events
     */
    explicit WeakPointSystem(EventEmitter emitter);

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    void syncWeakPointPositions(ECS::Registry& registry);

    void handleWeakPointDestruction(ECS::Registry& registry);

    void applyParentDamage(ECS::Registry& registry, ECS::Entity weakPoint);

    void disableBossPattern(ECS::Registry& registry, ECS::Entity weakPoint);

    EventEmitter _emitEvent;
};

}  // namespace rtype::games::rtype::server
