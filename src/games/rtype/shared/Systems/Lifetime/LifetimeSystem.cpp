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

void LifetimeSystem::update(ECS::Registry& registry, float deltaTime) {
    if (deltaTime < 0) {
        return;
    }
    auto view = registry.view<LifetimeComponent>();

    view.each([deltaTime, &registry](auto entity, LifetimeComponent& lifetime) {
        lifetime.remainingTime -= deltaTime;
        if (lifetime.remainingTime <= 0.0F) {
            if (!registry.hasComponent<DestroyTag>(entity)) {
                LOG_DEBUG("[LifetimeSystem] Entity " + std::to_string(entity.id) + " expired (lifetime <= 0)");
                registry.emplaceComponent<DestroyTag>(entity, DestroyTag{});
            }
        }
    });
}

}  // namespace rtype::games::rtype::shared
