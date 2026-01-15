/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PowerUpSystem - Manages active power-up timers and cleanup
*/

#include "PowerUpSystem.hpp"

#include "Logger/Macros.hpp"

namespace rtype::games::rtype::shared {

void PowerUpSystem::update(ECS::Registry& registry, float deltaTime) {
    if (deltaTime <= 0.0F) {
        return;
    }

    auto view = registry.view<ActivePowerUpComponent>();
    view.each([deltaTime, &registry](ECS::Entity entity,
                                     ActivePowerUpComponent& active) {
        active.remainingTime -= deltaTime;
        if (active.remainingTime > 0.0F) {
            return;
        }
        if (active.shieldActive &&
            registry.hasComponent<InvincibleTag>(entity)) {
            registry.removeComponent<InvincibleTag>(entity);
        }
        if (active.hasOriginalCooldown &&
            registry.hasComponent<ShootCooldownComponent>(entity)) {
            auto& cd = registry.getComponent<ShootCooldownComponent>(entity);
            cd.setCooldownTime(active.originalCooldown);
        }
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[PowerUpSystem] Power-up expired for entity "
                          << entity.id << " (" << static_cast<int>(active.type)
                          << ")");
        registry.removeComponent<ActivePowerUpComponent>(entity);
    });
}

}  // namespace rtype::games::rtype::shared
