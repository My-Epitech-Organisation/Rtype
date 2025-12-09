/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ProjectileSystem - Handles projectile movement and behavior
*/

#pragma once

#include <rtype/engine.hpp>

#include "../../../shared/Components/Tags.hpp"
#include "../../Components/PositionComponent.hpp"
#include "../../Components/VelocityComponent.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class ProjectileSystem
 * @brief System that handles projectile movement and updates
 *
 * This is a shared system used by both client and server.
 * It manages projectile-specific behavior and movement.
 */
class ProjectileSystem : public ::rtype::engine::ASystem {
   public:
    ProjectileSystem() : ASystem("ProjectileSystem") {}

    /**
     * @brief Update all projectile entities
     * @param registry ECS registry containing entities
     * @param deltaTime Time elapsed since last update
     */
    void update(ECS::Registry& registry, float deltaTime) override;
};

}  // namespace rtype::games::rtype::shared
