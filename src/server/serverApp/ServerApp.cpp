/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerApp - Implementation (refactored)
*/

#include "ServerApp.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <span>
#include <string>

#include "games/rtype/server/GameEngine.hpp"
#include "games/rtype/server/RTypeGameConfig.hpp"
#include "games/rtype/shared/Components/EntityType.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "server/serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.hpp"
#include "shared/NetworkUtils.hpp"

namespace rtype::server {

ServerApp::ServerApp(uint16_t port, size_t maxPlayers, uint32_t tickRate,
                     std::shared_ptr<std::atomic<bool>> shutdownFlag,
                     uint32_t clientTimeoutSeconds, bool verbose,
                     std::shared_ptr<BanManager> banManager)
    : _port(port),
      _tickRate(tickRate),
      _clientTimeoutSeconds(clientTimeoutSeconds),
      _verbose(verbose),
      _shutdownFlag(std::move(shutdownFlag)),
      _metrics(std::make_shared<ServerMetrics>()),
      _banManager(banManager ? std::move(banManager)
                             : std::make_shared<BanManager>()),
      _clientManager(maxPlayers, _metrics, _banManager, verbose),
      _stateManager(std::make_shared<GameStateManager>(MIN_PLAYERS_TO_START)),
      _packetProcessor(_metrics, verbose) {
    if (tickRate == 0) {
        throw std::invalid_argument("tickRate cannot be zero");
    }
}

void ServerApp::setLobbyCode(const std::string& code) {
    if (_networkServer) {
        _networkServer->setExpectedLobbyCode(code);
    }
}

void ServerApp::broadcastMessage(const std::string& message) {
    if (_networkServer) {
        // userId 0 is reserved for system messages
        _networkServer->broadcastChat(0, message);
    }
}

ServerApp::ServerApp(std::unique_ptr<IGameConfig> gameConfig,
                     std::shared_ptr<std::atomic<bool>> shutdownFlag,
                     bool verbose, std::shared_ptr<BanManager> banManager)
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
      _banManager(banManager ? std::move(banManager)
                             : std::make_shared<BanManager>()),
      _clientManager(gameConfig && gameConfig->isInitialized()
                         ? gameConfig->getServerSettings().maxPlayers
                         : 4,
                     _metrics, _banManager, verbose),
      _gameConfig(std::move(gameConfig)),
      _stateManager(std::make_shared<GameStateManager>(MIN_PLAYERS_TO_START)),
      _packetProcessor(_metrics, verbose) {
    if (_gameConfig && _gameConfig->isInitialized()) {
        LOG_INFO_CAT(
            ::rtype::LogCategory::GameEngine,
            "[Server] Configured from game: " << _gameConfig->getGameId());
    } else {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[Server] Game config not initialized, using defaults");
    }
}

ServerApp::~ServerApp() { shutdown(); }

bool ServerApp::run() {
    if (!initialize()) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Failed to initialize server");
        return false;
    }
    logStartupInfo();

    ServerLoop loop(_tickRate, _shutdownFlag);
    _serverLoop = &loop;
    loop.run([this]() { onFrame(); }, [this](float dt) { onUpdate(dt); },
             [this]() { onPostUpdate(); });
    _serverLoop = nullptr;

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine, "[Server] Shutting down...");
    shutdown();
    return true;
}

void ServerApp::onFrame() {
    processIncomingData();
    processRawNetworkData();
}

