/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerApp - Main server application with client management
*/

#include "ServerApp.hpp"

#include <algorithm>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace rtype::server {

ServerApp::ServerApp(uint16_t port, size_t maxPlayers, uint32_t tickRate,
                     std::shared_ptr<std::atomic<bool>> shutdownFlag,
                     uint32_t clientTimeoutSeconds, bool verbose)
    : _port(port),
      _tickRate(tickRate),
      _clientTimeoutSeconds(clientTimeoutSeconds),
      _verbose(verbose),
      _shutdownFlag(shutdownFlag),
      _metrics(std::make_shared<ServerMetrics>()),
      _clientManager(maxPlayers, _metrics, verbose) {
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
      _shutdownFlag(shutdownFlag),
      _metrics(std::make_shared<ServerMetrics>()),
      _clientManager(gameConfig && gameConfig->isInitialized()
                         ? gameConfig->getServerSettings().maxPlayers
                         : 4,
                     _metrics, verbose),
      _gameConfig(std::move(gameConfig)) {
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

    const auto timing = createLoopTiming();
    auto state = std::make_shared<LoopState>();
    state->previousTime = std::chrono::steady_clock::now();

    while (!_shutdownFlag->load(std::memory_order_acquire)) {
        const auto frameStartTime = std::chrono::steady_clock::now();

        const auto frameTime = calculateFrameTime(state, timing);
        state->accumulator += frameTime;

        processIncomingData();
        processRawNetworkData();
        performFixedUpdates(state, timing);
        broadcastGameState();
        sleepUntilNextFrame(frameStartTime, timing);
    }

    LOG_INFO("[Server] Shutting down...");
    shutdown();
    return true;
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

ServerApp::LoopTiming ServerApp::createLoopTiming() const noexcept {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::nanoseconds;
    const auto fixedDeltaTime =
        duration<double>(1.0 / static_cast<double>(_tickRate));
    return {.fixedDeltaNs = duration_cast<nanoseconds>(fixedDeltaTime),
            .maxFrameTime =
                duration_cast<nanoseconds>(milliseconds(MAX_FRAME_TIME_MS)),
            .maxUpdatesPerFrame = MAX_UPDATES_PER_FRAME};
}

std::chrono::nanoseconds ServerApp::calculateFrameTime(
    std::shared_ptr<LoopState> state, const LoopTiming& timing) noexcept {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::nanoseconds;
    using std::chrono::steady_clock;

    const auto currentTime = steady_clock::now();
    auto frameTime =
        duration_cast<nanoseconds>(currentTime - state->previousTime);
    state->previousTime = currentTime;

    if (frameTime > timing.maxFrameTime) {
        _metrics->tickOverruns.fetch_add(1, std::memory_order_relaxed);
        LOG_DEBUG("[Server] Frame time exceeded max ("
                  << duration_cast<milliseconds>(frameTime).count()
                  << "ms), clamping to "
                  << duration_cast<milliseconds>(timing.maxFrameTime).count()
                  << "ms");
        frameTime = timing.maxFrameTime;
    }
    return frameTime;
}

void ServerApp::performFixedUpdates(std::shared_ptr<LoopState> state,
                                    const LoopTiming& timing) noexcept {
    uint32_t updateCount = 0;

    while (state->accumulator >= timing.fixedDeltaNs &&
           updateCount < timing.maxUpdatesPerFrame) {
        _clientManager.checkClientTimeouts(_clientTimeoutSeconds);
        update();
        state->accumulator -= timing.fixedDeltaNs;
        ++updateCount;
    }

    if (updateCount >= timing.maxUpdatesPerFrame &&
        state->accumulator >= timing.fixedDeltaNs) {
        LOG_DEBUG("[Server] Dropping "
                  << (state->accumulator / timing.fixedDeltaNs)
                  << " ticks to catch up (overruns: "
                  << _metrics->tickOverruns.load(std::memory_order_relaxed)
                  << ")");
        state->accumulator = state->accumulator % timing.fixedDeltaNs;
    }
}

void ServerApp::sleepUntilNextFrame(
    std::chrono::steady_clock::time_point frameStartTime,
    const LoopTiming& timing) noexcept {
    using std::chrono::duration_cast;
    using std::chrono::microseconds;
    using std::chrono::nanoseconds;
    using std::chrono::steady_clock;
    const auto elapsed = steady_clock::now() - frameStartTime;
    const auto sleepTime = timing.fixedDeltaNs - elapsed;

    if (sleepTime <= nanoseconds{0}) {
        return;
    }
    const auto safeSleepTime =
        duration_cast<nanoseconds>(sleepTime * SLEEP_TIME_SAFETY_PERCENT / 100);
    if (safeSleepTime > microseconds(MIN_SLEEP_THRESHOLD_US)) {
        std::this_thread::sleep_for(safeSleepTime);
    }
    const auto targetTime = frameStartTime + timing.fixedDeltaNs;
    while (steady_clock::now() < targetTime) {
        std::this_thread::yield();
    }
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
    _gameEngine = engine::createGameEngine();
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
    netConfig.clientTimeout = std::chrono::milliseconds(_clientTimeoutSeconds * 1000);
    _networkServer = std::make_shared<NetworkServer>(netConfig);
    _networkSystem = std::make_unique<ServerNetworkSystem>(_registry, _networkServer);
    _networkSystem->onClientConnected(
        [this](std::uint32_t userId) { handleClientConnected(userId); });
    _networkSystem->onClientDisconnected(
        [this](std::uint32_t userId) { handleClientDisconnected(userId); });
    _networkSystem->setInputHandler(
        [this](std::uint32_t userId, std::uint8_t inputMask,
               std::optional<ECS::Entity> entity) {
            handleClientInput(userId, inputMask, entity);
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
                            << endpoint << " (server full or invalid state)");
                _metrics->connectionsRejected.fetch_add(
                    1, std::memory_order_relaxed);
                continue;
            }
        }

        _clientManager.updateClientActivity(clientId);

        processPacket(clientId, packet);
    }
}

