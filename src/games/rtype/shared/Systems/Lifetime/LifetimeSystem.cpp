/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LifetimeSystem - Manages entity lifecycle based on lifetime implementation
*/

#include "LifetimeSystem.hpp"

#include "../../Components/Tags.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::shared {

namespace {
    constexpr size_t PARALLEL_THRESHOLD = 100;
}

void LifetimeSystem::update(ECS::Registry& registry, float deltaTime) {
    if (deltaTime < 0) {
        return;
    }
    ECS::CommandBuffer cmdBuffer(std::ref(registry));
    const size_t entityCount = registry.countComponents<LifetimeComponent>();

    if (entityCount >= PARALLEL_THRESHOLD) {
        auto view = registry.parallelView<LifetimeComponent>();
        view.each([deltaTime, &cmdBuffer](auto entity, LifetimeComponent& lifetime) {
            lifetime.remainingTime -= deltaTime;
            if (lifetime.remainingTime <= 0.0F) {
                LOG_DEBUG("[LifetimeSystem] Entity " +
                          std::to_string(entity.id) +
                          " expired (lifetime <= 0)");
                cmdBuffer.emplaceComponentDeferred<DestroyTag>(entity, DestroyTag{});
            }
        });
    } else {
        auto view = registry.view<LifetimeComponent>();
        view.each([deltaTime, &cmdBuffer](auto entity, LifetimeComponent& lifetime) {
            lifetime.remainingTime -= deltaTime;
            if (lifetime.remainingTime <= 0.0F) {
                LOG_DEBUG("[LifetimeSystem] Entity " +
                          std::to_string(entity.id) +
                          " expired (lifetime <= 0)");
                cmdBuffer.emplaceComponentDeferred<DestroyTag>(entity, DestroyTag{});
            }
        });
    }

    cmdBuffer.flush();
}

}  // namespace rtype::games::rtype::shared