void ServerApp::onUpdate(float deltaTime) {
    _clientManager.checkClientTimeouts(_clientTimeoutSeconds);

    uint32_t playerCount =
        _networkServer ? _networkServer->getClientCount() : 0;

    _metricSnapshotCounter++;
    if (_metricSnapshotCounter >= METRICS_SNAPSHOT_INTERVAL) {
        _metricSnapshotCounter = 0;

        MetricsSnapshot snapshot;
        snapshot.timestamp = std::chrono::system_clock::now();
        snapshot.playerCount = playerCount;
        snapshot.packetsReceived =
            _metrics->packetsReceived.load(std::memory_order_relaxed);
        snapshot.packetsSent =
            _metrics->packetsSent.load(std::memory_order_relaxed);
        snapshot.bytesReceived =
            _metrics->bytesReceived.load(std::memory_order_relaxed);
        snapshot.bytesSent =
            _metrics->bytesSent.load(std::memory_order_relaxed);

        uint64_t totalPackets =
            snapshot.packetsReceived +
            _metrics->packetsDropped.load(std::memory_order_relaxed);
        snapshot.packetLossPercent =
            (totalPackets > 0)
                ? (100.0 *
                   _metrics->packetsDropped.load(std::memory_order_relaxed) /
                   totalPackets)
                : 0.0;

        snapshot.tickOverruns =
            _serverLoop ? _serverLoop->getTickOverruns() : 0;

        _metrics->addSnapshot(snapshot);

        // BANDWIDTH DEBUG OUTPUT
        // uint64_t totalBytesSent =
        // _metrics->bytesSent.load(std::memory_order_relaxed); uint64_t
        // avgBytesSentPerSec = totalBytesSent / (_metrics->getUptimeSeconds() >
        // 0 ? _metrics->getUptimeSeconds() : 1);

        // LOG_INFO_CAT(::rtype::LogCategory::Network,
        //              "[BANDWIDTH] Uptime: " << _metrics->getUptimeSeconds()
        //              << "s | " "Players: " << playerCount << " | " "Total
        //              sent: " << totalBytesSent << " bytes | " "Avg: " <<
        //              avgBytesSentPerSec << " B/s | " "Packets sent: " <<
        //              snapshot.packetsSent << " | " "Packets received: " <<
        //              snapshot.packetsReceived << " | " "Packet loss: " <<
        //              snapshot.packetLossPercent << "%");
    }

    _stateManager->update(deltaTime);

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
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Starting on port " << _port);
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Max players: " << _clientManager.getMaxPlayers());
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Tick rate: " << _tickRate << " Hz");
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] State: Waiting for players (need "
                     << MIN_PLAYERS_TO_START << " ready to start)");
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[Server] Client timeout: " << _clientTimeoutSeconds << "s");

    if (_gameConfig && _gameConfig->isInitialized()) {
        const auto gameplay = _gameConfig->getGameplaySettings();
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[Server] Game: " << _gameConfig->getGameId());
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[Server] Difficulty: " << gameplay.difficulty);
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[Server] Starting lives: " << gameplay.startingLives);
    }
}

bool ServerApp::reloadConfiguration() {
    if (!_gameConfig || !_gameConfig->isInitialized()) {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[Server] Cannot reload - game config not initialized");
        return false;
    }

    if (!_gameConfig->reloadConfiguration()) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Configuration reload failed");
        return false;
    }

    const auto gameplay = _gameConfig->getGameplaySettings();
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Configuration reloaded:");
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server]   Difficulty: " << gameplay.difficulty);
    LOG_INFO_CAT(
        ::rtype::LogCategory::GameEngine,
        "[Server]   Enemy speed multiplier: " << gameplay.enemySpeedMultiplier);

    const auto serverSettings = _gameConfig->getServerSettings();
    if (serverSettings.port != _port) {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[Server] Port change requires restart (current: "
                            << _port << ", new: " << serverSettings.port
                            << ")");
    }

    return true;
}

void ServerApp::stop() noexcept {
    _shutdownFlag->store(true, std::memory_order_release);
}

void ServerApp::setLobbyManager(LobbyManager* lobbyManager) {
    _lobbyManager = lobbyManager;
    if (_adminServer) {
        _adminServer.reset();
    }

    AdminServer::Config adminConfig;
    if (_gameConfig && _gameConfig->isInitialized()) {
        auto settings = _gameConfig->getServerSettings();
        adminConfig.port = settings.adminPort;
        adminConfig.enabled = settings.adminEnabled;
        adminConfig.localhostOnly = settings.adminLocalhostOnly;
        adminConfig.token = settings.adminToken;
    }

    if (!adminConfig.enabled) {
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[Server] Admin server disabled by configuration");
        return;
    }

    _adminServer =
        std::make_unique<AdminServer>(adminConfig, this, _lobbyManager);
    if (!_adminServer->start()) {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[Server] Failed to start admin server on port "
                            << adminConfig.port
                            << ". Admin panel will be unavailable.");
        _adminServer.reset();
    }
}

