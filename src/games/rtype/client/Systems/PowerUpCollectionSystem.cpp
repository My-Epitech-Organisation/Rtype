/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PowerUpCollectionSystem - Detects power-up collection and shows popups
*/

#include "PowerUpCollectionSystem.hpp"

#include "../../shared/Components/PlayerIdComponent.hpp"
#include "../../shared/Components/TransformComponent.hpp"
#include "../AllComponents.hpp"
#include "../GameScene/VisualCueFactory.hpp"
#include "Logger/Macros.hpp"

namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

namespace rtype::games::rtype::client {

constexpr float kPowerUpRenewalThreshold = 0.5f;

PowerUpCollectionSystem::PowerUpCollectionSystem(const std::string& font)
    : ::rtype::engine::ASystem("PowerUpCollectionSystem"), _font(font) {
    LOG_INFO("[PowerUpCollectionSystem] Initialized");
}

void PowerUpCollectionSystem::update(ECS::Registry& registry, float /*dt*/) {
    auto playerView =
        registry
            .view<PlayerTag, rs::PlayerIdComponent, rs::TransformComponent>();

    struct PendingPopup {
        float x, y;
        std::string displayName;
        ::rtype::display::Color color;
    };
    std::vector<PendingPopup> pendingPopups;

    size_t playerCount = 0;
    playerView.each([&](auto entity, PlayerTag& /*tag*/,
                        const rs::PlayerIdComponent& playerId,
                        const rs::TransformComponent& transform) {
        playerCount++;

        bool hasActivePowerUp =
            registry.hasComponent<rs::ActivePowerUpComponent>(entity);

        if (!hasActivePowerUp) {
            _lastPowerUpState.erase(playerId.playerId);
            return;
        }

        const auto& activePowerUp =
            registry.getComponent<rs::ActivePowerUpComponent>(entity);
        auto it = _lastPowerUpState.find(playerId.playerId);
        bool isNewCollection = false;

        if (it == _lastPowerUpState.end()) {
            isNewCollection = (activePowerUp.type != rs::PowerUpType::None);
        } else {
            if (it->second.type != activePowerUp.type) {
                isNewCollection = (activePowerUp.type != rs::PowerUpType::None);
            } else {
                float timeDiff =
                    activePowerUp.remainingTime - it->second.remainingTime;
                if (timeDiff > kPowerUpRenewalThreshold) {
                    isNewCollection = true;
                }
            }
        }

        if (isNewCollection) {
            std::string displayName = getPowerUpDisplayName(activePowerUp.type);
            ::rtype::display::Color color = getPowerUpColor(activePowerUp.type);

            LOG_DEBUG(
                "[PowerUpCollectionSystem] Player "
                << playerId.playerId << " collected power-up: " << displayName
                << " (remainingTime=" << activePowerUp.remainingTime << ")");

            pendingPopups.push_back(
                {transform.x + 20.f, transform.y, displayName, color});
        }
        _lastPowerUpState[playerId.playerId] = {activePowerUp.type,
                                                activePowerUp.remainingTime};
    });

    for (const auto& popup : pendingPopups) {
        VisualCueFactory::createPowerUpPopup(
            registry, ::rtype::display::Vector2<float>(popup.x, popup.y),
            popup.displayName, _font, popup.color);
    }
}

std::string PowerUpCollectionSystem::getPowerUpDisplayName(
    rs::PowerUpType type) const {
    switch (type) {
        case rs::PowerUpType::SpeedBoost:
            return "+50% Speed";
        case rs::PowerUpType::Shield:
            return "Shield ON";
        case rs::PowerUpType::RapidFire:
            return "+50% Fire Rate";
        case rs::PowerUpType::DoubleDamage:
            return "x2 Damage";
        case rs::PowerUpType::HealthBoost:
            return "+HP";
        case rs::PowerUpType::ForcePod:
            return "+Force Pod";
        default:
            return "+Power-Up";
    }
}

::rtype::display::Color PowerUpCollectionSystem::getPowerUpColor(
    rs::PowerUpType type) const {
    switch (type) {
        case rs::PowerUpType::SpeedBoost:
            return ::rtype::display::Color(255, 255, 0);  // Yellow
        case rs::PowerUpType::Shield:
            return ::rtype::display::Color(100, 200, 255);  // Blue
        case rs::PowerUpType::RapidFire:
            return ::rtype::display::Color(0, 255, 255);  // Cyan
        case rs::PowerUpType::DoubleDamage:
            return ::rtype::display::Color(255, 128, 0);  // Orange
        case rs::PowerUpType::HealthBoost:
            return ::rtype::display::Color(0, 255, 0);  // Green
        case rs::PowerUpType::ForcePod:
            return ::rtype::display::Color(255, 0, 255);  // Magenta
        default:
            return ::rtype::display::Color::White();
    }
}

}  // namespace rtype::games::rtype::client
