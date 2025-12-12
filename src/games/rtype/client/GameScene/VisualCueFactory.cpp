/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** VisualCueFactory.cpp
*/
#include "VisualCueFactory.hpp"

#include <utility>

#include "AllComponents.hpp"
#include "Components/RectangleComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "Graphic/Accessibility.hpp"
#include "games/rtype/shared/Components/LifetimeComponent.hpp"

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

    registry.emplaceComponent<::rtype::games::rtype::shared::Position>(
        entity, center.x - (size / 2.f), center.y - (size / 2.f));
    registry.emplaceComponent<ZIndex>(entity, zIndex);
    registry.emplaceComponent<::rtype::games::rtype::shared::LifetimeComponent>(
        entity, lifetime);
}

}  // namespace rtype::games::rtype::client