bool ServerApp::changeLevel(const std::string& levelId, bool force) {
    if (isPlaying() && !force) {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[Server] Cannot change level while game is running");
        return false;
    }

    std::filesystem::path path(levelId);
    std::string cleanId = path.stem().string();

    if (!force) {
        _initialLevel = cleanId;
    }

    if (_networkServer) {
        _networkServer->setLevelId(cleanId);
        _networkServer->broadcastLevelInfo();
    }

    if (_gameEngine) {
        std::string levelPath = "config/game/levels/" + cleanId + ".toml";
        if (!_gameEngine->loadLevelFromFile(levelPath)) {
            LOG_ERROR_CAT(
                ::rtype::LogCategory::GameEngine,
                "[Server] Failed to load level '" << levelPath << "'");
            return false;
        }
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[Server] Level changed to: " << cleanId);
    }
    return true;
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

std::optional<rtype::Endpoint> ServerApp::getClientEndpoint(
    ClientId clientId) const {
    if (_networkServer) {
        auto epOpt = _networkServer->getClientEndpoint(clientId);
        if (epOpt) {
            return *epOpt;
        }
    }
    return std::nullopt;
}

bool ServerApp::initialize() {
    _registry = std::make_shared<ECS::Registry>();
    _gameEngine = engine::createGameEngine(_registry);
    if (!_gameEngine) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Failed to create game engine");
        return false;
    }

    // Pass laser configuration to game engine before initialization
    if (_gameConfig) {
        auto* rtypeEngine =
            dynamic_cast<games::rtype::server::GameEngine*>(_gameEngine.get());
        auto* rtypeConfig =
            dynamic_cast<games::rtype::server::RTypeGameConfig*>(
                _gameConfig.get());
        if (rtypeEngine && rtypeConfig) {
            rtypeEngine->setLaserConfig(
                rtypeConfig->getRTypeConfig().gameplay.laser);
        }
    }

    if (!_gameEngine->initialize()) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Failed to initialize game engine");
        return false;
    }

    if (!_initialLevel.empty()) {
        std::string levelPath = "config/game/levels/" + _initialLevel + ".toml";
        if (!_gameEngine->loadLevelFromFile(levelPath)) {
            LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                            "[Server] Failed to load level '"
                                << levelPath << "' - using default/fallback");
        } else {
            LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                         "[Server] Level loaded: " << _initialLevel);
        }
    }

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Game engine initialized");

    NetworkServer::Config netConfig;
    netConfig.clientTimeout =
        std::chrono::milliseconds(_clientTimeoutSeconds * 1000);
    netConfig.reliabilityConfig.retransmitTimeout =
        std::chrono::milliseconds(1000);
    netConfig.reliabilityConfig.maxRetries = 15;
    _networkServer = std::make_shared<NetworkServer>(netConfig);
    _networkServer->setMetrics(_metrics);

    _networkServer->setBanManager(_banManager);
    _networkSystem =
        std::make_shared<ServerNetworkSystem>(_registry, _networkServer);

    _networkSystem->onClientConnected(
        [this](std::uint32_t userId) { handleClientConnected(userId); });
    _networkSystem->onClientDisconnected(
        [this](std::uint32_t userId) { handleClientDisconnected(userId); });

    _networkServer->onClientReady([this](std::uint32_t userId, bool isReady) {
        if (isReady) {
            _stateManager->playerReady(userId);
        } else {
            _stateManager->playerNotReady(userId);
        }
    });

    _stateManager->setOnPlayerReadyStateChanged(
        [this](std::uint32_t userId, bool isReady) {
            LOG_INFO("[ServerApp] Broadcasting player "
                     << userId
                     << " ready state: " << (isReady ? "READY" : "NOT READY"));
            _networkServer->broadcastPlayerReadyState(userId, isReady);
        });

    _stateManager->setOnCountdownStarted([this](float duration) {
        LOG_INFO("[ServerApp] Countdown started - broadcasting game start with "
                 << duration << "s");
        _networkServer->broadcastGameStart(duration);
        if (_onGameStartBroadcastCallback) {
            _onGameStartBroadcastCallback(duration);
        }
    });

    _stateManager->setOnCountdownCancelled([this]() {
        LOG_INFO("[ServerApp] Countdown cancelled - broadcasting cancel");
        _networkServer->broadcastGameStart(0.0f);
        if (_onGameStartBroadcastCallback) {
            _onGameStartBroadcastCallback(0.0f);
        }
    });

    _networkServer->onAdminCommand(
        [this](std::uint32_t userId, std::uint8_t commandType,
               std::uint8_t param, const std::string& clientIp) {
            handleAdminCommand(userId, commandType, param, clientIp);
        });

    _networkServer->setGameStateChecker([this]() {
        return _stateManager &&
               (_stateManager->isPlaying() || _stateManager->isPaused() ||
                _stateManager->isGameOver());
    });

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
        LOG_ERROR_CAT(
            ::rtype::LogCategory::GameEngine,
            "[Server] Failed to create entity spawner for game: " << gameId);
        return false;
    }
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Entity spawner created for game: " << gameId);

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

    _inputHandler->setChargedShotCallback([this](std::uint32_t networkId,
                                                 float x, float y,
                                                 std::uint8_t chargeLevel) {
        if (!_gameEngine) {
            return 0u;
        }
        auto* rtypeEngine =
            dynamic_cast<rtype::games::rtype::server::GameEngine*>(
                _gameEngine.get());
        if (!rtypeEngine) {
            return 0u;
        }
        return rtypeEngine->spawnChargedProjectile(networkId, x, y,
                                                   chargeLevel);
    });

    _inputHandler->setForcePodLaunchCallback(
        [this](std::uint32_t playerNetworkId) {
            if (!_gameEngine) {
                return;
            }
            auto* rtypeEngine =
                dynamic_cast<rtype::games::rtype::server::GameEngine*>(
                    _gameEngine.get());
            if (!rtypeEngine) {
                return;
            }
            auto* launchSystem = rtypeEngine->getForcePodLaunchSystem();
            if (launchSystem) {
                launchSystem->handleForcePodInput(rtypeEngine->getRegistry(),
                                                  playerNetworkId);
            }
        });

    _inputHandler->setLaserInputCallback([this](ECS::Entity playerEntity,
                                                std::uint32_t playerNetworkId,
                                                bool isFiring) {
        if (!_gameEngine) {
            return;
        }
        auto* rtypeEngine =
            dynamic_cast<rtype::games::rtype::server::GameEngine*>(
                _gameEngine.get());
        if (!rtypeEngine) {
            return;
        }
        auto* laserSystem = rtypeEngine->getLaserBeamSystem();
        if (laserSystem) {
            laserSystem->handleLaserInput(rtypeEngine->getRegistry(),
                                          playerEntity, playerNetworkId,
                                          isFiring);
        }
    });

    _networkSystem->setInputHandler([this](std::uint32_t userId,
                                           std::uint16_t inputMask,
                                           std::optional<ECS::Entity> entity) {
        _inputHandler->handleInput(userId, inputMask, entity);
    });

    _stateManager->setStateChangeCallback(
        [this](GameState oldState, GameState newState) {
            handleStateChange(oldState, newState);
        });

    if (!rtype::server::isUdpPortAvailable(static_cast<uint16_t>(_port))) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Port "
                          << _port
                          << " unavailable; cannot start network server");
        return false;
    }

    if (!_networkServer->start(_port)) {
        LOG_ERROR_CAT(
            ::rtype::LogCategory::GameEngine,
            "[Server] Failed to start network server on port " << _port);
        return false;
    }
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Network server started on port " << _port);

    if (!startNetworkThread()) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Failed to start network thread");
        return false;
    }

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Server initialized successfully");

    AdminServer::Config adminConfig;
    adminConfig.port = 8080;
    adminConfig.enabled = true;
    adminConfig.localhostOnly = true;
    _adminServer =
        std::make_unique<AdminServer>(adminConfig, this, _lobbyManager);
    if (!_adminServer->start()) {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[Server] Failed to start admin server on port 8080");
    } else {
        LOG_INFO_CAT(
            ::rtype::LogCategory::GameEngine,
            "[Server] Admin panel available at http://localhost:8080/admin");
    }

    return true;
}

