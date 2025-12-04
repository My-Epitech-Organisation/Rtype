/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerApp - Main server application with client management
*/

#include "ServerApp.hpp"

#include <algorithm>
#include <memory>
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
      _clientManager(maxPlayers, _metrics, verbose) {}

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
    // TODO(Clem): Initialize network socket when rtype_network is fully
    // implemented Example: _socket.bind(_port);

    if (!startNetworkThread()) {
        LOG_ERROR("[Server] Failed to start network thread");
        return false;
    }

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

    stopNetworkThread();
    _clientManager.clearAllClients();

    // TODO(Clem): Close network socket
    // _socket.close();
    LOG_DEBUG("[Server] Shutdown complete");
}

void ServerApp::processIncomingData() noexcept {
    while (auto packetOpt = _incomingPackets.pop()) {
        auto& [endpoint, packet] = *packetOpt;

        auto clientId = _clientManager.findClientByEndpoint(endpoint);
        if (clientId == ClientManager::INVALID_CLIENT_ID) {
            clientId = _clientManager.handleNewConnection(endpoint);
            if (clientId == ClientManager::INVALID_CLIENT_ID) {
                // TODO(Anyone): Log rejection reason and update metrics when
                // client connection is rejected
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
    // TODO(Sam): Update game state via ECS
    // This will be called every tick to update game logic
}

std::optional<rtype::network::Packet> ServerApp::extractPacketFromData(
    const Endpoint& endpoint, const std::vector<uint8_t>& rawData) noexcept {
    try {
        if (rawData.size() < rtype::network::kHeaderSize) {
            LOG_DEBUG("[Server] Received packet too small from "
                      << endpoint << " (" << rawData.size() << " bytes)");
            return std::nullopt;
        }

        auto header = rtype::network::Serializer::deserializeFromNetwork<
            rtype::network::Header>(std::vector<uint8_t>(
            rawData.begin(), rawData.begin() + rtype::network::kHeaderSize));

        if (!header.isValid()) {
            LOG_WARNING("[Server] Invalid RTGP header from "
                        << endpoint << " (magic: 0x" << std::hex
                        << static_cast<int>(header.magic) << ")");
            return std::nullopt;
        }

        size_t expectedSize = rtype::network::kHeaderSize + header.payloadSize;
        if (rawData.size() < expectedSize) {
            LOG_WARNING("[Server] Incomplete packet from "
                        << endpoint << " (expected: " << expectedSize
                        << ", got: " << rawData.size() << ")");
            return std::nullopt;
        }

        if (header.payloadSize > 0) {
            auto validationResult = rtype::network::Validator::validatePacket(
                rawData.data(), rawData.size(), header.isFromServer());
            if (validationResult.isErr()) {
                LOG_WARNING("[Server] Invalid packet from "
                            << endpoint << " (validation error code: "
                            << static_cast<int>(validationResult.error())
                            << ")");
                return std::nullopt;
            }
        }

        // TODO(Anybody): Eventually migrate to full RTGP packet handling
        rtype::network::Packet packet(
            static_cast<rtype::network::PacketType>(header.opcode));
        if (header.payloadSize > 0) {
            std::vector<uint8_t> payload(
                rawData.begin() + rtype::network::kHeaderSize, rawData.end());
            packet.setData(payload);
        }

        LOG_DEBUG("[Server] Successfully extracted packet from "
                  << endpoint << " (opcode: " << static_cast<int>(header.opcode)
                  << ", payload: " << header.payloadSize << " bytes)");
        return packet;
    } catch (const std::exception& e) {
        LOG_ERROR("[Server] Exception extracting packet from "
                  << endpoint << ": " << e.what());
        return std::nullopt;
    }
}

void ServerApp::broadcastGameState() noexcept {
    // TODO(Clem): Send game state to all connected clients
    // This will serialize entity states and send them via the network
}

void ServerApp::processPacket(ClientId clientId,
                              const rtype::network::Packet& packet) noexcept {
    // TODO(Clem): Implement packet processing logic
    // This will handle different packet types (player input, etc.)
    LOG_DEBUG("[Server] Processing packet from client "
              << clientId << " of type " << static_cast<int>(packet.type()));
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
        // TODO(Clem): Implement actual network receiving when rtype_network is
        // ready For now, this is a stub that simulates receiving packets
        //
        // Pseudocode:
        // if (_socket.hasData()) {
        //     Endpoint sender;
        //     std::vector<uint8_t> rawData = _socket.receive(sender);
        //     _rawNetworkData.push({sender, std::move(rawData)});
        // }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    LOG_DEBUG("[Server] Network thread exiting");
}

}  // namespace rtype::server
