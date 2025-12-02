/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerApp - Main server application with client management
*/

#include "ServerApp.hpp"

#include <algorithm>

namespace rtype::server {

ServerApp::ServerApp(uint16_t port, size_t maxPlayers, uint32_t tickRate,
                     std::atomic<bool>& shutdownFlag,
                     uint32_t clientTimeoutSeconds, bool verbose)
    : _port(port),
      _tickRate(tickRate),
      _clientTimeoutSeconds(clientTimeoutSeconds),
      _verbose(verbose),
      _shutdownFlag(shutdownFlag),
      _clientManager(maxPlayers, _metrics, verbose) {}

ServerApp::~ServerApp() { shutdown(); }

bool ServerApp::run() {
    if (!initialize()) {
        LOG_ERROR("[Server] Failed to initialize server");
        return false;
    }
    logStartupInfo();

    const auto timing = createLoopTiming();
    LoopState state{.previousTime = std::chrono::steady_clock::now()};

    while (!_shutdownFlag.load(std::memory_order_acquire)) {
        const auto frameStartTime = std::chrono::steady_clock::now();

        const auto frameTime = calculateFrameTime(state, timing);
        state.accumulator += frameTime;

        processIncomingData();
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
    LOG_DEBUG("[Server] Client timeout: " << _clientTimeoutSeconds << "s");
}

ServerApp::LoopTiming ServerApp::createLoopTiming() const noexcept {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::nanoseconds;
    const auto fixedDeltaTime =
        duration<double>(1.0 / static_cast<double>(_tickRate));
    return {.fixedDeltaNs = duration_cast<nanoseconds>(fixedDeltaTime),
            .maxFrameTime = duration_cast<nanoseconds>(milliseconds(250)),
            .maxUpdatesPerFrame = 5};
}

std::chrono::nanoseconds ServerApp::calculateFrameTime(
    LoopState& state, const LoopTiming& timing) noexcept {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::nanoseconds;
    using std::chrono::steady_clock;

    const auto currentTime = steady_clock::now();
    auto frameTime =
        duration_cast<nanoseconds>(currentTime - state.previousTime);
    state.previousTime = currentTime;

    if (frameTime > timing.maxFrameTime) {
        _metrics.tickOverruns.fetch_add(1, std::memory_order_relaxed);
        LOG_DEBUG("[Server] Frame time exceeded max ("
                  << duration_cast<milliseconds>(frameTime).count()
                  << "ms), clamping to "
                  << duration_cast<milliseconds>(timing.maxFrameTime).count()
                  << "ms");
        frameTime = timing.maxFrameTime;
    }
    return frameTime;
}

void ServerApp::performFixedUpdates(LoopState& state,
                                    const LoopTiming& timing) noexcept {
    uint32_t updateCount = 0;

    while (state.accumulator >= timing.fixedDeltaNs &&
           updateCount < timing.maxUpdatesPerFrame) {
        _clientManager.checkClientTimeouts(_clientTimeoutSeconds);
        update();
        state.accumulator -= timing.fixedDeltaNs;
        ++updateCount;
    }

    if (updateCount >= timing.maxUpdatesPerFrame &&
        state.accumulator >= timing.fixedDeltaNs) {
        LOG_DEBUG("[Server] Dropping "
                  << (state.accumulator / timing.fixedDeltaNs)
                  << " ticks to catch up (overruns: "
                  << _metrics.tickOverruns.load(std::memory_order_relaxed)
                  << ")");
        state.accumulator = state.accumulator % timing.fixedDeltaNs;
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
    const auto safeSleepTime = duration_cast<nanoseconds>(sleepTime * 95 / 100);
    if (safeSleepTime > microseconds(100)) {
        std::this_thread::sleep_for(safeSleepTime);
    }
    const auto targetTime = frameStartTime + timing.fixedDeltaNs;
    while (steady_clock::now() < targetTime) {
        std::this_thread::yield();
    }
}

void ServerApp::stop() noexcept {
    _shutdownFlag.store(true, std::memory_order_release);
}

bool ServerApp::isRunning() const noexcept {
    return !_shutdownFlag.load(std::memory_order_acquire);
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
    // TODO(Clem): Initialize network socket when rtype_network is fully
    // implemented Example: _socket.bind(_port);
    LOG_DEBUG(
        "[Server] Initialized (network stub - waiting for rtype_network "
        "implementation)");
    return true;
}

void ServerApp::shutdown() noexcept {
    if (_hasShutdown.exchange(true, std::memory_order_acq_rel)) {
        LOG_DEBUG("[Server] Shutdown already performed, skipping");
        return;
    }
    _clientManager.clearAllClients();
    // TODO(Clem): Close network socket
    // _socket.close();
    LOG_DEBUG("[Server] Shutdown complete");
}

void ServerApp::processIncomingData() noexcept {
    // TODO(Clem): Implement when rtype_network is ready
    // This is a stub that simulates receiving data
    //
    // Pseudocode:
    // while (_socket.hasData()) {
    //     Endpoint sender;
    //     auto data = _socket.receive(sender);
    //
    //     // Check if this is a new client (O(1) lookup now!)
    //     auto clientId = _clientManager.findClientByEndpoint(sender);
    //     if (clientId == ClientManager::INVALID_CLIENT_ID) {
    //         clientId = _clientManager.handleNewConnection(sender);
    //     }
    //
    //     // Update last activity time
    //     _clientManager.updateClientActivity(clientId);
    //
    //     // Process the packet
    //     processPacket(clientId, data);
    // }
}

void ServerApp::update() noexcept {
    // TODO(Sam): Update game state via ECS
    // This will be called every tick to update game logic
}

void ServerApp::broadcastGameState() noexcept {
    // TODO(Clem): Send game state to all connected clients
    // This will serialize entity states and send them via the network
}

}  // namespace rtype::server
