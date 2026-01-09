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

    size_t playerCount = 0;
    playerView.each([&](auto entity, PlayerTag& /*tag*/,
                        const rs::PlayerIdComponent& playerId,
                        const rs::TransformComponent& transform) {
        playerCount++;

        bool hasActivePowerUp =
            registry.hasComponent<rs::ActivePowerUpComponent>(entity);
        LOG_DEBUG("[PowerUpCollectionSystem] Checking player "
                  << playerId.playerId << " (entity=" << entity.id
                  << ") hasActivePowerUp=" << hasActivePowerUp);

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

            VisualCueFactory::createPowerUpPopup(
                registry,
                ::rtype::display::Vector2<float>(transform.x + 20.f,
                                                 transform.y),
                displayName, _font, color);
        }
        _lastPowerUpState[playerId.playerId] = {activePowerUp.type,
                                                activePowerUp.remainingTime};
    });

    LOG_DEBUG("[PowerUpCollectionSystem] update() finished, checked "
              << playerCount << " players");
}

std::string PowerUpCollectionSystem::getPowerUpDisplayName(
    rs::PowerUpType type) const {
    switch (type) {
        case rs::PowerUpType::SpeedBoost:
            return "+Speed";
        case rs::PowerUpType::Shield:
            return "+Shield";
        case rs::PowerUpType::RapidFire:
            return "+Rapid Fire";
        case rs::PowerUpType::DoubleDamage:
            return "+Double Damage";
        case rs::PowerUpType::HealthBoost:
            return "+Health";
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
        default:
            return ::rtype::display::Color::White();
    }
}

}  // namespace rtype::games::rtype::client
