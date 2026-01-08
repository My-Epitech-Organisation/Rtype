/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AISystem - Shared AI behavior logic implementation
*/

#include "AISystem.hpp"

#include <limits>

#include "../../Components/Tags.hpp"
#include "Behaviors/BehaviorRegistry.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::shared {

namespace {
constexpr size_t PARALLEL_THRESHOLD = 50;
}

void AISystem::update(ECS::Registry& registry, float deltaTime) {
    updateChaseTargets(registry);

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

void AISystem::updateChaseTargets(ECS::Registry& registry) {
    struct PlayerInfo {
        float x;
        float y;
    };
    std::vector<PlayerInfo> players;

    auto playerView = registry.view<PlayerTag, TransformComponent>();
    playerView.each([&players](ECS::Entity /*entity*/, const PlayerTag& /*tag*/,
                               const TransformComponent& transform) {
        players.push_back({transform.x, transform.y});
    });

    auto chaseView = registry.view<EnemyTag, AIComponent, TransformComponent>();
    chaseView.each([&players](ECS::Entity /*entity*/, const EnemyTag& /*tag*/,
                              AIComponent& ai,
                              const TransformComponent& transform) {
        if (ai.behavior != AIBehavior::Chase) {
            return;
        }
        float bestDist2 = std::numeric_limits<float>::max();
        float targetX = ai.targetX;
        float targetY = ai.targetY;

        for (const auto& player : players) {
            float dx = player.x - transform.x;
            float dy = player.y - transform.y;
            float dist2 = dx * dx + dy * dy;

            if (dist2 < bestDist2) {
                bestDist2 = dist2;
                targetX = player.x;
                targetY = player.y;
            }
        }
        ai.targetX = targetX;
        ai.targetY = targetY;
    });
}

}  // namespace rtype::games::rtype::shared
