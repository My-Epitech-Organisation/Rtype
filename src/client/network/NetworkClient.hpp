/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NetworkClient - High-level client networking API
*/

#ifndef SRC_CLIENT_NETWORK_NETWORKCLIENT_HPP_
#define SRC_CLIENT_NETWORK_NETWORKCLIENT_HPP_

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <vector>

#include <asio.hpp>

#include "compression/Compressor.hpp"
#include "connection/Connection.hpp"
#include "connection/ConnectionEvents.hpp"
#include "core/Types.hpp"
#include "protocol/Header.hpp"
#include "protocol/Payloads.hpp"
#include "transport/AsioUdpSocket.hpp"
#include "transport/IoContext.hpp"

namespace rtype::client {

/**
 * @brief Event data for entity spawn notification
 */
struct EntitySpawnEvent {
    std::uint32_t entityId;
    network::EntityType type;
    std::uint8_t subType{0};
    float x;
    float y;
    std::uint32_t userId = 0;
};

/**
 * @brief Event data for entity movement notification
 */
struct EntityMoveEvent {
    std::uint32_t entityId;
    std::uint32_t serverTick;
    float x;
    float y;
    float vx;
    float vy;
};

/**
 * @brief Event data for batched entity movement notifications
 */
struct EntityMoveBatchEvent {
    std::vector<EntityMoveEvent> entities;
};

/**
 * @brief Event data for entity health updates
 */
struct EntityHealthEvent {
    std::uint32_t entityId;
    std::int32_t current;
    std::int32_t max;
};

struct PowerUpEvent {
    std::uint32_t playerId;
    std::uint8_t powerUpType;
    float duration;
};

/**
 * @brief Event data for lobby information
 */
struct LobbyInfo {
    std::string code;
    std::uint16_t port;
    std::uint8_t playerCount;
    std::uint8_t maxPlayers;
    bool isActive;
};

/**
 * @brief Event data for lobby list response
 */
struct LobbyListEvent {
    std::vector<LobbyInfo> lobbies;
};

/**
 * @brief Event data for game state change
 */
struct GameStateEvent {
    network::GameState state;
};

/**
 * @brief Event data for game over notification
 */
struct GameOverEvent {
    std::uint32_t finalScore;
};

/**
 * @brief High-level client networking API
 *
 * Encapsulates all low-level networking details and provides a clean
 * interface for game code. Game code does NOT touch raw sockets or packets.
 *
 * Features:
 * - Automatic connection management with RUDP reliability
 * - Thread-safe callback queuing for main thread processing
 * - Clean separation: Network knows nothing about game logic
 *
 * Usage:
 * @code
 * NetworkClient client;
 *
 * client.onConnected([](uint32_t myUserId) {
 *     std::cout << "Connected with ID: " << myUserId << std::endl;
 * });
 *
 * client.onEntitySpawn([](EntitySpawnEvent event) {
 *     // Spawn entity in your game world
 * });
 *
 * client.connect("127.0.0.1", 4242);
 *
 * // In game loop:
 * while (running) {
 *     client.poll();  // Process callbacks and maintain connection
 *     client.sendInput(InputMask::kUp | InputMask::kShoot);
 * }
 *
 * client.disconnect();
 * @endcode
 *
 * Thread-safety: Callbacks are queued and dispatched on the thread calling
 * poll(). Network I/O is handled by a dedicated background thread.
 */

/**
 * @brief Sleep duration for network thread polling loop (in milliseconds)
 */
static constexpr std::chrono::milliseconds kNetworkThreadSleepDuration{3};

class NetworkClient {
   public:
    using DisconnectReason = network::DisconnectReason;
    using CallbackId = std::size_t;

    /**
     * @brief Configuration for NetworkClient
     */
    struct Config {
        network::Connection::Config connectionConfig;

        Config() = default;
    };

    /**
     * @brief Construct a new NetworkClient
     * @param config Optional configuration
     */
    explicit NetworkClient(const Config& config = Config{});

    /**
     * @brief Testable constructor for injecting a mock socket and optionally
     *        disabling the background network thread. Useful for unit tests.
     */
    NetworkClient(const Config& config,
                  std::unique_ptr<network::IAsyncSocket> socket,
                  bool startNetworkThread = true);

    /**
     * @brief Destructor - automatically disconnects if connected
     */
    ~NetworkClient();

