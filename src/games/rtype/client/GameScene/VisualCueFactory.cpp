/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** VisualCueFactory.cpp
*/
#include "VisualCueFactory.hpp"

#include <random>
#include <utility>

#include "AllComponents.hpp"
#include "Components/RectangleComponent.hpp"
#include "Components/TagComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "Graphic/Accessibility.hpp"
#include "Logger/Macros.hpp"
#include "games/rtype/shared/Components/LifetimeComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"

namespace rtype::games::rtype::client {

void VisualCueFactory::createFlash(ECS::Registry& registry,
                                   const sf::Vector2f& center,
                                   const sf::Color& color, float size,
                                   float lifetime, int zIndex) {
    if (registry.hasSingleton<AccessibilitySettings>()) {
        const auto& acc = registry.getSingleton<AccessibilitySettings>();
        if (!acc.showVisualCues) {
            return;
        }
    } else {
        return;
    }

    auto entity = registry.spawnEntity();

    registry.emplaceComponent<Rectangle>(
        entity, std::pair<float, float>({size, size}), color, color);
    auto& rect = registry.getComponent<Rectangle>(entity);
    rect.currentColor = color;
    rect.outlineColor = sf::Color::White;
    rect.outlineThickness = 3.f;

    registry
        .emplaceComponent<::rtype::games::rtype::shared::TransformComponent>(
            entity, center.x - (size / 2.f), center.y - (size / 2.f));
    registry.emplaceComponent<ZIndex>(entity, zIndex);
    registry.emplaceComponent<::rtype::games::rtype::shared::LifetimeComponent>(
        entity, lifetime);
    registry.emplaceComponent<GameTag>(entity);
}

void VisualCueFactory::createDamagePopup(ECS::Registry& registry,
                                         const sf::Vector2f& position,
                                         int damage,
                                         std::shared_ptr<sf::Font> font,
                                         const sf::Color& color) {
    auto entity = registry.spawnEntity();
    LOG_DEBUG_CAT(
        ::rtype::LogCategory::Graphics,
        "[VisualCueFactory] Damage popup entity=" + std::to_string(entity.id) +
            " dmg=" + std::to_string(damage));

    std::string damageText = "-" + std::to_string(damage);
    registry.emplaceComponent<Text>(entity, font, color, 32, damageText);

    registry.emplaceComponent<StaticTextTag>(entity);

    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(-20, 20);
    float offsetX = static_cast<float>(dist(gen));
    registry
        .emplaceComponent<::rtype::games::rtype::shared::TransformComponent>(
            entity, position.x + offsetX, position.y - 20.f);

    registry.emplaceComponent<::rtype::games::rtype::shared::VelocityComponent>(
        entity, 0.f, -80.f);

    registry.emplaceComponent<ZIndex>(entity, 200);

    registry.emplaceComponent<::rtype::games::rtype::shared::LifetimeComponent>(
        entity, 1.2f);
    registry.emplaceComponent<GameTag>(entity);
}

void VisualCueFactory::createPowerUpPopup(ECS::Registry& registry,
                                          const sf::Vector2f& position,
                                          const std::string& powerUpName,
                                          std::shared_ptr<sf::Font> font,
                                          const sf::Color& color) {
    auto entity = registry.spawnEntity();
    LOG_DEBUG("[VisualCueFactory] PowerUp popup entity=" +
              std::to_string(entity.id) + " name=" + powerUpName);

    registry.emplaceComponent<Text>(entity, font, color, 28, powerUpName);
    registry.emplaceComponent<StaticTextTag>(entity);

    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(-15, 15);
    float offsetX = static_cast<float>(dist(gen));
    registry
        .emplaceComponent<::rtype::games::rtype::shared::TransformComponent>(
            entity, position.x + offsetX, position.y - 30.f);

    registry.emplaceComponent<::rtype::games::rtype::shared::VelocityComponent>(
        entity, 0.f, -60.f);

    registry.emplaceComponent<ZIndex>(entity, 200);

    registry.emplaceComponent<::rtype::games::rtype::shared::LifetimeComponent>(
        entity, 1.5f);
    registry.emplaceComponent<GameTag>(entity);
}

}  // namespace rtype::games::rtype::client