void ServerApp::processRawNetworkData() noexcept {
    while (auto rawDataOpt = _rawNetworkData.pop()) {
        auto& [endpoint, rawData] = *rawDataOpt;

        if (auto packetOpt = extractPacketFromData(endpoint, rawData)) {
            _incomingPackets.push({endpoint, std::move(*packetOpt)});
        }
    }
}

void ServerApp::update() noexcept {
    if (_gameState != GameState::Playing) {
        return;
    }
    const float deltaTime = 1.0F / static_cast<float>(_tickRate);

    if (_gameEngine && _gameEngine->isRunning()) {
        _gameEngine->update(deltaTime);
    }
    if (_networkSystem) {
        _networkSystem->update();
    }
    processGameEvents();
    syncEntityPositions();
}

std::optional<rtype::network::Packet> ServerApp::extractPacketFromData(
    const Endpoint& endpoint, const std::vector<uint8_t>& rawData) noexcept {
    try {
        auto validationResult =
            rtype::network::Serializer::validateAndExtractPacket(
                std::span<const std::uint8_t>(rawData), false);

        if (validationResult.isErr()) {
            LOG_DEBUG("[Server] Dropped packet from "
                      << endpoint << " (validation error: "
                      << rtype::network::toString(validationResult.error())
                      << ")");
            _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
            return std::nullopt;
        }

        auto [header, payload] = validationResult.value();

        std::string connectionKey = endpoint.toString();
        auto seqResult =
            _securityContext.validateSequenceId(connectionKey, header.seqId);
        if (seqResult.isErr()) {
            LOG_DEBUG("[Server] Dropped packet from "
                      << endpoint << " (invalid sequence: "
                      << rtype::network::toString(seqResult.error())
                      << ", SeqID=" << header.seqId << ")");
            _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
            return std::nullopt;
        }

        auto userIdResult = _securityContext.validateUserIdMapping(
            connectionKey, header.userId);
        if (userIdResult.isErr()) {
            LOG_WARNING("[Server] Dropped packet from "
                        << endpoint << " (UserID spoofing: claimed="
                        << header.userId << ")");
            _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
            return std::nullopt;
        }

        rtype::network::Packet packet(
            static_cast<rtype::network::PacketType>(header.opcode));
        if (header.payloadSize > 0) {
            std::vector<uint8_t> payloadData(payload.begin(), payload.end());
            packet.setData(payloadData);
        }

        LOG_DEBUG("[Server] Accepted packet from "
                  << endpoint << " (OpCode=" << static_cast<int>(header.opcode)
                  << ", SeqID=" << header.seqId << ", UserID=" << header.userId
                  << ", Payload=" << header.payloadSize << " bytes)");

        return packet;
    } catch (const std::exception& e) {
        LOG_ERROR("[Server] Exception extracting packet from "
                  << endpoint << ": " << e.what());
        _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
        return std::nullopt;
    } catch (...) {
        LOG_ERROR("[Server] Unknown exception extracting packet from "
                  << endpoint);
        _metrics->packetsDropped.fetch_add(1, std::memory_order_relaxed);
        return std::nullopt;
    }
}

void ServerApp::broadcastGameState() noexcept {
    if (_networkSystem) {
        _networkSystem->broadcastEntityUpdates();
    }
}