    // Non-copyable
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    // Non-movable (due to internal state complexity)
    NetworkClient(NetworkClient&&) = delete;
    NetworkClient& operator=(NetworkClient&&) = delete;

    /**
     * @brief Initiate connection to a server
     *
     * Non-blocking. Connection result will be delivered via onConnected
     * or onDisconnected callbacks.
     *
     * @param host Server hostname or IP address
     * @param port Server port number
     * @return true if connection attempt started, false if already connected
     */
    bool connect(const std::string& host, std::uint16_t port);

    /**
     * @brief Gracefully disconnect from server
     *
     * Sends disconnect packet and waits for acknowledgement.
     * onDisconnected callback will be called with
     * DisconnectReason::LocalRequest
     */
    void disconnect();

    /**
     * @brief Check if currently connected to server
     * @return true if connected and ready to send/receive game data
     */
    [[nodiscard]] bool isConnected() const noexcept;

    /**
     * @brief Get the assigned user ID (only valid when connected)
     * @return User ID if connected, std::nullopt otherwise
     */
    [[nodiscard]] std::optional<std::uint32_t> userId() const noexcept;

    /**
     * @brief Get the current network latency (RTT) in milliseconds
     * @return Current latency, or 0 if no PONG received yet
     */
    [[nodiscard]] std::uint32_t latencyMs() const noexcept;

    /**
     * @brief Send player input to server
     *
     * Input is sent unreliably (latest state matters, old inputs discarded).
     * Use InputMask flags combined with bitwise OR.
     *
     * @param inputMask Combined input flags (e.g., InputMask::kUp |
     * InputMask::kShoot)
     * @return true if sent, false if not connected
     */
    bool sendInput(std::uint8_t inputMask);

    /**
     * @brief Send a chat message to the lobby
     *
     * @param message Text message to broadcast
     * @return true if sent, false if msg too long or not connected
     */
    bool sendChat(const std::string& message);

    /**
     * @brief Send a ping to the server
     *
     * Used for latency calculation.
     * @return true if sent, false if not connected
     */
    bool ping();

    /**
     * @brief Send ready/not ready state to server
     *
     * Sends C_READY packet to signal lobby ready state.
     * @param isReady true if ready, false if not ready
     * @return true if sent, false if not connected
     */
    bool sendReady(bool isReady);

    /**
     * @brief Request low bandwidth mode from server
     *
     * Sends C_SET_BANDWIDTH_MODE packet. When enabled, server will send
     * entity updates at reduced rates to save bandwidth.
     * @param enable true for low bandwidth mode, false for normal
     * @return true if sent, false if not connected
     */
    bool setLowBandwidthMode(bool enable);

    /**
     * @brief Request lobby list from discovery server
     *
     * Sends C_REQUEST_LOBBIES to the server. Response will be delivered
     * via onLobbyListReceived callback.
     *
     * Note: Should be sent to discovery server port (typically base port),
     * not a specific lobby port.
     *
     * @param discoveryIp IP address of discovery server (default: 127.0.0.1)
     * @param discoveryPort Port of discovery server (default: 4242)
     * @return true if sent successfully
     */
    bool requestLobbyList(const std::string& discoveryIp = "127.0.0.1",
                          std::uint16_t discoveryPort = 4242);

    /**
     * @brief Register callback for successful connection
     * @param callback Function receiving the assigned user ID
     */
    void onConnected(std::function<void(std::uint32_t myUserId)> callback);
    CallbackId addConnectedCallback(
        std::function<void(std::uint32_t myUserId)> callback);
    void removeConnectedCallback(CallbackId id);

    /**
     * @brief Register callback for disconnection (graceful or unexpected)
     * @param callback Function receiving the disconnect reason
     */
    void onDisconnected(std::function<void(DisconnectReason)> callback);
    CallbackId addDisconnectedCallback(
        std::function<void(DisconnectReason)> callback);
    void removeDisconnectedCallback(CallbackId id);

    /**
     * @brief Clear all disconnect callbacks (call from scene destructors)
     */
    void clearDisconnectedCallbacks();

    /**
     * @brief Register callback for entity spawn events
     * @param callback Function receiving spawn event data
     */
    void onEntitySpawn(std::function<void(EntitySpawnEvent)> callback);

