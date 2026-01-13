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

#include "Logger/Macros.hpp"
#include "../AllComponents.hpp"
#include "../Components/ChargeShotVisualComponent.hpp"
#include "../Components/ColorTintComponent.hpp"
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
        visual.updateGlow(charge.currentLevel);
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

        if (visual.shouldShake) {
            LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                          "[ChargeVisualSystem] Applying screen shake: "
                              << visual.shakeIntensity);
            applyScreenShake(visual.shakeIntensity);
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

void ChargeVisualSystem::applyScreenShake(float intensity) {
    if (!_display) return;

    if (!_isShaking) {
        _originalViewCenter = _display->getViewCenter();
        _originalViewSize = _display->getViewSize();
        _isShaking = true;
    }

    static thread_local std::mt19937 rng(std::random_device{}());
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
