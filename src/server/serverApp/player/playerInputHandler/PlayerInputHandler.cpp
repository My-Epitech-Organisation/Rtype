/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PlayerInputHandler - Implementation
*/

#include "PlayerInputHandler.hpp"

#include <rtype/common.hpp>
#include <rtype/network.hpp>

#include "games/rtype/shared/Components/CooldownComponent.hpp"
#include "games/rtype/shared/Components/PositionComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "network/ServerNetworkSystem.hpp"

namespace rtype::server {

using Position = rtype::games::rtype::shared::Position;
using Velocity = rtype::games::rtype::shared::VelocityComponent;
using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;

PlayerInputHandler::PlayerInputHandler(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<ServerNetworkSystem> networkSystem,
    std::shared_ptr<GameStateManager> stateManager,
    std::shared_ptr<const IGameConfig> gameConfig,
    bool verbose)
    : _registry(std::move(registry)),
      _networkSystem(std::move(networkSystem)),
      _stateManager(std::move(stateManager)),
      _gameConfig(std::move(gameConfig)),
      _verbose(verbose) {
    if (_gameConfig && _gameConfig->isInitialized()) {
        _playerSpeed = _gameConfig->getGameplaySettings().playerSpeed;
    }
}

void PlayerInputHandler::handleInput(std::uint32_t userId,
                                     std::uint8_t inputMask,
                                     std::optional<ECS::Entity> entity) {
    if (_stateManager &&
        (_stateManager->isWaiting() || _stateManager->isPaused())) {
        if (!_stateManager->isPlayerReady(userId)) {
            _stateManager->playerReady(userId);
        }
    }

    if (_verbose) {
        LOG_DEBUG("[InputHandler] Input from userId="
                  << userId << " inputMask=" << static_cast<int>(inputMask)
                  << " hasEntity=" << entity.has_value());
    }

    if (_stateManager && !_stateManager->isPlaying()) {
        return;
    }

    if (!entity.has_value()) {
        return;
    }

    ECS::Entity playerEntity = *entity;
    if (!_registry->isAlive(playerEntity)) {
        return;
    }

    processMovement(playerEntity, inputMask);

    if (inputMask & rtype::network::InputMask::kShoot) {
        processShoot(userId, playerEntity);
    }
}

void PlayerInputHandler::processMovement(ECS::Entity entity,
                                         std::uint8_t inputMask) {
    float vx = 0.0F;
    float vy = 0.0F;

    if (inputMask & rtype::network::InputMask::kUp) {
        vy -= _playerSpeed;
    }
    if (inputMask & rtype::network::InputMask::kDown) {
        vy += _playerSpeed;
    }
    if (inputMask & rtype::network::InputMask::kLeft) {
        vx -= _playerSpeed;
    }
    if (inputMask & rtype::network::InputMask::kRight) {
        vx += _playerSpeed;
    }

    if (!_registry->hasComponent<Velocity>(entity)) {
        return;
    }

    auto& vel = _registry->getComponent<Velocity>(entity);
    vel.vx = vx;
    vel.vy = vy;

    if (_networkSystem) {
        auto networkIdOpt = _networkSystem->getNetworkId(entity);
        if (networkIdOpt.has_value() &&
            _registry->hasComponent<Position>(entity)) {
            auto& pos = _registry->getComponent<Position>(entity);
            _networkSystem->updateEntityPosition(*networkIdOpt, pos.x, pos.y,
                                                 vx, vy);
        }
    }
}

void PlayerInputHandler::processShoot(std::uint32_t userId,
                                      ECS::Entity entity) {
    if (!_registry->hasComponent<Position>(entity) ||
        !_registry->hasComponent<ShootCooldown>(entity)) {
        if (_verbose) {
            LOG_DEBUG("[InputHandler] Player "
                      << userId << " missing Position or ShootCooldown");
        }
        return;
    }

    auto& cooldown = _registry->getComponent<ShootCooldown>(entity);
    if (!cooldown.canShoot()) {
        if (_verbose) {
            LOG_DEBUG("[InputHandler] Player "
                      << userId
                      << " cooldown not ready: " << cooldown.currentCooldown);
        }
        return;
    }

    if (!_shootCallback || !_networkSystem) {
        return;
    }

    auto networkIdOpt = _networkSystem->getNetworkId(entity);
    if (!networkIdOpt.has_value()) {
        if (_verbose) {
            LOG_DEBUG("[InputHandler] Player " << userId
                                               << " has no networkId");
        }
        return;
    }

    auto& pos = _registry->getComponent<Position>(entity);
    std::uint32_t projectileId = _shootCallback(*networkIdOpt, pos.x, pos.y);

    if (projectileId != 0) {
        cooldown.triggerCooldown();
        LOG_DEBUG("[InputHandler] Player " << userId << " fired projectile "
                                           << projectileId);
    }
}

}  // namespace rtype::server