void ServerApp::shutdown() noexcept {
    if (_hasShutdown.exchange(true, std::memory_order_acq_rel)) {
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Shutdown already performed, skipping");
        return;
    }

    if (_adminServer) {
        _adminServer->stop();
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Admin server stopped");
    }

    stopNetworkThread();
    if (_networkServer) {
        _networkServer->stop();
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Network server stopped");
    }
    if (_gameEngine && _gameEngine->isRunning()) {
        _gameEngine->shutdown();
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Game engine shutdown");
    }

    _clientManager.clearAllClients();
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[Server] Shutdown complete");
}

void ServerApp::handleClientConnected(std::uint32_t userId) {
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Client connected: userId=" << userId);
    _metrics->totalConnections.fetch_add(1, std::memory_order_relaxed);

    size_t connectedCount = _stateManager->getConnectedPlayerCount() + 1;
    _stateManager->setConnectedPlayerCount(connectedCount);
    LOG_DEBUG("[Server] Connected players: " << connectedCount);

    if (_stateManager->isWaiting()) {
        LOG_INFO_CAT(
            ::rtype::LogCategory::GameEngine,
            "[Server] Waiting for client " << userId << " to signal ready");
    }

    if (_entitySpawner) {
        PlayerSpawnConfig config{userId, _stateManager->getReadyPlayerCount()};
        auto result = _entitySpawner->spawnPlayer(config);
        if (!result.success) {
            LOG_ERROR_CAT(
                ::rtype::LogCategory::GameEngine,
                "[Server] Failed to spawn player for userId=" << userId);
        }
    }
}

