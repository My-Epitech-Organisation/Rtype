/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PlayerPowerUpVisualSystem - Adds simple visual cues for active power-ups
*/

#include "PlayerPowerUpVisualSystem.hpp"

#include "../AllComponents.hpp"
#include "../shared/Components/PowerUpComponent.hpp"

namespace rtype::games::rtype::client {

namespace rs = ::rtype::games::rtype::shared;

PlayerPowerUpVisualSystem::PlayerPowerUpVisualSystem()
    : ::rtype::engine::ASystem("PlayerPowerUpVisualSystem") {}

void PlayerPowerUpVisualSystem::update(ECS::Registry& registry, float /*dt*/) {
    registry.view<Image, PlayerTag, rs::ActivePowerUpComponent>().each(
        [&](auto entity, Image& img, PlayerTag& /*tag*/,
            const rs::ActivePowerUpComponent& active) {
            if (active.remainingTime <= 0.0f) {
                if (registry.hasComponent<ColorTint>(entity)) {
                    registry.removeComponent<ColorTint>(entity);
                }
                return;
            }
            ::rtype::display::Color tint = ::rtype::display::Color::White();
            switch (active.type) {
                case rs::PowerUpType::Shield:
                    tint = {255, 215, 0, 220};
                    break;
                case rs::PowerUpType::SpeedBoost:
                    tint = {140, 255, 180, 220};
                    break;
                case rs::PowerUpType::RapidFire:
                    tint = {120, 200, 255, 220};
                    break;
                case rs::PowerUpType::DoubleDamage:
                    tint = {255, 120, 120, 220};
                    break;
                case rs::PowerUpType::HealthBoost:
                    tint = {220, 180, 255, 220};
                    break;
                default:
                    return;
            }
            if (active.shieldActive) {
                tint = {255, 215, 0, 240};
            }
            if (!registry.hasComponent<ColorTint>(entity)) {
                registry.emplaceComponent<ColorTint>(entity, tint.r, tint.g, tint.b, tint.a);
            } else {
                auto& colorTint = registry.getComponent<ColorTint>(entity);
                colorTint.r = tint.r;
                colorTint.g = tint.g;
                colorTint.b = tint.b;
                colorTint.a = tint.a;
            }
            if (registry.hasComponent<BoxingComponent>(entity)) {
                auto& box = registry.getComponent<BoxingComponent>(entity);
                box.outlineColor = tint;
                box.fillColor = {tint.r, tint.g, tint.b, 40};
            }
        });
}

}  // namespace rtype::games::rtype::client
