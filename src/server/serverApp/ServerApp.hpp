/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerApp - Main server application (refactored)
*/

#ifndef SRC_SERVER_SERVERAPP_SERVERAPP_HPP_
#define SRC_SERVER_SERVERAPP_SERVERAPP_HPP_

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

#include <rtype/common.hpp>
#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>
#include <rtype/network.hpp>

#include "server/clientManager/ClientManager.hpp"
#include "server/network/NetworkServer.hpp"
#include "server/network/ServerNetworkSystem.hpp"
#include "server/serverApp/ServerLoop.hpp"
#include "server/serverApp/game/gameEvent/GameEventProcessor.hpp"
#include "server/serverApp/game/gameStateManager/GameStateManager.hpp"
#include "server/serverApp/packetProcessor/PacketProcessor.hpp"
#include "server/serverApp/player/playerInputHandler/PlayerInputHandler.hpp"
#include "server/shared/AdminServer.hpp"
#include "server/shared/BanManager.hpp"
#include "server/shared/Client.hpp"
#include "server/shared/IEntitySpawner.hpp"
#include "server/shared/IGameConfig.hpp"
#include "server/shared/ServerMetrics.hpp"

namespace rtype::server {

class LobbyManager;

using rtype::ClientId;
using rtype::Endpoint;

/**
 * @brief Main server application class (refactored)
 *
 * Composes specialized components:
 * - GameStateManager: Game state transitions
 * - PacketProcessor: Packet validation
 * - PlayerInputHandler: Input processing
 * - GameEventProcessor: Event routing
 * - IEntitySpawner: Entity spawning (game-specific)
 * - ServerLoop: Main loop timing
 */
class ServerApp {
   public:
    static constexpr uint32_t DEFAULT_CLIENT_TIMEOUT_SECONDS = 10;
    static constexpr size_t MIN_PLAYERS_TO_START = 1;

    /**
     * @brief Construct with manual configuration
     */
    explicit ServerApp(
        uint16_t port, size_t maxPlayers, uint32_t tickRate,
        std::shared_ptr<std::atomic<bool>> shutdownFlag,
        uint32_t clientTimeoutSeconds = DEFAULT_CLIENT_TIMEOUT_SECONDS,
        bool verbose = false, std::shared_ptr<BanManager> banManager = nullptr);

    /**
     * @brief Construct with game configuration
     */
    explicit ServerApp(std::unique_ptr<IGameConfig> gameConfig,
                       std::shared_ptr<std::atomic<bool>> shutdownFlag,
                       bool verbose = false,
                       std::shared_ptr<BanManager> banManager = nullptr);

    ~ServerApp();

    ServerApp(const ServerApp&) = delete;
    ServerApp& operator=(const ServerApp&) = delete;
    ServerApp(ServerApp&&) = delete;
    ServerApp& operator=(ServerApp&&) = delete;

    /**
     * @brief Start the server main loop (blocking)
     */
    [[nodiscard]] bool run();

    /**
     * @brief Signal the server to stop
     */
    void stop() noexcept;

    /**
     * @brief Set the level/map to load on initialization
     * @param levelId The level identifier
     */
    void setLevel(const std::string& levelId) { _initialLevel = levelId; }

    /**
     * @brief Change the current level (reloads if necessary)
     * @param levelId The level identifier
     * @param force Force level change even if game is running (for automatic transitions)
     */
    bool changeLevel(const std::string& levelId, bool force = false);

    /**
     * @brief Check if server is running
     */
    [[nodiscard]] bool isRunning() const noexcept;

    [[nodiscard]] size_t getConnectedClientCount() const noexcept;
    [[nodiscard]] std::vector<ClientId> getConnectedClientIds() const;
    [[nodiscard]] std::optional<Client> getClientInfo(ClientId clientId) const;
    /**
     * @brief Get the network endpoint for a connected client (authoritative via
     * NetworkServer)
     * @param clientId The client/user ID assigned by NetworkServer
     * @return Optional endpoint with address and port if found
     */
    [[nodiscard]] std::optional<rtype::Endpoint> getClientEndpoint(
        ClientId clientId) const;

    [[nodiscard]] const ServerMetrics& getMetrics() const noexcept {
        return *_metrics;
    }
    [[nodiscard]] ClientManager& getClientManager() noexcept {
        return _clientManager;
    }
    [[nodiscard]] const ClientManager& getClientManager() const noexcept {
        return _clientManager;
    }
    [[nodiscard]] GameState getGameState() const noexcept {
        return _stateManager->getState();
    }
    [[nodiscard]] bool isPlaying() const noexcept {
        return _stateManager->isPlaying();
    }
    [[nodiscard]] bool isCountdownActive() const noexcept {
        return _stateManager->isCountdownActive();
    }
    [[nodiscard]] float getCountdownRemaining() const noexcept {
        return _stateManager->getCountdownRemaining();
    }
    [[nodiscard]] size_t getReadyPlayerCount() const noexcept {
        return _stateManager->getReadyPlayerCount();
    }
    [[nodiscard]] IGameConfig* getGameConfig() noexcept {
        return _gameConfig.get();
    }
    [[nodiscard]] const IGameConfig* getGameConfig() const noexcept {
        return _gameConfig.get();
    }
    [[nodiscard]] bool hasGameConfig() const noexcept {
        return _gameConfig != nullptr && _gameConfig->isInitialized();
    }

