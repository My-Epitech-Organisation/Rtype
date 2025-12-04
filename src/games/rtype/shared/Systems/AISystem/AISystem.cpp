/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AISystem - Shared AI behavior logic implementation
*/

#include "AISystem.hpp"

#include "Behaviors/BehaviorRegistry.hpp"

namespace rtype::games::rtype::shared {

void AISystem::update(ECS::Registry& registry, float deltaTime) {
    auto view =
        registry.view<AIComponent, TransformComponent, VelocityComponent>();
    const auto& behaviorRegistry = BehaviorRegistry::instance();

    view.each(
        [deltaTime, &behaviorRegistry](ECS::Entity /*entity*/, AIComponent& ai,
                                       const TransformComponent& transform,
                                       VelocityComponent& velocity) {
            auto behavior = behaviorRegistry.getBehavior(ai.behavior);
            if (behavior) {
                behavior->apply(ai, transform, velocity, deltaTime);
            }
        });
}

}  // namespace rtype::games::rtype::shared