void ServerApp::handleClientDisconnected(std::uint32_t userId) {
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Client disconnected: userId=" << userId);
    _stateManager->playerLeft(userId);

    size_t connectedCount = _stateManager->getConnectedPlayerCount();
    if (connectedCount > 0) {
        connectedCount--;
    }
    _stateManager->setConnectedPlayerCount(connectedCount);
    LOG_DEBUG(
        "[Server] Connected players after disconnect: " << connectedCount);

    if (_entitySpawner) {
        _entitySpawner->destroyPlayerByUserId(userId);
    }
}

void ServerApp::handleStateChange(GameState oldState, GameState newState) {
    switch (newState) {
        case GameState::Playing:
            _score = 0;
            LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                         "[Server] *** GAME STARTED *** ("
                             << _stateManager->getReadyPlayerCount()
                             << " players)");
            if (_networkSystem) {
                _networkSystem->broadcastGameStart();
            }

            {
                std::string levelName;
                std::string background;
                std::string levelMusic;

                if (_gameEngine) {
                    auto* rtypeEngine =
                        dynamic_cast<rtype::games::rtype::server::GameEngine*>(
                            _gameEngine.get());
                    if (rtypeEngine && rtypeEngine->getDataDrivenSpawner()) {
                        levelName = rtypeEngine->getDataDrivenSpawner()
                                        ->getWaveManager()
                                        .getLevelName();
                        background = rtypeEngine->getDataDrivenSpawner()
                                         ->getWaveManager()
                                         .getBackground();
                        levelMusic = rtypeEngine->getDataDrivenSpawner()
                                         ->getWaveManager()
                                         .getLevelMusic();
                        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                                      "[Server] Level name from WaveManager: '"
                                          << levelName << "' background: '"
                                          << background << "' music: '"
                                          << levelMusic << "'");
                    }
                }

                if (levelName.empty() && !_initialLevel.empty()) {
                    levelName = _initialLevel;
                    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                                  "[Server] Using _initialLevel as fallback: '"
                                      << levelName << "'");
                }

                if (!levelName.empty() && _networkServer) {
                    LOG_INFO_CAT(
                        ::rtype::LogCategory::GameEngine,
                        "[Server] Broadcasting initial level announce: "
                            << levelName << " background: " << background
                            << " music: " << levelMusic);
                    _networkServer->broadcastLevelAnnounce(
                        levelName, background, levelMusic);
                } else {
                    LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                                    "[Server] No level name to broadcast");
                }
            }
            break;
        case GameState::Paused:
            LOG_INFO_CAT(
                ::rtype::LogCategory::GameEngine,
                "[Server] Game paused - waiting for players to reconnect");
            break;
        case GameState::WaitingForPlayers:
            if (oldState == GameState::Paused) {
                LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                             "[Server] Resuming wait for players");
            } else if (oldState == GameState::GameOver) {
                LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                             "[Server] Back to lobby - waiting for players");
            }
            break;
        case GameState::GameOver:
            LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                         "[Server] *** GAME OVER *** Final score=" << _score);
            LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                         "[Server] Victory Status: "
                             << (_isVictory ? "VICTORY" : "DEFEAT"));

            if (!_isVictory) {
                LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                             "[Server] Sending DEFEAT packet to all clients");
            } else {
                LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                             "[Server] Sending VICTORY packet to all clients");
            }

            if (_networkSystem) {
                LOG_INFO_CAT(
                    ::rtype::LogCategory::GameEngine,
                    "[Server] Broadcasting GameOver via NetworkSystem");
                _networkSystem->broadcastGameState(
                    NetworkServer::GameState::GameOver);
                _networkSystem->broadcastGameOver(_score, _isVictory);
            } else if (_networkServer) {
                LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                             "[Server] Sending GameOver via NetworkServer");
                _networkServer->updateGameState(
                    NetworkServer::GameState::GameOver);
                _networkServer->sendGameOver(_score, _isVictory);
            }
            resetToLobby();
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
                LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                                "[Server] Rejected connection from "
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
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Network thread started");
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Failed to start network thread: " << e.what());
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
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[Server] Network thread stopped");
    }
}

