/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LifetimeSystem - Manages entity lifecycle based on lifetime
*/

#pragma once

#include <rtype/engine.hpp>

#include "../../Components/LifetimeComponent.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class LifetimeSystem
 * @brief System that destroys entities when their lifetime expires
 *
 * This is a shared system used by both client and server.
 * It decrements the lifetime timer and destroys entities when it reaches zero.
 */
class LifetimeSystem : public ::rtype::engine::ASystem {
   public:
    LifetimeSystem() : ASystem("LifetimeSystem") {}

    /**
     * @brief Update all entities with a LifetimeComponent
     * @param registry ECS registry containing entities
     * @param deltaTime Time elapsed since last update
     */
    void update(ECS::Registry& registry, float deltaTime) override;
};

}  // namespace rtype::games::rtype::shared
