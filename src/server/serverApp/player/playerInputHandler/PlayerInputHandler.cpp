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
#include "games/rtype/shared/Components/ProjectileComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "games/rtype/shared/Components/WeaponComponent.hpp"
#include "network/ServerNetworkSystem.hpp"

namespace rtype::server {

using Transform = rtype::games::rtype::shared::TransformComponent;
using Velocity = rtype::games::rtype::shared::VelocityComponent;
using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;
using WeaponComp = rtype::games::rtype::shared::WeaponComponent;
using ProjectileType = rtype::games::rtype::shared::ProjectileType;

PlayerInputHandler::PlayerInputHandler(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<ServerNetworkSystem> networkSystem,
    std::shared_ptr<GameStateManager> stateManager,
    std::shared_ptr<const IGameConfig> gameConfig, bool verbose)
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
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[InputHandler] Input from userId="
                          << userId
                          << " inputMask=" << static_cast<int>(inputMask)
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

    // Check if player has continuous laser weapon selected
    bool hasLaserWeapon = false;
    if (_registry->hasComponent<WeaponComp>(playerEntity)) {
        const auto& weapon = _registry->getComponent<WeaponComp>(playerEntity);
        hasLaserWeapon = (weapon.getCurrentWeapon().projectileType ==
                         ProjectileType::ContinuousLaser);
    }

    bool isShooting = (inputMask & rtype::network::InputMask::kShoot) != 0;

    if (hasLaserWeapon && _laserCallback && _networkSystem) {
        // Handle laser weapon - call callback every frame with current state
        auto networkIdOpt = _networkSystem->getNetworkId(playerEntity);
        if (networkIdOpt.has_value()) {
            _laserCallback(playerEntity, *networkIdOpt, isShooting);
        }
    } else if (isShooting) {
        // Normal projectile shooting
        processShoot(userId, playerEntity);
    }

    if (inputMask & rtype::network::InputMask::kForcePod) {
        processForcePodLaunch(userId);
    }

    // Handle weapon switch (only on rising edge)
    bool weaponSwitchPressed =
        (inputMask & rtype::network::InputMask::kWeaponSwitch) != 0;
    bool wasWeaponSwitchPressed = _weaponSwitchStates[userId];
    _weaponSwitchStates[userId] = weaponSwitchPressed;

    if (weaponSwitchPressed && !wasWeaponSwitchPressed) {
        processWeaponSwitch(playerEntity);
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
            _registry->hasComponent<Transform>(entity)) {
            auto& pos = _registry->getComponent<Transform>(entity);
            _networkSystem->updateEntityPosition(*networkIdOpt, pos.x, pos.y,
                                                 vx, vy);
        }
    }
}

void PlayerInputHandler::processShoot(std::uint32_t userId,
                                      ECS::Entity entity) {
    if (!_registry->hasComponent<Transform>(entity) ||
        !_registry->hasComponent<ShootCooldown>(entity)) {
        if (_verbose) {
            LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                          "[InputHandler] Player "
                              << userId
                              << " missing Position or ShootCooldown");
        }
        return;
    }

    auto& cooldown = _registry->getComponent<ShootCooldown>(entity);
    if (!cooldown.canShoot()) {
        if (_verbose) {
            LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                          "[InputHandler] Player " << userId
                                                   << " cooldown not ready: "
                                                   << cooldown.currentCooldown);
        }
        return;
    }

    if (!_shootCallback || !_networkSystem) {
        return;
    }

    auto networkIdOpt = _networkSystem->getNetworkId(entity);
    if (!networkIdOpt.has_value()) {
        if (_verbose) {
            LOG_DEBUG_CAT(
                ::rtype::LogCategory::GameEngine,
                "[InputHandler] Player " << userId << " has no networkId");
        }
        return;
    }

    auto& pos = _registry->getComponent<Transform>(entity);
    std::uint32_t projectileId = _shootCallback(*networkIdOpt, pos.x, pos.y);

    if (projectileId != 0) {
        cooldown.triggerCooldown();
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[InputHandler] Player " << userId << " fired projectile "
                                               << projectileId);
    }
}

void PlayerInputHandler::processForcePodLaunch(std::uint32_t userId) {
    if (!_forcePodCallback) {
        return;
    }
    _forcePodCallback(userId);
}

void PlayerInputHandler::processWeaponSwitch(ECS::Entity entity) {
    if (!_registry->hasComponent<WeaponComp>(entity)) {
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[InputHandler] Weapon switch: no WeaponComponent");
        return;
    }

    auto& weapon = _registry->getComponent<WeaponComp>(entity);
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[InputHandler] Weapon switch requested: unlockedSlots="
                     << static_cast<int>(weapon.unlockedSlots)
                     << " currentSlot=" << static_cast<int>(weapon.currentSlot));
    if (weapon.unlockedSlots > 1) {
        weapon.nextWeapon();
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[InputHandler] Weapon switched to slot "
                         << static_cast<int>(weapon.currentSlot));
    } else {
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[InputHandler] Cannot switch: only 1 slot unlocked");
    }
}

}  // namespace rtype::server
