/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChargeInputSystem - Implementation
*/

#include "ChargeInputSystem.hpp"

#include "Logger/Macros.hpp"
#include "../Components/ChargeShotVisualComponent.hpp"
#include "../Components/TagComponent.hpp"
#include "../GameScene/RtypeGameScene.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"

namespace rtype::games::rtype::client {

ChargeInputSystem::ChargeInputSystem() : ASystem("ChargeInputSystem") {}

void ChargeInputSystem::update(ECS::Registry& registry, float dt) {
    _chargedShotReleased = false;

    // Read charge shot button state from singleton if available
    if (registry.hasSingleton<ChargeShotInputState>()) {
        _shootPressed = registry.getSingleton<ChargeShotInputState>().isPressed;
        static bool lastLoggedState = false;
        if (_shootPressed != lastLoggedState) {
            LOG_DEBUG_CAT(::rtype::LogCategory::Input,
                          "[ChargeInputSystem] Charge shot button state: "
                              << (_shootPressed ? "PRESSED" : "RELEASED"));
            lastLoggedState = _shootPressed;
        }
    }

    auto view =
        registry.view<shared::ChargeComponent, ControllableTag>();

    size_t entityCount = 0;
    static bool loggedOnce = false;
    if (!loggedOnce && _shootPressed) {
        LOG_DEBUG_CAT(::rtype::LogCategory::Input,
                      "[ChargeInputSystem] Looking for entities with "
                      "ChargeComponent + ControllableTag");
        loggedOnce = true;
    }
    
    view.each([this, dt, &registry, &entityCount](ECS::Entity entity,
                                    shared::ChargeComponent& charge,
                                    ControllableTag& /*tag*/) {
        entityCount++;
        if (_shootPressed && !_wasShootPressed) {
            LOG_DEBUG_CAT(::rtype::LogCategory::Input,
                          "[ChargeInputSystem] Started charging");
            charge.startCharging();
        }

        if (_shootPressed) {
            charge.update(dt);
        }

        if (!_shootPressed && _wasShootPressed && charge.wasCharging) {
            shared::ChargeLevel level = charge.release();

            if (level != shared::ChargeLevel::None) {
                LOG_DEBUG_CAT(
                    ::rtype::LogCategory::Input,
                    "[ChargeInputSystem] Released charge at level "
                        << static_cast<int>(level));
                _chargedShotReleased = true;
                _lastReleasedLevel = level;

                if (registry.hasComponent<ChargeShotVisual>(entity)) {
                    auto& visual =
                        registry.getComponent<ChargeShotVisual>(entity);
                    if (level == shared::ChargeLevel::Level3) {
                        LOG_DEBUG_CAT(
                            ::rtype::LogCategory::Input,
                            "[ChargeInputSystem] Triggering max charge shake!");
                        visual.triggerMaxChargeShake();
                    }
                    visual.reset();
                }

                if (_releaseCallback) {
                    _releaseCallback(level);
                }
            }
        }

        if (!_shootPressed && !charge.isCharging) {
            charge.currentCharge = 0.0F;
            charge.currentLevel = shared::ChargeLevel::None;
        }
    });

    static bool loggedEntityCount = false;
    if (!loggedEntityCount && _shootPressed) {
        LOG_DEBUG_CAT(::rtype::LogCategory::Input,
                      "[ChargeInputSystem] Found " << entityCount
                                                    << " controllable entities");
        loggedEntityCount = true;
    }

    _wasShootPressed = _shootPressed;
}

}  // namespace rtype::games::rtype::client
