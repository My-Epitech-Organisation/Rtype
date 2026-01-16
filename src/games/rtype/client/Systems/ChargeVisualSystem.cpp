/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargeVisualSystem - Implementation
*/

#include "ChargeVisualSystem.hpp"

#include <cmath>
#include <random>
#include <utility>

#include "../AllComponents.hpp"
#include "../Components/ChargeShotVisualComponent.hpp"
#include "../Components/ColorTintComponent.hpp"
#include "Logger/Macros.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

namespace rtype::games::rtype::client {

ChargeVisualSystem::ChargeVisualSystem(
    std::shared_ptr<::rtype::display::IDisplay> display,
    std::shared_ptr<AudioLib> audioLib)
    : ASystem("ChargeVisualSystem"),
      _display(std::move(display)),
      _audioLib(std::move(audioLib)) {}

void ChargeVisualSystem::update(ECS::Registry& registry, float dt) {
    auto view =
        registry.view<shared::ChargeComponent, ChargeShotVisual, ColorTint>();

    view.each([this, dt, &registry](ECS::Entity entity,
                                    shared::ChargeComponent& charge,
                                    ChargeShotVisual& visual, ColorTint& tint) {
        visual.updateGlow(charge.currentLevel, charge.isCharging);
        visual.updateShake(dt);

        if (charge.isCharging) {
            auto [r, g, b] =
                ChargeShotVisual::getGlowColor(charge.currentLevel);

            float blendFactor = visual.glowIntensity;
            tint.r = static_cast<uint8_t>(
                255 +
                static_cast<int>((static_cast<int>(r) - 255) * blendFactor));
            tint.g = static_cast<uint8_t>(
                255 +
                static_cast<int>((static_cast<int>(g) - 255) * blendFactor));
            tint.b = static_cast<uint8_t>(
                255 +
                static_cast<int>((static_cast<int>(b) - 255) * blendFactor));
        } else {
            tint.r = 255;
            tint.g = 255;
            tint.b = 255;
        }

        if (visual.isShaking()) {
            float intensity = visual.getEffectiveShakeIntensity();
            applyScreenShake(intensity);
        } else if (_isShaking) {
            resetScreenShake();
        }
    });

    auto chargeBarView = registry.view<shared::ChargeComponent, ChargeBarUI,
                                       shared::TransformComponent>();

    chargeBarView.each(
        [dt](ECS::Entity entity, const shared::ChargeComponent& charge,
             ChargeBarUI& bar, const shared::TransformComponent& transform) {
            bar.setChargePercent(charge.getChargePercent());
            bar.update(dt);
        });
}

void ChargeVisualSystem::renderChargeBar(ECS::Registry& registry) {
    if (!_display) return;

    auto chargeBarView = registry.view<shared::ChargeComponent, ChargeBarUI,
                                       shared::TransformComponent>();

    chargeBarView.each([this](ECS::Entity entity,
                              const shared::ChargeComponent& charge,
                              const ChargeBarUI& bar,
                              const shared::TransformComponent& transform) {
        (void)entity;
        if (!charge.isCharging || bar.displayPercent < 0.01F) {
            return;
        }

        float barX = transform.x + bar.offsetX;
        float barY = transform.y + bar.offsetY;
        _display->drawRectangle({barX, barY}, {bar.barWidth, bar.barHeight},
                                {40, 40, 40, 200}, {100, 100, 100, 255}, 2.0F);

        auto [r, g, b] = bar.getBarColor();
        float filledWidth = bar.barWidth * bar.displayPercent;

        _display->drawRectangle({barX, barY}, {filledWidth, bar.barHeight},
                                {r, g, b, 230}, {0, 0, 0, 0}, 0.0F);

        float marker1X = barX + bar.barWidth * 0.33F;
        float marker2X = barX + bar.barWidth * 0.66F;

        _display->drawRectangle({marker1X, barY}, {2.0F, bar.barHeight},
                                {255, 255, 255, 150}, {0, 0, 0, 0}, 0.0F);

        _display->drawRectangle({marker2X, barY}, {2.0F, bar.barHeight},
                                {255, 255, 255, 150}, {0, 0, 0, 0}, 0.0F);
    });
}

void ChargeVisualSystem::applyScreenShake(float intensity) {
    if (!_display) {
        return;
    }

    if (!_isShaking) {
        _originalViewCenter = _display->getViewCenter();
        _originalViewSize = _display->getViewSize();
        _isShaking = true;
    }

    static thread_local std::random_device rd;
    static thread_local std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(-1.0F, 1.0F);

    float offsetX = dist(rng) * intensity;
    float offsetY = dist(rng) * intensity;

    ::rtype::display::Vector2f newCenter = {_originalViewCenter.x + offsetX,
                                            _originalViewCenter.y + offsetY};
    _display->setView(newCenter, _originalViewSize);
}

void ChargeVisualSystem::resetScreenShake() {
    if (!_display || !_isShaking) return;

    _display->setView(_originalViewCenter, _originalViewSize);
    _isShaking = false;
}

}  // namespace rtype::games::rtype::client
