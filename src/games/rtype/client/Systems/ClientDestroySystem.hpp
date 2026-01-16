/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ClientDestroySystem - Client-side entity destruction
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CLIENTDESTROYSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CLIENTDESTROYSYSTEM_HPP_

#include <vector>

#include <rtype/engine.hpp>

#include "../../shared/Components/Tags.hpp"
#include "Logger/Macros.hpp"

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

    void update(ECS::Registry& registry, float /*deltaTime*/) override;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_CLIENTDESTROYSYSTEM_HPP_
