/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BossPhaseSystem - Handles boss phase transitions based on health
*/

#pragma once

#include <functional>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

namespace rtype::games::rtype::server {

/**
 * @class BossPhaseSystem
 * @brief System that manages boss phase transitions
 *
 * Monitors boss health and triggers phase transitions when health
 * drops below configured thresholds. Emits events for visual/audio feedback.
 *
 * Phase transitions:
 * - Check health ratio against phase thresholds (<= check, not ==)
 * - Trigger brief invulnerability during transition
 * - Emit phase change event for client feedback
 * - Update attack patterns for new phase
 */
class BossPhaseSystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    /**
     * @brief Construct with event emitter
     * @param emitter Function to emit game events
     */
    explicit BossPhaseSystem(EventEmitter emitter);

    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    void handlePhaseTransition(ECS::Registry& registry, ECS::Entity entity,
                               std::size_t newPhaseIndex);

    void updatePhaseTransitions(ECS::Registry& registry, float deltaTime);

    void updateBossMovement(ECS::Registry& registry, float deltaTime);

    void checkBossDefeated(ECS::Registry& registry, ECS::Entity entity);

    EventEmitter _emitEvent;
};

}  // namespace rtype::games::rtype::server
