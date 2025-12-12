/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerApp - Implementation (refactored)
*/

#include "ServerApp.hpp"

#include <algorithm>
#include <memory>
#include <span>

#include "server/serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.hpp"
#include "games/rtype/shared/Components/EntityType.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"

namespace rtype::server {

ServerApp::ServerApp(uint16_t port, size_t maxPlayers, uint32_t tickRate,
                     std::shared_ptr<std::atomic<bool>> shutdownFlag,
                     uint32_t clientTimeoutSeconds, bool verbose)
    : _port(port),
      _tickRate(tickRate),
      _clientTimeoutSeconds(clientTimeoutSeconds),
      _verbose(verbose),
      _shutdownFlag(std::move(shutdownFlag)),
      _metrics(std::make_shared<ServerMetrics>()),
      _clientManager(maxPlayers, _metrics, verbose),
      _stateManager(std::make_shared<GameStateManager>(MIN_PLAYERS_TO_START)),
      _packetProcessor(_metrics, verbose) {
    if (tickRate == 0) {
        throw std::invalid_argument("tickRate cannot be zero");
    }
}

ServerApp::ServerApp(std::unique_ptr<IGameConfig> gameConfig,
                     std::shared_ptr<std::atomic<bool>> shutdownFlag,
                     bool verbose)
    : _port(gameConfig && gameConfig->isInitialized()
                ? gameConfig->getServerSettings().port
                : 4000),
      _tickRate(gameConfig && gameConfig->isInitialized()
                    ? gameConfig->getServerSettings().tickRate
                    : 60),
      _clientTimeoutSeconds(DEFAULT_CLIENT_TIMEOUT_SECONDS),
      _verbose(verbose),
      _shutdownFlag(std::move(shutdownFlag)),
      _metrics(std::make_shared<ServerMetrics>()),
      _clientManager(gameConfig && gameConfig->isInitialized()
                         ? gameConfig->getServerSettings().maxPlayers
                         : 4,
                     _metrics, verbose),
      _gameConfig(std::move(gameConfig)),
      _stateManager(std::make_shared<GameStateManager>(MIN_PLAYERS_TO_START)),
      _packetProcessor(_metrics, verbose) {
    if (_gameConfig && _gameConfig->isInitialized()) {
        LOG_INFO("[Server] Configured from game: " << _gameConfig->getGameId());
    } else {
        LOG_WARNING("[Server] Game config not initialized, using defaults");
    }
}

ServerApp::~ServerApp() { shutdown(); }

bool ServerApp::run() {
    if (!initialize()) {
        LOG_ERROR("[Server] Failed to initialize server");
        return false;
    }
    logStartupInfo();

    ServerLoop loop(_tickRate, _shutdownFlag);
    loop.run([this]() { onFrame(); }, [this](float dt) { onUpdate(dt); },
             [this]() { onPostUpdate(); });

    LOG_INFO("[Server] Shutting down...");
    shutdown();
    return true;
}

void ServerApp::onFrame() {
    processIncomingData();
    processRawNetworkData();
}

void ServerApp::onUpdate(float deltaTime) {
    _clientManager.checkClientTimeouts(_clientTimeoutSeconds);

    if (!_stateManager->isPlaying()) {
        return;
    }

    updatePlayerMovement(deltaTime);

    if (_gameEngine && _gameEngine->isRunning()) {
        _gameEngine->update(deltaTime);
    }
    if (_networkSystem) {
        _networkSystem->update();
    }
    if (_eventProcessor) {
        _eventProcessor->processEvents();
    }

    checkGameOverCondition();
}

void ServerApp::onPostUpdate() {
    if (_eventProcessor) {
        _eventProcessor->syncEntityPositions();
    }
    if (_networkSystem) {
        _networkSystem->broadcastEntityUpdates();
    }
}

void ServerApp::logStartupInfo() const noexcept {
    LOG_INFO("[Server] Starting on port " << _port);
    LOG_INFO("[Server] Max players: " << _clientManager.getMaxPlayers());
    LOG_INFO("[Server] Tick rate: " << _tickRate << " Hz");
    LOG_INFO("[Server] State: Waiting for players (need "
             << MIN_PLAYERS_TO_START << " ready to start)");
    LOG_DEBUG("[Server] Client timeout: " << _clientTimeoutSeconds << "s");

    if (_gameConfig && _gameConfig->isInitialized()) {
        const auto gameplay = _gameConfig->getGameplaySettings();
        LOG_INFO("[Server] Game: " << _gameConfig->getGameId());
        LOG_INFO("[Server] Difficulty: " << gameplay.difficulty);
        LOG_INFO("[Server] Starting lives: " << gameplay.startingLives);
    }
}

bool ServerApp::reloadConfiguration() {
    if (!_gameConfig || !_gameConfig->isInitialized()) {
        LOG_WARNING("[Server] Cannot reload - game config not initialized");
        return false;
    }

    if (!_gameConfig->reloadConfiguration()) {
        LOG_ERROR("[Server] Configuration reload failed");
        return false;
    }

    const auto gameplay = _gameConfig->getGameplaySettings();
    LOG_INFO("[Server] Configuration reloaded:");
    LOG_INFO("[Server]   Difficulty: " << gameplay.difficulty);
    LOG_INFO(
        "[Server]   Enemy speed multiplier: " << gameplay.enemySpeedMultiplier);

    const auto serverSettings = _gameConfig->getServerSettings();
    if (serverSettings.port != _port) {
        LOG_WARNING("[Server] Port change requires restart (current: "
                    << _port << ", new: " << serverSettings.port << ")");
    }

    return true;
}

void ServerApp::stop() noexcept {
    _shutdownFlag->store(true, std::memory_order_release);
}

bool ServerApp::isRunning() const noexcept {
    return !_shutdownFlag->load(std::memory_order_acquire);
}

size_t ServerApp::getConnectedClientCount() const noexcept {
    return _clientManager.getConnectedClientCount();
}

std::vector<ClientId> ServerApp::getConnectedClientIds() const {
    return _clientManager.getConnectedClientIds();
}

std::optional<Client> ServerApp::getClientInfo(ClientId clientId) const {
    return _clientManager.getClientInfo(clientId);
}

bool ServerApp::initialize() {
    _registry = std::make_shared<ECS::Registry>();
    _gameEngine = engine::createGameEngine(_registry);
    if (!_gameEngine) {
        LOG_ERROR("[Server] Failed to create game engine");
        return false;
    }
    if (!_gameEngine->initialize()) {
        LOG_ERROR("[Server] Failed to initialize game engine");
        return false;
    }
    LOG_INFO("[Server] Game engine initialized");

    NetworkServer::Config netConfig;
    netConfig.clientTimeout =
        std::chrono::milliseconds(_clientTimeoutSeconds * 1000);
    _networkServer = std::make_shared<NetworkServer>(netConfig);
    _networkSystem =
        std::make_shared<ServerNetworkSystem>(_registry, _networkServer);

    _networkSystem->onClientConnected(
        [this](std::uint32_t userId) { handleClientConnected(userId); });
    _networkSystem->onClientDisconnected(
        [this](std::uint32_t userId) { handleClientDisconnected(userId); });

    std::string gameId = _gameConfig && _gameConfig->isInitialized()
                             ? _gameConfig->getGameId()
                             : "rtype";

    GameEngineOpt gameEngineOpt = std::nullopt;
    if (_gameEngine) {
        gameEngineOpt = std::ref(*_gameEngine);
    }
    GameConfigOpt gameConfigOpt = std::nullopt;
    if (_gameConfig && _gameConfig->isInitialized()) {
        gameConfigOpt = std::cref(*_gameConfig);
    }

    _entitySpawner = EntitySpawnerFactory::create(
        gameId, _registry, _networkSystem, gameEngineOpt, gameConfigOpt);

    if (!_entitySpawner) {
        LOG_ERROR(
            "[Server] Failed to create entity spawner for game: " << gameId);
        return false;
    }
    LOG_INFO("[Server] Entity spawner created for game: " << gameId);

    _inputHandler = std::make_unique<PlayerInputHandler>(
        _registry, _networkSystem, _stateManager, _gameConfig, _verbose);
    _eventProcessor = std::make_unique<GameEventProcessor>(
        _gameEngine, _networkSystem, _verbose,
        [this](const engine::GameEvent& event) { onGameEvent(event); });
    _inputHandler->setShootCallback(
        [this](std::uint32_t networkId, float x, float y) {
            if (!_entitySpawner) {
                return 0u;
            }
            auto entityOpt = _networkSystem->findEntityByNetworkId(networkId);
            if (!entityOpt.has_value()) {
                return 0u;
            }
            return _entitySpawner->handlePlayerShoot(*entityOpt, networkId);
        });
    _networkSystem->setInputHandler([this](std::uint32_t userId,
                                           std::uint8_t inputMask,
                                           std::optional<ECS::Entity> entity) {
        _inputHandler->handleInput(userId, inputMask, entity);
    });

    _stateManager->setStateChangeCallback(
        [this](GameState oldState, GameState newState) {
            handleStateChange(oldState, newState);
        });

    _gameEngine->setEventCallback([this](const engine::GameEvent& event) {
        if (_verbose) {
            LOG_DEBUG("[Server] Game event: type="
                      << static_cast<int>(event.type)
                      << " entityId=" << event.entityNetworkId);
        }
    });

    if (!_networkServer->start(_port)) {
        LOG_ERROR("[Server] Failed to start network server on port " << _port);
        return false;
    }
    LOG_INFO("[Server] Network server started on port " << _port);

    if (!startNetworkThread()) {
        LOG_ERROR("[Server] Failed to start network thread");
        return false;
    }

    LOG_INFO("[Server] Server initialized successfully");
    return true;
}

void ServerApp::shutdown() noexcept {
    if (_hasShutdown.exchange(true, std::memory_order_acq_rel)) {
        LOG_DEBUG("[Server] Shutdown already performed, skipping");
        return;
    }

    stopNetworkThread();
    if (_networkServer) {
        _networkServer->stop();
        LOG_DEBUG("[Server] Network server stopped");
    }
    if (_gameEngine && _gameEngine->isRunning()) {
        _gameEngine->shutdown();
        LOG_DEBUG("[Server] Game engine shutdown");
    }

    _clientManager.clearAllClients();
    LOG_DEBUG("[Server] Shutdown complete");
}

void ServerApp::handleClientConnected(std::uint32_t userId) {
    LOG_INFO("[Server] Client connected: userId=" << userId);
    _metrics->totalConnections.fetch_add(1, std::memory_order_relaxed);

    if (_stateManager->isWaiting()) {
        LOG_INFO("[Server] Waiting for client " << userId
                                                << " to signal ready");
    }

    if (_entitySpawner) {
        PlayerSpawnConfig config{userId, _stateManager->getReadyPlayerCount()};
        auto result = _entitySpawner->spawnPlayer(config);
        if (!result.success) {
            LOG_ERROR("[Server] Failed to spawn player for userId=" << userId);
        }
    }
}

void ServerApp::handleClientDisconnected(std::uint32_t userId) {
    LOG_INFO("[Server] Client disconnected: userId=" << userId);
    _stateManager->playerLeft(userId);

    if (_entitySpawner) {
        _entitySpawner->destroyPlayerByUserId(userId);
    }
}

void ServerApp::handleStateChange(GameState oldState, GameState newState) {
    switch (newState) {
        case GameState::Playing:
            _score = 0;
            LOG_INFO("[Server] *** GAME STARTED *** ("
                     << _stateManager->getReadyPlayerCount() << " players)");
            if (_networkSystem) {
                _networkSystem->broadcastGameStart();
            }
            break;
        case GameState::Paused:
            LOG_INFO("[Server] Game paused - waiting for players to reconnect");
            break;
        case GameState::WaitingForPlayers:
            if (oldState == GameState::Paused) {
                LOG_INFO("[Server] Resuming wait for players");
            }
            break;
        case GameState::GameOver:
            LOG_INFO("[Server] *** GAME OVER *** Final score=" << _score);
            if (_networkSystem) {
                _networkSystem->broadcastGameState(
                    NetworkServer::GameState::GameOver);
                _networkSystem->broadcastGameOver(_score);
            } else if (_networkServer) {
                _networkServer->updateGameState(NetworkServer::GameState::GameOver);
                _networkServer->sendGameOver(_score);
            }
            break;
    }
}

void ServerApp::processIncomingData() noexcept {
    if (_networkServer && _networkServer->isRunning()) {
        _networkServer->poll();
    }

    while (auto packetOpt = _incomingPackets.pop()) {
        auto& [endpoint, packet] = *packetOpt;

        auto clientId = _clientManager.findClientByEndpoint(endpoint);
        if (clientId == ClientManager::INVALID_CLIENT_ID) {
            clientId = _clientManager.handleNewConnection(endpoint);
            if (clientId == ClientManager::INVALID_CLIENT_ID) {
                LOG_WARNING("[Server] Rejected connection from "
                            << endpoint << " (server full)");
                _metrics->connectionsRejected.fetch_add(
                    1, std::memory_order_relaxed);
                continue;
            }
        }

        _clientManager.updateClientActivity(clientId);
    }
}

void ServerApp::processRawNetworkData() noexcept {
    while (auto rawDataOpt = _rawNetworkData.pop()) {
        auto& [endpoint, rawData] = *rawDataOpt;
        auto packetOpt = _packetProcessor.processRawData(
            endpoint.toString(), std::span<const std::uint8_t>(rawData));

        if (packetOpt) {
            _incomingPackets.push({endpoint, std::move(*packetOpt)});
        }
    }
}

bool ServerApp::startNetworkThread() {
    try {
        _networkThreadRunning.store(true, std::memory_order_release);
        _networkThread = std::thread(&ServerApp::networkThreadFunction, this);
        LOG_DEBUG("[Server] Network thread started");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("[Server] Failed to start network thread: " << e.what());
        _networkThreadRunning.store(false, std::memory_order_release);
        return false;
    }
}

void ServerApp::stopNetworkThread() noexcept {
    if (_networkThreadRunning.load(std::memory_order_acquire)) {
        _networkThreadRunning.store(false, std::memory_order_release);
        if (_networkThread.joinable()) {
            _networkThread.join();
        }
        LOG_DEBUG("[Server] Network thread stopped");
    }
}

void ServerApp::networkThreadFunction() noexcept {
    LOG_DEBUG("[Server] Network thread running");

    while (_networkThreadRunning.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    LOG_DEBUG("[Server] Network thread exiting");
}

void ServerApp::updatePlayerMovement(float deltaTime) noexcept {
    if (!_entitySpawner) {
        return;
    }
    _entitySpawner->updateAllPlayersMovement(
        deltaTime,
        [this](uint32_t networkId, float x, float y, float vx, float vy) {
            if (_networkSystem) {
                _networkSystem->updateEntityPosition(networkId, x, y, vx, vy);
            }
        });
}

void ServerApp::checkGameOverCondition() {
    if (!_stateManager || !_stateManager->isPlaying()) {
        return;
    }

    if (countAlivePlayers() > 0) {
        return;
    }

    LOG_INFO("[Server] All players defeated - ending game");

    if (_gameEngine && _gameEngine->isRunning()) {
        _gameEngine->shutdown();
    }

    _stateManager->transitionTo(GameState::GameOver);
}

std::size_t ServerApp::countAlivePlayers() {
    if (!_registry) {
        return 0;
    }

    std::size_t aliveCount = 0;
    auto view = _registry->view<games::rtype::shared::PlayerTag,
                                games::rtype::shared::HealthComponent>();

    view.each([this, &aliveCount](ECS::Entity entity,
                                  const games::rtype::shared::PlayerTag&,
                                  const games::rtype::shared::HealthComponent& health) {
        bool markedForDestroy =
            _registry->hasComponent<games::rtype::shared::DestroyTag>(entity);
        if (health.isAlive() && !markedForDestroy) {
            ++aliveCount;
        }
    });

    return aliveCount;
}

void ServerApp::onGameEvent(const engine::GameEvent& event) {
    if (!_stateManager || !_stateManager->isPlaying()) {
        return;
    }

    if (event.type == engine::GameEventType::EntityDestroyed &&
        event.entityType == static_cast<std::uint8_t>(
                              games::rtype::shared::EntityType::Enemy)) {
        _score += ENEMY_DESTRUCTION_SCORE;
    }
}

}  // namespace rtype::server
