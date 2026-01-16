/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodShootingSystem - Handles Force Pod automatic shooting
*/

#pragma once

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "../../../shared/Components/CooldownComponent.hpp"
#include "../../../shared/Components/ForcePodComponent.hpp"
#include "../../../shared/Components/NetworkIdComponent.hpp"
#include "../../../shared/Components/Tags.hpp"
#include "../../../shared/Components/TransformComponent.hpp"

namespace rtype::games::rtype::server {

class ProjectileSpawnerSystem;

/**
 * @brief System that handles Force Pod automatic shooting
 *
 * Force Pods shoot automatically when attached to a player or when detached.
 * Shooting rate and direction depend on the Force Pod's state.
 */
class ForcePodShootingSystem : public ::rtype::engine::ASystem {
   public:
    /**
     * @brief Construct a new Force Pod Shooting System
     * @param projectileSpawner Pointer to projectile spawner system
     */
    explicit ForcePodShootingSystem(ProjectileSpawnerSystem* projectileSpawner);

    /**
     * @brief Update Force Pod shooting logic
     * @param registry ECS registry
     * @param deltaTime Time elapsed since last update
     */
    void update(ECS::Registry& registry, float deltaTime) override;

   private:
    ProjectileSpawnerSystem* _projectileSpawner;
    static constexpr float SHOOT_COOLDOWN = 0.9F;  // Shoots ~3 times per second
};

}  // namespace rtype::games::rtype::server
