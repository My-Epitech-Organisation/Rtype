/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientDestroySystem
*/

#include "ClientDestroySystem.hpp"

#include <vector>

namespace rtype::games::rtype::client {

void ClientDestroySystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    auto destroyView = registry.view<shared::DestroyTag>();

    std::vector<ECS::Entity> toDestroy;
    destroyView.each([&toDestroy](auto entity, const shared::DestroyTag&) {
        toDestroy.push_back(entity);
    });
    auto lifetimeView = registry.view<shared::LifetimeComponent>();
    lifetimeView.each(
        [&toDestroy](auto entity, const shared::LifetimeComponent& life) {
            if (life.remainingTime <= 0.0F) {
                toDestroy.push_back(entity);
            }
        });

    if (!toDestroy.empty()) {
        LOG_DEBUG("[ClientDestroySystem] Destroying " +
                  std::to_string(toDestroy.size()) + " entities");
        for (auto entity : toDestroy) {
            registry.killEntity(entity);
        }
    }
}

}  // namespace rtype::games::rtype::client