    /**
     * @brief Get ban manager for admin operations
     */
    [[nodiscard]] BanManager& getBanManager() noexcept { return *_banManager; }
    [[nodiscard]] const BanManager& getBanManager() const noexcept {
        return *_banManager;
    }

    /**
     * @brief Kick a client by user ID
     * @return true if disconnected, false otherwise
     */
    bool kickClient(ClientId clientId) {
        if (_networkServer) {
            return _networkServer->disconnectClient(clientId);
        }
        return false;
    }

    /**
     * @brief Set LobbyManager reference for admin panel integration
     * @param lobbyManager Pointer to LobbyManager instance
     */
    void setLobbyManager(LobbyManager* lobbyManager);

    void playerReady(std::uint32_t userId) {
        _stateManager->playerReady(userId);
    }

    // Test helpers to manipulate game state for integration tests
    void playerNotReady(std::uint32_t userId) {
        _stateManager->playerNotReady(userId);
    }

    void forceStart() { _stateManager->forceStart(); }

    /**
     * @brief Test hook: register a callback to be invoked when ServerApp
     * broadcasts a game start (used in tests to capture broadcast events).
     */
    void setOnGameStartBroadcastCallback(std::function<void(float)> cb) {
        _onGameStartBroadcastCallback = std::move(cb);
    }

    /**
     * @brief Set the expected lobby code for join validation
     */
    void setLobbyCode(const std::string& code);

    /**
     * @brief Test hook: set default countdown duration used by GameStateManager
     * (useful to make tests deterministic)
     */
    void setDefaultCountdown(float seconds) {
        if (_stateManager) {
            _stateManager->setDefaultCountdown(seconds);
        }
    }

    [[nodiscard]] bool reloadConfiguration();

    void registerUserIdMapping(const Endpoint& endpoint,
                               std::uint32_t userId) noexcept {
        _packetProcessor.registerConnection(endpoint.toString(), userId);
    }

   private:
    [[nodiscard]] bool initialize();
    void shutdown() noexcept;
    void logStartupInfo() const noexcept;

    void onFrame();
    void onUpdate(float deltaTime);
    void onPostUpdate();

    void checkGameOverCondition();
    [[nodiscard]] std::size_t countAlivePlayers();
    void onGameEvent(const engine::GameEvent& event);

    void handleClientConnected(std::uint32_t userId);
    void handleClientDisconnected(std::uint32_t userId);
    void handleStateChange(GameState oldState, GameState newState);

    void resetToLobby();

    void processIncomingData() noexcept;
    void processRawNetworkData() noexcept;
    [[nodiscard]] bool startNetworkThread();
    void stopNetworkThread() noexcept;
    void networkThreadFunction() noexcept;

    void updatePlayerMovement(float deltaTime) noexcept;

    uint16_t _port;
    uint32_t _tickRate;
    uint32_t _clientTimeoutSeconds;
    bool _verbose;
    std::shared_ptr<std::atomic<bool>> _shutdownFlag;
    std::atomic<bool> _hasShutdown{false};

    std::shared_ptr<ServerMetrics> _metrics;
    std::shared_ptr<BanManager> _banManager;
    LobbyManager* _lobbyManager{nullptr};
    ClientManager _clientManager;
    std::shared_ptr<IGameConfig> _gameConfig;
    std::unique_ptr<AdminServer> _adminServer;

    std::shared_ptr<GameStateManager> _stateManager;
    PacketProcessor _packetProcessor;
    std::unique_ptr<PlayerInputHandler> _inputHandler;
    std::unique_ptr<GameEventProcessor> _eventProcessor;
    std::unique_ptr<IEntitySpawner> _entitySpawner;

    SafeQueue<std::pair<Endpoint, std::vector<uint8_t>>> _rawNetworkData;
    SafeQueue<std::pair<Endpoint, rtype::network::Packet>> _incomingPackets;
    std::thread _networkThread;
    std::atomic<bool> _networkThreadRunning{false};

    std::shared_ptr<engine::IGameEngine> _gameEngine;
    std::shared_ptr<NetworkServer> _networkServer;
    std::shared_ptr<ServerNetworkSystem> _networkSystem;
    std::shared_ptr<ECS::Registry> _registry;

    // Server loop (owned by run method, used for tick overrun tracking)
    ServerLoop* _serverLoop{nullptr};

    uint32_t _metricSnapshotCounter{0};
    static constexpr uint32_t METRICS_SNAPSHOT_INTERVAL = 60;  // in seconds

    // Test hooks
    std::function<void(float)> _onGameStartBroadcastCallback;

    std::uint32_t _score{0};
    static constexpr std::uint32_t ENEMY_DESTRUCTION_SCORE = 100;
    std::string _initialLevel;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_SERVERAPP_HPP_