    /**
     * @brief Register callback for entity movement updates
     * @param callback Function receiving movement event data
     */
    void onEntityMove(std::function<void(EntityMoveEvent)> callback);

    /**
     * @brief Register callback for batched entity movement updates
     * @param callback Function receiving batch event data
     */
    void onEntityMoveBatch(std::function<void(EntityMoveBatchEvent)> callback);

    /**
     * @brief Register callback for entity destruction (convenience wrapper)
     * @param callback Function receiving the destroyed entity ID
     * @note Prefer using addEntityDestroyCallback/removeEntityDestroyCallback
     * to manage lifetime explicitly.
     */
    void onEntityDestroy(std::function<void(std::uint32_t entityId)> callback);

    /**
     * @brief Add entity destroy callback and return identifier for removal
     * @param callback Function receiving the destroyed entity ID
     */
    CallbackId addEntityDestroyCallback(
        std::function<void(std::uint32_t entityId)> callback);

    /**
     * @brief Remove previously added entity destroy callback
     * @param id Identifier returned by addEntityDestroyCallback
     */
    void removeEntityDestroyCallback(CallbackId id);

    /**
     * @brief Clear all entity-destroy callbacks
     */
    void clearEntityDestroyCallbacks();

    /**
     * @brief Register callback for entity health updates
     * @param callback Function receiving the health event data
     */
    void onEntityHealth(std::function<void(EntityHealthEvent)> callback);

    /**
     * @brief Register callback for power-up events
     */
    void onPowerUpEvent(std::function<void(PowerUpEvent)> callback);

    /**
     * @brief Register callback for server position correction
     *
     * Called when server's authoritative position differs from client
     * prediction. Use to snap or interpolate player position.
     *
     * @param callback Function receiving corrected x,y position
     */
    void onPositionCorrection(std::function<void(float x, float y)> callback);

    /**
     * @brief Register callback for game state changes
     * @param callback Function receiving the new game state
     */
    void onGameStateChange(std::function<void(GameStateEvent)> callback);
    void onGameOver(std::function<void(GameOverEvent)> callback);
    void onChatReceived(
        std::function<void(std::uint32_t, std::string)> callback);

    /**
     * @brief Register callback for game start countdown
     * @param callback Function receiving countdown duration in seconds
     */
    void onGameStart(std::function<void(float countdownDuration)> callback);

    /**
     * @brief Register callback for player ready state changes
     * @param callback Function receiving userId and ready status
     */
    void onPlayerReadyStateChanged(
        std::function<void(std::uint32_t userId, bool isReady)> callback);

    /**
     * @brief Register callback for bandwidth mode changes broadcast by server
     * @param callback Function receiving userId, lowBandwidthEnabled, and
     * activeCount
     */
    void onBandwidthModeChanged(
        std::function<void(std::uint32_t userId, bool lowBandwidth,
                           std::uint8_t activeCount)>
            callback);

    /**
     * @brief Register callback for lobby list response
     * @param callback Function receiving the list of available lobbies
     */
    void onLobbyListReceived(std::function<void(LobbyListEvent)> callback);

    /**
     * @brief Send the lobby join code to the server (must be used after
     * connect)
     * @param code 6-character lobby code
     */
    bool sendJoinLobby(const std::string& code);

    /**
     * @brief Register callback for join lobby response
     * @param callback Function receiving (accepted, reason)
     */
    void onJoinLobbyResponse(std::function<void(bool, uint8_t)> callback);

    /**
     * @brief Process network events and dispatch callbacks
     *
     * Must be called regularly (e.g., each game frame) to:
     * - Update connection state and handle timeouts/retransmissions
     * - Send queued outgoing packets
     * - Dispatch queued callbacks to registered handlers
     *
     * Note: I/O polling is handled by a dedicated network thread.
     * This method only handles connection maintenance and callback dispatch.
     *
     * Callbacks are executed on the calling thread.
     */
    void poll();

    /**
     * @brief Clear all pending callbacks in the queue
     * @note Call before scene destruction to prevent stale callbacks
     */
    void clearPendingCallbacks();

    // Test helpers (use from unit tests only)
    void test_dispatchCallbacks();
    void test_processIncomingPacket(const network::Buffer& data,
                                    const network::Endpoint& sender);
    void test_queueCallback(std::function<void()> callback);
    void test_startReceive();
    void test_handlePong(const network::Header& header,
                         const network::Buffer& payload);

