/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AISystem - Shared AI behavior logic implementation
*/

#include "AISystem.hpp"

#include "Behaviors/BehaviorRegistry.hpp"

namespace rtype::games::rtype::shared {

namespace {
constexpr size_t PARALLEL_THRESHOLD = 50;
}

void AISystem::update(ECS::Registry& registry, float deltaTime) {
    const size_t entityCount = registry.countComponents<AIComponent>();
    const auto& behaviorRegistry = BehaviorRegistry::instance();
    if (entityCount >= PARALLEL_THRESHOLD) {
        auto view = registry.parallelView<AIComponent, TransformComponent,
                                          VelocityComponent>();
        view.each([deltaTime, &behaviorRegistry](
                      ECS::Entity /*entity*/, AIComponent& ai,
                      const TransformComponent& transform,
                      VelocityComponent& velocity) {
            auto behavior = behaviorRegistry.getBehavior(ai.behavior);
            if (behavior) {
                behavior->apply(ai, transform, velocity, deltaTime);
            }
        });
    } else {
        auto view =
            registry.view<AIComponent, TransformComponent, VelocityComponent>();
        view.each([deltaTime, &behaviorRegistry](
                      ECS::Entity /*entity*/, AIComponent& ai,
                      const TransformComponent& transform,
                      VelocityComponent& velocity) {
            auto behavior = behaviorRegistry.getBehavior(ai.behavior);
            if (behavior) {
                behavior->apply(ai, transform, velocity, deltaTime);
            }
        });
    }
}

}  // namespace rtype::games::rtype::shared