void ServerApp::processPacket(ClientId clientId,
                              const rtype::network::Packet& packet) noexcept {
    if (_verbose) {
        LOG_DEBUG("[Server] Legacy packet processing from client "
                  << clientId << " of type " << static_cast<int>(packet.type()));
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

void ServerApp::handleClientConnected(std::uint32_t userId) {
    LOG_INFO("[Server] Client connected: userId=" << userId);
    _metrics->totalConnections.fetch_add(1, std::memory_order_relaxed);

    if (_gameState == GameState::WaitingForPlayers) {
        LOG_INFO("[Server] Waiting for client " << userId
                 << " to signal ready (send START_GAME packet)");
    }

    // TODO(Priority 2): Spawn player entity for this client
    // This will be implemented in the next phase:
    // 1. Create player entity in registry
    // 2. Register it with network system
    // 3. Associate userId with entity
}

void ServerApp::handleClientDisconnected(std::uint32_t userId) {
    LOG_INFO("[Server] Client disconnected: userId=" << userId);
    _readyPlayers.erase(userId);
    if (_gameState == GameState::Playing && _readyPlayers.empty()) {
        transitionToState(GameState::Paused);
    }

    // TODO(Priority 2): Handle player entity cleanup
    // This will be implemented in the next phase:
    // 1. Find player entity associated with userId
    // 2. Destroy entity in registry
    // 3. Notify other clients
}

void ServerApp::handleClientInput(std::uint32_t userId, std::uint8_t inputMask,
                                  std::optional<ECS::Entity> entity) {
    if (_gameState == GameState::WaitingForPlayers ||
        _gameState == GameState::Paused) {
        if (_readyPlayers.find(userId) == _readyPlayers.end()) {
            handlePlayerReady(userId);
        }
    }
    if (_verbose) {
        LOG_DEBUG("[Server] Input from userId=" << userId
                  << " inputMask=" << static_cast<int>(inputMask)
                  << " hasEntity=" << entity.has_value());
    }
    if (_gameState != GameState::Playing) {
        return;
    }

    // TODO(Priority 2): Apply input to player entity
    // This will be implemented in the next phase:
    // 1. Validate entity exists
    // 2. Apply input to velocity/movement components
    // 3. Server-authoritative movement
    (void)entity;  // Suppress unused warning for now
}

void ServerApp::processGameEvents() {
    if (!_gameEngine || !_networkSystem) {
        return;
    }
    auto events = _gameEngine->getPendingEvents();

    for (const auto& event : events) {
        switch (event.type) {
            case engine::GameEventType::EntitySpawned: {
                if (_verbose) {
                    LOG_DEBUG("[Server] Entity spawned: networkId="
                              << event.entityNetworkId
                              << " pos=(" << event.x << ", " << event.y << ")");
                }
                break;
            }
            case engine::GameEventType::EntityDestroyed: {
                _networkSystem->unregisterNetworkedEntityById(
                    event.entityNetworkId);
                if (_verbose) {
                    LOG_DEBUG("[Server] Entity destroyed: networkId="
                              << event.entityNetworkId);
                }
                break;
            }
            case engine::GameEventType::EntityUpdated: {
                _networkSystem->updateEntityPosition(
                    event.entityNetworkId, event.x, event.y, 0.0F, 0.0F);
                break;
            }
        }
    }
    _gameEngine->clearPendingEvents();
}

void ServerApp::syncEntityPositions() {
    if (_networkSystem) {
        _networkSystem->broadcastEntityUpdates();
    }
}

void ServerApp::playerReady(std::uint32_t userId) {
    handlePlayerReady(userId);
}

void ServerApp::handlePlayerReady(std::uint32_t userId) {
    if (_gameState == GameState::Playing) {
        LOG_DEBUG("[Server] Player " << userId
                  << " signaled ready but game already running");
        return;
    }

    _readyPlayers.insert(userId);
    LOG_INFO("[Server] Player " << userId << " is ready ("
             << _readyPlayers.size() << "/" << MIN_PLAYERS_TO_START
             << " needed to start)");

    checkGameStart();
}

void ServerApp::checkGameStart() {
    if (_gameState != GameState::WaitingForPlayers &&
        _gameState != GameState::Paused) {
        return;
    }

    if (_readyPlayers.size() >= MIN_PLAYERS_TO_START) {
        transitionToState(GameState::Playing);
    }
}

void ServerApp::transitionToState(GameState newState) {
    if (_gameState == newState) {
        return;
    }

    const auto stateToString = [](GameState state) -> const char* {
        switch (state) {
            case GameState::WaitingForPlayers: return "WaitingForPlayers";
            case GameState::Playing: return "Playing";
            case GameState::Paused: return "Paused";
            default: return "Unknown";
        }
    };

    LOG_INFO("[Server] State transition: " << stateToString(_gameState)
             << " -> " << stateToString(newState));

    GameState oldState = _gameState;
    _gameState = newState;

    switch (newState) {
        case GameState::Playing: {
            LOG_INFO("[Server] *** GAME STARTED *** ("
                     << _readyPlayers.size() << " players)");
            if (_networkSystem) {
                _networkSystem->broadcastGameStart();
            }
            break;
        }
        case GameState::Paused: {
            LOG_INFO("[Server] Game paused - waiting for players to reconnect");
            break;
        }
        case GameState::WaitingForPlayers: {
            if (oldState == GameState::Paused) {
                LOG_INFO("[Server] Resuming wait for players");
            }
            break;
        }
    }
}

}  // namespace rtype::server
