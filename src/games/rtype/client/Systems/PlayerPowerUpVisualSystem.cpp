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
    // Reset colors for players without active power-ups
    // Skip players with ColorTint component to avoid overriding ColorTintSystem
    registry.view<Image, PlayerTag>().each(
        [&](auto entity, Image& img, PlayerTag& /*tag*/) {
            if (registry.hasComponent<ColorTint>(entity)) {
                return;  // ColorTintSystem handles color for this entity
            }
            img.sprite.setColor(sf::Color::White);
            if (registry.hasComponent<BoxingComponent>(entity)) {
                auto& box = registry.getComponent<BoxingComponent>(entity);
                box.outlineColor = sf::Color::Red;
                box.fillColor = sf::Color(255, 255, 255, 30);
            }
        });

    // Apply power-up visual effects
    // Skip players with ColorTint component to avoid overriding ColorTintSystem
    registry.view<Image, PlayerTag, rs::ActivePowerUpComponent>().each(
        [&](auto entity, Image& img, PlayerTag& /*tag*/,
            const rs::ActivePowerUpComponent& active) {
            if (registry.hasComponent<ColorTint>(entity)) {
                return;  // ColorTintSystem handles color for this entity
            }
            if (active.remainingTime <= 0.0f) {
                return;
            }
            sf::Color tint = sf::Color::White;
            switch (active.type) {
                case rs::PowerUpType::Shield:
                    tint = sf::Color(255, 215, 0, 220);
                    break;
                case rs::PowerUpType::SpeedBoost:
                    tint = sf::Color(140, 255, 180, 220);
                    break;
                case rs::PowerUpType::RapidFire:
                    tint = sf::Color(120, 200, 255, 220);
                    break;
                case rs::PowerUpType::DoubleDamage:
                    tint = sf::Color(255, 120, 120, 220);
                    break;
                case rs::PowerUpType::HealthBoost:
                    tint = sf::Color(220, 180, 255, 220);
                    break;
                default:
                    return;
            }
            if (active.shieldActive) {
                tint = sf::Color(255, 215, 0, 240);
            }
            img.sprite.setColor(tint);
            if (registry.hasComponent<BoxingComponent>(entity)) {
                auto& box = registry.getComponent<BoxingComponent>(entity);
                box.outlineColor = tint;
                box.fillColor = sf::Color(tint.r, tint.g, tint.b, 40);
            }
        });
}

}  // namespace rtype::games::rtype::client
