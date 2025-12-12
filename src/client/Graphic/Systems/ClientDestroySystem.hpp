/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ClientDestroySystem - Client-side entity destruction
*/

#ifndef SRC_CLIENT_GRAPHIC_SYSTEMS_CLIENTDESTROYSYSTEM_HPP_
#define SRC_CLIENT_GRAPHIC_SYSTEMS_CLIENTDESTROYSYSTEM_HPP_

#include <vector>

#include <rtype/engine.hpp>
#include "Logger/Macros.hpp"
#include "../../../games/rtype/shared/Components/Tags.hpp"
#include "../../../games/rtype/shared/Components/LifetimeComponent.hpp"

namespace rtype::games::rtype::client {

/**
 * @class ClientDestroySystem
 * @brief Client-side system that destroys entities marked with DestroyTag
 *
 * This system handles cleanup of local entities (like visual effects, popups)
 * that have been marked for destruction by LifetimeSystem.
 */
class ClientDestroySystem : public ::rtype::engine::ASystem {
   public:
    ClientDestroySystem() : ASystem("ClientDestroySystem") {}

    void update(ECS::Registry& registry, float /*deltaTime*/) override {
        auto destroyView = registry.view<shared::DestroyTag>();

        std::vector<ECS::Entity> toDestroy;
        destroyView.each([&toDestroy](auto entity, const shared::DestroyTag&) {
            toDestroy.push_back(entity);
        });
        auto lifetimeView = registry.view<shared::LifetimeComponent>();
        lifetimeView.each([&toDestroy](auto entity, const shared::LifetimeComponent& life) {
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
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_CLIENT_GRAPHIC_SYSTEMS_CLIENTDESTROYSYSTEM_HPP_