void ServerApp::networkThreadFunction() noexcept {
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[Server] Network thread running");

    while (_networkThreadRunning.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[Server] Network thread exiting");
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

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] DEBUG: checkGameOverCondition triggered");
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Alive players count: 0");
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] All players defeated - ending game (DEFEAT)");
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Setting isVictory = false and shutting down engine");

    _isVictory = false;

    if (_gameEngine && _gameEngine->isRunning()) {
        _gameEngine->shutdown();
    }

    _stateManager->transitionTo(GameState::GameOver);
}

void ServerApp::resetToLobby() {
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Resetting session to lobby");

    if (_registry) {
        _registry->removeEntitiesIf([](ECS::Entity) { return true; });
        _registry->cleanupTombstones();
    }
    if (_networkSystem) {
        _networkSystem->resetState();
        _networkSystem->broadcastGameState(NetworkServer::GameState::Lobby);
    } else if (_networkServer) {
        _networkServer->updateGameState(NetworkServer::GameState::Lobby);
    }

    _score = 0;
    _isVictory = false;

    if (_gameEngine) {
        if (_gameEngine->isRunning()) {
            _gameEngine->shutdown();
        }
        _gameEngine->initialize();

        if (_initialLevel.empty()) {
            _initialLevel = "level_1";
        }

        std::string levelPath = "config/game/levels/" + _initialLevel + ".toml";
        if (!_gameEngine->loadLevelFromFile(levelPath)) {
            LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                            "[Server] Failed to reload level '"
                                << levelPath << "' during lobby reset");
        } else {
            LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                         "[Server] Level reset to: " << _initialLevel);
        }
    }
    if (_stateManager) {
        _stateManager->reset();
    }
    if (_entitySpawner) {
        auto connected = getConnectedClientIds();
        std::size_t idx = 0;
        for (auto userId : connected) {
            PlayerSpawnConfig cfg{userId, idx++};
            auto result = _entitySpawner->spawnPlayer(cfg);
            if (!result.success) {
                LOG_ERROR_CAT(
                    ::rtype::LogCategory::GameEngine,
                    "[Server] Failed to respawn player for userId=" << userId);
            }
        }
    }
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[Server] Reset to lobby complete");
}

