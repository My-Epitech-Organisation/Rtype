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
                                   const ::rtype::display::Vector2f& center,
                                   const ::rtype::display::Color& color,
                                   float size, float lifetime, int zIndex) {
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
    rect.outlineColor = ::rtype::display::Color::White();
    rect.outlineThickness = 3.f;

    registry
        .emplaceComponent<::rtype::games::rtype::shared::TransformComponent>(
            entity, center.x - (size / 2.f), center.y - (size / 2.f));
    registry.emplaceComponent<ZIndex>(entity, zIndex);
    registry.emplaceComponent<::rtype::games::rtype::shared::LifetimeComponent>(
        entity, lifetime);
    registry.emplaceComponent<GameTag>(entity);
}

void VisualCueFactory::createDamagePopup(
    ECS::Registry& registry, const ::rtype::display::Vector2f& position,
    int damage, const std::string& fontName,
    const ::rtype::display::Color& color) {
    auto entity = registry.spawnEntity();
    LOG_DEBUG_CAT(
        ::rtype::LogCategory::Graphics,
        "[VisualCueFactory] Damage popup entity=" + std::to_string(entity.id) +
            " dmg=" + std::to_string(damage));

    std::string damageText = "-" + std::to_string(damage);
    registry.emplaceComponent<Text>(entity, fontName, color, 32, damageText);

    registry.emplaceComponent<StaticTextTag>(entity);
    registry.emplaceComponent<CenteredTextTag>(entity);

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

void VisualCueFactory::createPowerUpPopup(
    ECS::Registry& registry, const ::rtype::display::Vector2<float>& position,
    const std::string& powerUpName, const std::string& font,
    const ::rtype::display::Color& color) {
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

void VisualCueFactory::createConfetti(ECS::Registry& registry,
                                      float screenWidth, float screenHeight,
                                      int particleCount) {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posXDist(0.f, screenWidth);
    std::uniform_real_distribution<float> posYDist(-150.f, 0.f);
    std::uniform_real_distribution<float> velXDist(-120.f, 120.f);
    std::uniform_real_distribution<float> velYDist(300.f, 700.f);
    std::uniform_real_distribution<float> sizeDist(6.f, 22.f);
    std::uniform_real_distribution<float> lifetimeDist(1.2f, 2.0f);
    std::uniform_int_distribution<int> colorDist(0, 5);

    // Festive colors: red, green, blue, yellow, magenta, cyan
    const ::rtype::display::Color colors[] = {
        ::rtype::display::Color(255, 50, 50, 255),   // Red
        ::rtype::display::Color(50, 255, 50, 255),   // Green
        ::rtype::display::Color(50, 100, 255, 255),  // Blue
        ::rtype::display::Color(255, 255, 50, 255),  // Yellow
        ::rtype::display::Color(255, 50, 255, 255),  // Magenta
        ::rtype::display::Color(50, 255, 255, 255)   // Cyan
    };

    LOG_DEBUG_CAT(::rtype::LogCategory::Graphics,
                  "[VisualCueFactory] Creating confetti effect with "
                      << particleCount << " particles");

    for (int i = 0; i < particleCount; ++i) {
        auto entity = registry.spawnEntity();

        float size = sizeDist(gen);
        const auto& color = colors[colorDist(gen)];

        registry.emplaceComponent<Rectangle>(
            entity, std::pair<float, float>({size, size}), color, color);

        float x = posXDist(gen);
        float y = posYDist(gen);
        registry.emplaceComponent<
            ::rtype::games::rtype::shared::TransformComponent>(entity, x, y);

        float velX = velXDist(gen);
        float velY = velYDist(gen);
        registry
            .emplaceComponent<::rtype::games::rtype::shared::VelocityComponent>(
                entity, velX, velY);

        registry.emplaceComponent<ZIndex>(entity, 250);

        float lifetime = lifetimeDist(gen);
        registry
            .emplaceComponent<::rtype::games::rtype::shared::LifetimeComponent>(
                entity, lifetime);
        registry.emplaceComponent<GameTag>(entity);
    }
}

}  // namespace rtype::games::rtype::client