   private:
    void dispatchCallbacks();
    void queueCallback(std::function<void()> callback);

    void networkThreadLoop();

    void startReceive();
    void handleReceive(network::Result<std::size_t> result);
    void processIncomingPacket(const network::Buffer& data,
                               const network::Endpoint& sender);
    void handleEntitySpawn(const network::Header& header,
                           const network::Buffer& payload);
    void handleEntityMove(const network::Header& header,
                          const network::Buffer& payload);
    void handleEntityMoveBatch(const network::Header& header,
                               const network::Buffer& payload);
    void handleEntityDestroy(const network::Header& header,
                             const network::Buffer& payload);
    void handleEntityHealth(const network::Header& header,
                            const network::Buffer& payload);
    void handlePowerUpEvent(const network::Header& header,
                            const network::Buffer& payload);
    void handleUpdatePos(const network::Header& header,
                         const network::Buffer& payload);
    void handleUpdateState(const network::Header& header,
                           const network::Buffer& payload);
    void handleGameOver(const network::Header& header,
                        const network::Buffer& payload);
    void handleGameStart(const network::Header& header,
                         const network::Buffer& payload);
    void handlePlayerReadyState(const network::Header& header,
                                const network::Buffer& payload);
    void handleBandwidthModeChanged(const network::Header& header,
                                    const network::Buffer& payload);
    void handleLobbyList(const network::Header& header,
                         const network::Buffer& payload);
    void handleJoinLobbyResponse(const network::Header& header,
                                 const network::Buffer& payload);
    void handleChat(const network::Header& header,
                    const network::Buffer& payload);
    void handlePong(const network::Header& header,
                    const network::Buffer& payload);

    void flushOutgoing();

    /**
     * @brief Send an ACK packet immediately to acknowledge reliable messages
     *
     * This is called right after receiving a reliable message to ensure
     * the server receives acknowledgment promptly, preventing retry timeouts.
     * @param ackSeqId The specific sequence ID to acknowledge
     */
    void sendAck(std::uint16_t ackSeqId);

    Config config_;

    network::Compressor compressor_;

    network::IoContext ioContext_;

    std::unique_ptr<network::IAsyncSocket> socket_;

    network::Connection connection_;

    std::optional<network::Endpoint> serverEndpoint_;

    std::shared_ptr<network::Buffer> receiveBuffer_;
    std::shared_ptr<network::Endpoint> receiveSender_;
    std::atomic<bool> receiveInProgress_{false};

    std::mutex callbackMutex_;
    std::queue<std::function<void()>> callbackQueue_;

    std::vector<std::function<void(std::uint32_t)>> onConnectedCallbacks_;
    std::vector<std::function<void(DisconnectReason)>> onDisconnectedCallbacks_;
    std::function<void(EntitySpawnEvent)> onEntitySpawnCallback_;
    std::function<void(EntityMoveEvent)> onEntityMoveCallback_;
    std::function<void(EntityMoveBatchEvent)> onEntityMoveBatchCallback_;
    std::vector<std::function<void(std::uint32_t)>> onEntityDestroyCallbacks_;
    std::function<void(EntityHealthEvent)> onEntityHealthCallback_;
    std::function<void(float, float)> onPositionCorrectionCallback_;
    std::function<void(GameStateEvent)> onGameStateChangeCallback_;
    std::function<void(GameOverEvent)> onGameOverCallback_;
    std::function<void(std::uint32_t, std::string)> onChatReceivedCallback_;
    std::function<void(float)> onGameStartCallback_;
    std::function<void(std::uint32_t, bool)> onPlayerReadyStateChangedCallback_;
    std::function<void(PowerUpEvent)> onPowerUpCallback_;
    std::function<void(LobbyListEvent)> onLobbyListReceivedCallback_;
    std::function<void(bool, uint8_t)> onJoinLobbyResponseCallback_;
    std::function<void(std::uint32_t, bool, std::uint8_t)>
        onBandwidthModeChangedCallback_;

    std::thread networkThread_;
    std::atomic<bool> networkThreadRunning_{false};
};

}  // namespace rtype::client

#endif  // SRC_CLIENT_NETWORK_NETWORKCLIENT_HPP_