std::size_t ServerApp::countAlivePlayers() {
    if (!_registry) {
        LOG_WARNING_CAT(
            ::rtype::LogCategory::GameEngine,
            "[ServerApp] countAlivePlayers: _registry is null! Returning 0.");
        return 0;
    }

    std::size_t aliveCount = 0;
    std::size_t totalPlayers = 0;
    auto view = _registry->view<games::rtype::shared::PlayerTag,
                                games::rtype::shared::HealthComponent>();

    view.each([this, &aliveCount, &totalPlayers](
                  ECS::Entity entity, const games::rtype::shared::PlayerTag&,
                  const games::rtype::shared::HealthComponent& health) {
        bool markedForDestroy =
            _registry->hasComponent<games::rtype::shared::DestroyTag>(entity);
        totalPlayers++;

        bool isAlive = health.isAlive();
        if (isAlive && !markedForDestroy) {
            ++aliveCount;
        } else {
            // Only log why we think this player is dead if we are about to
            // return 0 total alive
        }
    });

    if (aliveCount == 0) {
        LOG_INFO_CAT(
            ::rtype::LogCategory::GameEngine,
            "[ServerApp] countAlivePlayers returned 0. Detailed scan:");
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "  - Registry p: " << _registry.get());

        view.each([this](ECS::Entity entity,
                         const games::rtype::shared::PlayerTag&,
                         const games::rtype::shared::HealthComponent& health) {
            bool markedForDestroy =
                _registry->hasComponent<games::rtype::shared::DestroyTag>(
                    entity);
            LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                         "  - Player Entity "
                             << entity.id << ": Health=" << health.current
                             << "/" << health.max
                             << " Alive=" << health.isAlive()
                             << " DestroyTag=" << markedForDestroy);
        });

        if (totalPlayers == 0) {
            LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                         "  - NO PlayerTag entities found in registry!");
            _registry->view<games::rtype::shared::PlayerTag>().each(
                [](auto entity, auto&) {
                    LOG_INFO_CAT(
                        ::rtype::LogCategory::GameEngine,
                        "  - Found entity "
                            << entity.id
                            << " with PlayerTag but MISSING HealthComponent??");
                });
        }
    }

    return aliveCount;
}

void ServerApp::handleAdminCommand(std::uint32_t userId,
                                   std::uint8_t commandType, std::uint8_t param,
                                   const std::string& clientIp) {
    // Validate localhost - only allow admin commands from the same machine
    bool isLocalhost = (clientIp == "127.0.0.1" || clientIp == "::1" ||
                        clientIp.rfind("127.", 0) == 0);

    if (!isLocalhost) {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[Server] Admin command rejected: "
                            << clientIp << " is not localhost");
        _networkServer->sendAdminResponse(
            userId, commandType, false, 0,
            "Admin commands only available from localhost");
        return;
    }

    switch (static_cast<network::AdminCommandType>(commandType)) {
        case network::AdminCommandType::GodMode: {
            auto entityOpt = _networkSystem->findEntityByNetworkId(userId);
            if (!entityOpt.has_value()) {
                _networkServer->sendAdminResponse(userId, commandType, false, 0,
                                                  "Player entity not found");
                return;
            }

            ECS::Entity playerEntity = entityOpt.value();
            bool hasGodMode =
                _registry->hasComponent<games::rtype::shared::InvincibleTag>(
                    playerEntity);
            bool newState;

            if (param == 2) {
                newState = !hasGodMode;
            } else {
                newState = (param == 1);
            }

            if (newState && !hasGodMode) {
                _registry
                    ->emplaceComponent<games::rtype::shared::InvincibleTag>(
                        playerEntity);
                LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                             "[Server] God mode ENABLED for userId=" << userId);
            } else if (!newState && hasGodMode) {
                _registry->removeComponent<games::rtype::shared::InvincibleTag>(
                    playerEntity);
                LOG_INFO_CAT(
                    ::rtype::LogCategory::GameEngine,
                    "[Server] God mode DISABLED for userId=" << userId);
            }

            std::string msg =
                newState ? "God mode enabled" : "God mode disabled";
            _networkServer->sendAdminResponse(userId, commandType, true,
                                              newState ? 1 : 0, msg);
            break;
        }
        default:
            _networkServer->sendAdminResponse(userId, commandType, false, 0,
                                              "Unknown command type");
            break;
    }
}

