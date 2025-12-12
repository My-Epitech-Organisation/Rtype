/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameSession - Implementation
*/

#include "GameSession.hpp"

#include <algorithm>

#include <rtype/common.hpp>

#include "network/ServerNetworkSystem.hpp"

namespace rtype::server {

namespace InputMask {
constexpr std::uint8_t kUp = 0x01;
constexpr std::uint8_t kDown = 0x02;
constexpr std::uint8_t kLeft = 0x04;
constexpr std::uint8_t kRight = 0x08;
constexpr std::uint8_t kShoot = 0x10;
}  // namespace InputMask

GameSession::GameSession(std::shared_ptr<ECS::Registry> registry,
                         std::shared_ptr<ServerNetworkSystem> networkSystem,
                         std::unique_ptr<IEntitySpawner> entitySpawner,
                         std::shared_ptr<engine::IGameEngine> gameEngine,
                         std::shared_ptr<const IGameConfig> gameConfig,
                         const GameSessionConfig& config)
    : _registry(std::move(registry)),
      _networkSystem(std::move(networkSystem)),
      _entitySpawner(std::move(entitySpawner)),
      _gameEngine(std::move(gameEngine)),
      _gameConfig(std::move(gameConfig)),
      _config(config) {}

void GameSession::update(float deltaTime) {
    if (_state != GameState::Playing) {
        return;
    }

    updatePlayerMovement(deltaTime);

    if (_gameEngine && _gameEngine->isRunning()) {
        _gameEngine->update(deltaTime);
    }

    processGameEvents();
    syncEntityPositions();
}

void GameSession::handleClientConnected(std::uint32_t userId) {
    LOG_INFO("[GameSession] Client connected: userId=" << userId);

    if (_state == GameState::WaitingForPlayers) {
        LOG_INFO("[GameSession] Waiting for client " << userId
                                                     << " to signal ready");
    }

    if (!_entitySpawner) {
        LOG_ERROR("[GameSession] No entity spawner available");
        return;
    }

    PlayerSpawnConfig spawnConfig{};
    spawnConfig.userId = userId;
    spawnConfig.playerIndex = _readyPlayers.size();

    auto result = _entitySpawner->spawnPlayer(spawnConfig);
    if (result.success) {
        LOG_INFO("[GameSession] Spawned player for userId="
                 << userId << " networkId=" << result.networkId << " pos=("
                 << result.x << ", " << result.y << ")");
    } else {
        LOG_ERROR("[GameSession] Failed to spawn player for userId=" << userId);
    }
}

void GameSession::handleClientDisconnected(std::uint32_t userId) {
    LOG_INFO("[GameSession] Client disconnected: userId=" << userId);
    _readyPlayers.erase(userId);

    if (_state == GameState::Playing && _readyPlayers.empty()) {
        transitionToState(GameState::Paused);
    }

    if (_networkSystem) {
        auto entityOpt = _networkSystem->getPlayerEntity(userId);
        if (entityOpt.has_value() && _entitySpawner) {
            _entitySpawner->destroyPlayer(*entityOpt);
            LOG_INFO(
                "[GameSession] Destroyed player entity for userId=" << userId);
        }
    }
}

void GameSession::handleClientInput(std::uint32_t userId,
                                    std::uint8_t inputMask,
                                    std::optional<ECS::Entity> entity) {
    if (_state == GameState::WaitingForPlayers || _state == GameState::Paused) {
        if (_readyPlayers.find(userId) == _readyPlayers.end()) {
            playerReady(userId);
        }
    }

    if (_config.verbose) {
        LOG_DEBUG("[GameSession] Input from userId="
                  << userId << " inputMask=" << static_cast<int>(inputMask)
                  << " hasEntity=" << entity.has_value());
    }

    if (_state != GameState::Playing || !entity.has_value() ||
        !_entitySpawner) {
        return;
    }

    ECS::Entity playerEntity = *entity;
    if (!_registry->isAlive(playerEntity)) {
        return;
    }

    float playerSpeed = _entitySpawner->getPlayerSpeed();
    float vx = 0.0F;
    float vy = 0.0F;

    if (inputMask & InputMask::kUp) {
        vy -= playerSpeed;
    }
    if (inputMask & InputMask::kDown) {
        vy += playerSpeed;
    }
    if (inputMask & InputMask::kLeft) {
        vx -= playerSpeed;
    }
    if (inputMask & InputMask::kRight) {
        vx += playerSpeed;
    }

    _entitySpawner->updatePlayerVelocity(playerEntity, vx, vy);

    auto networkIdOpt = _entitySpawner->getEntityNetworkId(playerEntity);
    if (networkIdOpt.has_value() && _networkSystem) {
        auto posOpt = _entitySpawner->getEntityPosition(playerEntity);
        if (posOpt.has_value()) {
            _networkSystem->updateEntityPosition(*networkIdOpt, posOpt->x,
                                                 posOpt->y, vx, vy);
        }
    }

    if (inputMask & InputMask::kShoot) {
        if (_entitySpawner->canPlayerShoot(playerEntity)) {
            auto netIdOpt = _entitySpawner->getEntityNetworkId(playerEntity);
            if (netIdOpt.has_value()) {
                std::uint32_t projectileId =
                    _entitySpawner->handlePlayerShoot(playerEntity, *netIdOpt);
                if (projectileId != 0) {
                    _entitySpawner->triggerShootCooldown(playerEntity);
                    if (_config.verbose) {
                        LOG_DEBUG("[GameSession] Player "
                                  << userId << " fired projectile "
                                  << projectileId);
                    }
                }
            }
        }
    }
}

void GameSession::playerReady(std::uint32_t userId) {
    if (_state == GameState::Playing) {
        if (_config.verbose) {
            LOG_DEBUG("[GameSession] Player "
                      << userId << " signaled ready but game already running");
        }
        return;
    }

    _readyPlayers.insert(userId);
    LOG_INFO("[GameSession] Player "
             << userId << " is ready (" << _readyPlayers.size() << "/"
             << _config.minPlayersToStart << " needed to start)");

    checkGameStart();
}

void GameSession::transitionToState(GameState newState) {
    if (_state == newState) {
        return;
    }

    LOG_INFO("[GameSession] State transition: " << toString(_state) << " -> "
                                                << toString(newState));

    GameState oldState = _state;
    _state = newState;

    switch (newState) {
        case GameState::Playing:
            LOG_INFO("[GameSession] *** GAME STARTED *** ("
                     << _readyPlayers.size() << " players)");
            if (_networkSystem) {
                _networkSystem->broadcastGameStart();
            }
            break;
        case GameState::Paused:
            LOG_INFO(
                "[GameSession] Game paused - waiting for players to reconnect");
            break;
        case GameState::WaitingForPlayers:
            if (oldState == GameState::Paused) {
                LOG_INFO("[GameSession] Resuming wait for players");
            }
            break;
    }

    if (_stateChangeCallback) {
        _stateChangeCallback(oldState, newState);
    }
}

void GameSession::checkGameStart() {
    if (_state != GameState::WaitingForPlayers && _state != GameState::Paused) {
        return;
    }

    if (_readyPlayers.size() >= _config.minPlayersToStart) {
        transitionToState(GameState::Playing);
    }
}

void GameSession::updatePlayerMovement(float deltaTime) {
    if (!_entitySpawner || !_networkSystem) {
        return;
    }

    _entitySpawner->updateAllPlayersMovement(
        deltaTime,
        [this](std::uint32_t networkId, float x, float y, float vx, float vy) {
            _networkSystem->updateEntityPosition(networkId, x, y, vx, vy);
        });
}

void GameSession::processGameEvents() {
    if (!_gameEngine || !_networkSystem) {
        return;
    }

    auto events = _gameEngine->getPendingEvents();

    for (const auto& event : events) {
        auto processed = _gameEngine->processEvent(event);

        if (!processed.valid) {
            continue;
        }

        switch (processed.type) {
            case engine::GameEventType::EntitySpawned: {
                network::EntityType networkType =
                    static_cast<network::EntityType>(
                        processed.networkEntityType);
                _networkSystem->broadcastEntitySpawn(
                    processed.networkId, networkType, processed.x, processed.y);
                break;
            }
            case engine::GameEventType::EntityDestroyed:
                _networkSystem->unregisterNetworkedEntityById(
                    processed.networkId);
                break;
            case engine::GameEventType::EntityUpdated:
                _networkSystem->updateEntityPosition(
                    processed.networkId, processed.x, processed.y, processed.vx,
                    processed.vy);
                break;
            case engine::GameEventType::EntityHealthChanged:
                _networkSystem->updateEntityHealth(event.entityNetworkId,
                                                   event.healthCurrent,
                                                   event.healthMax);
                break;
        }
    }

    _gameEngine->clearPendingEvents();
}

void GameSession::syncEntityPositions() {
    if (!_networkSystem || !_gameEngine) {
        return;
    }

    _gameEngine->syncEntityPositions(
        [this](uint32_t networkId, float x, float y, float vx, float vy) {
            _networkSystem->updateEntityPosition(networkId, x, y, vx, vy);
        });

    _networkSystem->broadcastEntityUpdates();
}

}  // namespace rtype::server
