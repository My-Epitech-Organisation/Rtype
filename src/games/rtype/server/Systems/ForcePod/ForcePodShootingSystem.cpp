/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodShootingSystem - Implementation
*/

#include "ForcePodShootingSystem.hpp"

#include "../Projectile/ProjectileSpawnerSystem.hpp"

namespace rtype::games::rtype::server {

using shared::ForcePodComponent;
using shared::ForcePodState;
using shared::ForcePodTag;
using shared::NetworkIdComponent;
using shared::ShootCooldownComponent;
using shared::TransformComponent;

ForcePodShootingSystem::ForcePodShootingSystem(
    ProjectileSpawnerSystem* projectileSpawner)
    : ASystem("ForcePodShootingSystem"),
      _projectileSpawner(projectileSpawner) {}

void ForcePodShootingSystem::update(ECS::Registry& registry, float deltaTime) {
    auto podView = registry.view<ForcePodTag, ForcePodComponent,
                                 TransformComponent, NetworkIdComponent>();

    podView.each([&](ECS::Entity podEntity, const ForcePodTag&,
                     const ForcePodComponent& forcePod,
                     const TransformComponent& transform,
                     const NetworkIdComponent& networkId) {
        if (forcePod.state == ForcePodState::Returning) {
            return;
        }

        if (!registry.hasComponent<ShootCooldownComponent>(podEntity)) {
            registry.emplaceComponent<ShootCooldownComponent>(podEntity,
                                                              SHOOT_COOLDOWN);
        }

        auto& cooldown =
            registry.getComponent<ShootCooldownComponent>(podEntity);
        cooldown.update(deltaTime);

        if (cooldown.canShoot()) {
            cooldown.triggerCooldown();
            if (_projectileSpawner) {
                _projectileSpawner->spawnPlayerProjectile(
                    registry, networkId.networkId, transform.x, transform.y);
            }
        }
    });
}

}  // namespace rtype::games::rtype::server