void ServerApp::onGameEvent(const engine::GameEvent& event) {
    if (!_stateManager) {
        return;
    }

    if (event.type == engine::GameEventType::GameOver) {
        auto* rtypeEngine =
            dynamic_cast<rtype::games::rtype::server::GameEngine*>(
                _gameEngine.get());

        if (rtypeEngine) {
            auto nextLevel =
                rtypeEngine->getDataDrivenSpawner()->getNextLevel();

            if (nextLevel && !nextLevel->empty()) {
                LOG_WARNING_CAT(
                    ::rtype::LogCategory::GameEngine,
                    "[ServerApp] Ignoring GameOver event because next level '"
                        << *nextLevel << "' is available");
                return;
            }
        }

        LOG_INFO_CAT(
            ::rtype::LogCategory::GameEngine,
            "[ServerApp] GameOver event received (VICTORY), transitioning to "
            "GameOver state");
        _isVictory = true;
        _stateManager->transitionTo(GameState::GameOver);
        return;
    }

    if (event.type == engine::GameEventType::LevelComplete) {
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[ServerApp] LevelComplete event received");

        auto* rtypeEngine =
            dynamic_cast<rtype::games::rtype::server::GameEngine*>(
                _gameEngine.get());
        if (rtypeEngine) {
            auto nextLevel =
                rtypeEngine->getDataDrivenSpawner()->getNextLevel();

            if (nextLevel) {
                LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                             "[ServerApp] Raw next_level found in config: '"
                                 << *nextLevel << "'");
            } else {
                LOG_INFO_CAT(
                    ::rtype::LogCategory::GameEngine,
                    "[ServerApp] No next_level found in configuration");
            }

            if (nextLevel && !nextLevel->empty()) {
                std::string nextId = *nextLevel;
                std::filesystem::path path(nextId);
                std::string cleanId = path.stem().string();

                LOG_INFO_CAT(
                    ::rtype::LogCategory::GameEngine,
                    "[ServerApp] Transitioning to next level: " << cleanId);

                changeLevel(cleanId, true);
                rtypeEngine->startLevel();

                auto levelName = rtypeEngine->getDataDrivenSpawner()
                                     ->getWaveManager()
                                     .getLevelName();
                auto background = rtypeEngine->getDataDrivenSpawner()
                                      ->getWaveManager()
                                      .getBackground();
                auto levelMusic = rtypeEngine->getDataDrivenSpawner()
                                      ->getWaveManager()
                                      .getLevelMusic();
                if (levelName.empty()) {
                    levelName = cleanId;
                }

                if (_networkServer) {
                    _networkServer->broadcastLevelAnnounce(
                        levelName, background, levelMusic);
                }
            } else {
                LOG_INFO_CAT(
                    ::rtype::LogCategory::GameEngine,
                    "[ServerApp] No next level, ending game (VICTORY)");
                _isVictory = true;
                _stateManager->transitionTo(GameState::GameOver);
            }
        }
        return;
    }

    if (!_stateManager->isPlaying()) {
        return;
    }

    if (event.type == engine::GameEventType::EntityDestroyed &&
        event.entityType == static_cast<std::uint8_t>(
                                games::rtype::shared::EntityType::Enemy)) {
        _score += ENEMY_DESTRUCTION_SCORE;
    }
}

}  // namespace rtype::server
