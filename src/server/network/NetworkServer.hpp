/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NetworkServer - High-level server networking API
*/

#ifndef SRC_SERVER_NETWORK_NETWORKSERVER_HPP_
#define SRC_SERVER_NETWORK_NETWORKSERVER_HPP_

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <asio.hpp>

#include "compression/Compressor.hpp"
#include "connection/ConnectionEvents.hpp"
#include "core/Types.hpp"
#include "protocol/Header.hpp"
#include "protocol/Payloads.hpp"
#include "protocol/SecurityContext.hpp"
#include "reliability/ReliableChannel.hpp"
#include "server/shared/BanManager.hpp"
#include "transport/AsioUdpSocket.hpp"
#include "transport/IoContext.hpp"

namespace rtype::server {

class ServerMetrics;

/**
 * @brief Configuration for NetworkServer
 */
struct NetworkServerConfig {
    std::chrono::milliseconds clientTimeout{10000};
    network::ReliableChannel::Config reliabilityConfig{};
    network::Compressor::Config compressionConfig{};
    bool enableCompression = true;

    bool enablePacketStats = false;

    std::string expectedLobbyCode{};
};

/**
 * @brief High-level server networking API
 *
 * Encapsulates all low-level networking details and provides a clean
 * interface for game server code. Game code does NOT touch raw sockets or
 * packets.
 *
 * Features:
 * - Multi-client connection management with RUDP reliability
 * - Thread-safe callback queuing for main thread processing
 * - Automatic user ID assignment
 * - Packet validation and security (anti-replay, anti-spoofing)
 * - Clean separation: Network knows nothing about game logic
 *
 * Usage:
 * @code
 * NetworkServer server;
 *
 * server.onClientConnected([](uint32_t userId) {
 *     std::cout << "Client " << userId << " connected" << std::endl;
 * });
 *
 * server.onClientInput([&](uint32_t userId, uint8_t input) {
 *     // Handle player input
 * });
 *
 * server.start(4242);
 *
 * // In game loop:
 * while (running) {
 *     server.poll();  // Process incoming packets and callbacks
 *
 *     // Broadcast game state
 *     server.spawnEntity(entityId, EntityType::Player, x, y);
 *     server.moveEntity(entityId, x, y, vx, vy);
 * }
 *
 * server.stop();
 * @endcode
 *
 * Thread-safety: Callbacks are queued and dispatched on the thread calling
 * poll(). The network I/O itself is handled via asio's polling model.
 */
class NetworkServer {
   public:
    using EntityType = network::EntityType;
    using GameState = network::GameState;
    using Config = NetworkServerConfig;

    /**
     * @brief Construct a new NetworkServer
     * @param config Optional configuration
     */
    explicit NetworkServer(const Config& config = Config{});

    /**
     * @brief Destructor - automatically stops the server
     */
    ~NetworkServer();

    // Non-copyable
    NetworkServer(const NetworkServer&) = delete;
    NetworkServer& operator=(const NetworkServer&) = delete;

    // Non-movable (due to internal state complexity)
    NetworkServer(NetworkServer&&) = delete;
    NetworkServer& operator=(NetworkServer&&) = delete;

    /**
     * @brief Start the server on the specified port
     *
     * Begins accepting client connections.
     *
     * @param port Port number to bind to (default: 4242)
     * @return true if started successfully, false on error
     */
    bool start(std::uint16_t port = network::kDefaultPort);

    /**
     * @brief Configure expected lobby code for validation
     * @param code 6-char lobby code that clients must send via C_JOIN_LOBBY
     */
    void setExpectedLobbyCode(const std::string& code) {
        config_.expectedLobbyCode = code;
    }

    /**
     * @brief Set the server metrics for tracking packet statistics
     * @param metrics Shared pointer to ServerMetrics
     */
    void setMetrics(std::shared_ptr<ServerMetrics> metrics) {
        _metrics = std::move(metrics);
    }

    /**
     * @brief Get the connected client count
     * @return Number of connected clients
     */
    [[nodiscard]] std::size_t getClientCount() const {
        std::lock_guard lock(clientsMutex_);
        return clients_.size();
    }

    /**
     * @brief Stop the server and disconnect all clients
     */
    void stop();

    /**
     * @brief Check if server is running
     * @return true if server is running and accepting connections
     */
    [[nodiscard]] bool isRunning() const noexcept;

    /**
     * @brief Get the port the server is listening on
     * @return Port number, 0 if not running
     */
    [[nodiscard]] std::uint16_t port() const noexcept;

    /**
     * @brief Spawn an entity on all connected clients
     *
     * Sent reliably - guaranteed delivery and ordering.
     *
     * @param id Entity ID (must be unique)
     * @param type Entity type (Player, Bydos, Missile, etc.)
     * @param subType Entity subtype (enemy variant, etc.)
     * @param x Initial X position
     * @param y Initial Y position
     */
    void spawnEntity(std::uint32_t id, EntityType type, std::uint8_t subType,
                     float x, float y);

    /**
     * @brief Update entity position/velocity on all clients
     *
     * Sent unreliably - latest state matters, old updates discarded.
     *
     * @param id Entity ID
     * @param x New X position
     * @param y New Y position
     * @param vx X velocity
     * @param vy Y velocity
     */
    void moveEntity(std::uint32_t id, float x, float y, float vx, float vy);

    /**
     * @brief Broadcast batched entity moves to all clients
     *
     * More efficient than individual moveEntity calls - reduces packet overhead
     * and enables LZ4 compression for larger batches.
     *
     * @param entities Vector of (entityId, x, y, vx, vy) tuples
     */
    void moveEntitiesBatch(
        const std::vector<
            std::tuple<std::uint32_t, float, float, float, float>>& entities);

    /**
     * @brief Destroy an entity on all clients
     *
     * Sent reliably - guaranteed delivery.
     *
     * @param id Entity ID to destroy
     */
    void destroyEntity(std::uint32_t id);

    /**
     * @brief Broadcast entity health/lives to all clients
     * @param id Entity ID
     * @param current Current health value
     * @param max Maximum health value
     */
    void updateEntityHealth(std::uint32_t id, std::int32_t current,
                            std::int32_t max);

    /**
     * @brief Notify clients that a player picked a power-up
     * @param playerId Target player network id
     * @param powerUpType Encoded power-up type
     * @param duration Remaining duration (seconds)
     */
    void broadcastPowerUp(std::uint32_t playerId, std::uint8_t powerUpType,
                          float duration);

    /**
     * @brief Broadcast a chat message to all clients
     * @param senderId The sender's user ID (0 for system)
     * @param message  The text message content
     */
    void broadcastChat(std::uint32_t senderId, const std::string& message);

    /**
     * @brief Update game state on all clients
     *
     * Sent reliably - guaranteed delivery.
     *
     * @param state New game state (Lobby, Running, Paused, GameOver)
     */
    void updateGameState(GameState state);

    /**
     * @brief Broadcast game over with final score
     *
     * Sent reliably to all clients so they can display the end-game screen.
     *
     * @param finalScore Final aggregated score for the session
     */
    void sendGameOver(std::uint32_t finalScore);

    /**
     * @brief Broadcast game start with countdown
     *
     * Sent reliably to all clients to trigger countdown timer.
     *
     * @param countdownDuration Countdown duration in seconds (e.g., 3.0f)
     */
    void broadcastGameStart(float countdownDuration);

    /**
     * @brief Broadcast player ready state change to all clients
     *
     * Sent reliably to all clients to update lobby UI with ready status.
     *
     * @param userId User ID of the player whose ready state changed
     * @param isReady True if player is ready, false otherwise
     */
    void broadcastPlayerReadyState(std::uint32_t userId, bool isReady);

    /**
     * @brief Spawn an entity on a specific client
     *
     * Useful for syncing existing entities to newly connected clients.
     *
     * @param userId Target client's user ID
     * @param id Entity ID
     * @param type Entity type
     * @param subType Entity subtype (enemy variant, etc.)
     * @param x Initial X position
     * @param y Initial Y position
     */
    void spawnEntityToClient(std::uint32_t userId, std::uint32_t id,
                             EntityType type, std::uint8_t subType, float x,
                             float y);

    /**
     * @brief Update entity position/velocity on a specific client
     *
     * @param userId Target client's user ID
     * @param id Entity ID
     * @param x New X position
     * @param y New Y position
     * @param vx X velocity
     * @param vy Y velocity
     */
    void moveEntityToClient(std::uint32_t userId, std::uint32_t id, float x,
                            float y, float vx, float vy);

    /**
     * @brief Destroy an entity on a specific client
     *
     * @param userId Target client's user ID
     * @param id Entity ID to destroy
     */
    void destroyEntityToClient(std::uint32_t userId, std::uint32_t id);

    /**
     * @brief Send health/lives update to a specific client
     * @param userId Target client's user ID
     * @param id Entity ID
     * @param current Current health value
     * @param max Maximum health value
     */
    void updateEntityHealthToClient(std::uint32_t userId, std::uint32_t id,
                                    std::int32_t current, std::int32_t max);

    void sendPowerUpToClient(std::uint32_t userId, std::uint32_t playerId,
                             std::uint8_t powerUpType, float duration);

    /**
     * @brief Update game state on a specific client
     *
     * @param userId Target client's user ID
     * @param state New game state
     */
    void updateGameStateToClient(std::uint32_t userId, GameState state);

    /**
     * @brief Send position correction to a specific client
     *
     * Used for server-authoritative position reconciliation.
     * Sent unreliably - represents current authoritative state.
     *
     * @param userId Target client's user ID
     * @param x Corrected X position
     * @param y Corrected Y position
     */
    void correctPosition(std::uint32_t userId, float x, float y);

    /**
     * @brief Send a user list response to a specific client
     *
     * @param userId Target client's user ID
     * @param userIds List of connected user IDs
     */
    void sendUserList(std::uint32_t userId,
                      const std::vector<std::uint32_t>& userIds);

    /**
     * @brief Register callback for client connection
     * @param callback Function receiving the new client's user ID
     */
    void onClientConnected(std::function<void(std::uint32_t userId)> callback);

    /**
     * @brief Register callback for client disconnection
     * @param callback Function receiving the disconnected client's user ID and
     * reason
     */
    void onClientDisconnected(
        std::function<void(std::uint32_t userId,
                           network::DisconnectReason reason)>
            callback);

    /**
     * @brief Register callback for client input
     * @param callback Function receiving (userId, inputMask)
     */
    void onClientInput(
        std::function<void(std::uint32_t userId, std::uint8_t input)> callback);

    /**
     * @brief Register callback for get users request
     * @param callback Function receiving the requesting user ID
     */
    void onGetUsersRequest(std::function<void(std::uint32_t userId)> callback);

    /**
     * @brief Register callback for client ready/not ready signals
     * @param callback Function receiving user ID and ready state (true=ready)
     */
    void onClientReady(
        std::function<void(std::uint32_t userId, bool isReady)> callback);

    /**
     * @brief Register callback for client chat messages
     */
    void onClientChat(
        std::function<void(std::uint32_t, const std::string&)> callback);

    /**
     * @brief Process incoming packets and dispatch callbacks
     *
     * Must be called regularly (e.g., each game frame) to:
     * - Receive and process client packets
     * - Handle connection timeouts and retransmissions
     * - Dispatch queued callbacks to registered handlers
     *
     * Callbacks are executed on the calling thread.
     */
    void poll();

    /**
     * @brief Get list of connected client user IDs
     * @return Vector of connected user IDs
     */
    [[nodiscard]] std::vector<std::uint32_t> getConnectedClients() const;

    /**
     * @brief Get the number of connected clients
     * @return Number of currently connected clients
     */
    [[nodiscard]] std::size_t clientCount() const noexcept;

    /**
     * @brief Get the network endpoint for a connected user ID
     * @param userId Network user ID assigned to the client
     * @return Optional endpoint if the client exists
     */
    [[nodiscard]] std::optional<network::Endpoint> getClientEndpoint(
        std::uint32_t userId) const;

    /**
     * @brief Disconnect a client by user ID
     * Sends a DISCONNECT packet and removes the client from server maps.
     * @param userId Target client's user ID
     * @param reason Optional reason (default: RemoteRequest)
     * @return true if client was found and disconnected, false otherwise
     */
    bool disconnectClient(std::uint32_t userId,
                          network::DisconnectReason reason =
                              network::DisconnectReason::RemoteRequest);

    /**
     * @brief Check if a client is in low bandwidth mode
     * @param userId Client's user ID
     * @return true if client requested low bandwidth mode
     */
    [[nodiscard]] bool isLowBandwidthMode(std::uint32_t userId) const;

    /**
     * @brief Set bandwidth mode for a client
     * @param userId Client's user ID
     * @param lowBandwidth true for low bandwidth mode
     */
    void setClientBandwidthMode(std::uint32_t userId, bool lowBandwidth);

    /**
     * @brief Callback for client bandwidth mode changes
     */
    using BandwidthModeCallback =
        std::function<void(std::uint32_t userId, bool lowBandwidth)>;
    void onBandwidthModeChanged(BandwidthModeCallback callback);

   private:
    /**
     * @brief Client connection state
     */
    struct ClientConnection {
        network::Endpoint endpoint;
        std::uint32_t userId;
        network::ReliableChannel reliableChannel;
        std::chrono::steady_clock::time_point lastActivity;
        std::uint16_t nextSeqId{0};
        bool joined{false};
        bool lowBandwidthMode{false};

        explicit ClientConnection(const network::Endpoint& ep, std::uint32_t id,
                                  const network::ReliableChannel::Config& cfg)
            : endpoint(ep),
              userId(id),
              reliableChannel(cfg),
              lastActivity(std::chrono::steady_clock::now()) {}
    };

    void dispatchCallbacks();
    void queueCallback(std::function<void()> callback);

    void startReceive();
    void handleReceive(network::Result<std::size_t> result);
    void processIncomingPacket(const network::Buffer& data,
                               const network::Endpoint& sender);

    void handleConnect(const network::Header& header,
                       const network::Buffer& payload,
                       const network::Endpoint& sender);
    void handleDisconnect(const network::Header& header,
                          const network::Endpoint& sender);
    void handleInput(const network::Header& header,
                     const network::Buffer& payload,
                     const network::Endpoint& sender);
    void handleGetUsers(const network::Header& header,
                        const network::Endpoint& sender);
    void handlePing(const network::Header& header,
                    const network::Endpoint& sender);
    void handleReady(const network::Header& header,
                     const network::Buffer& payload,
                     const network::Endpoint& sender);
    void handleChat(const network::Header& header,
                    const network::Buffer& payload,
                    const network::Endpoint& sender);

    void handleJoinLobby(const network::Header& header,
                         const network::Buffer& payload,
                         const network::Endpoint& sender);
    void handleBandwidthMode(const network::Header& header,
                             const network::Buffer& payload,
                             const network::Endpoint& sender);

    [[nodiscard]] std::string makeConnectionKey(
        const network::Endpoint& ep) const;
    [[nodiscard]] std::shared_ptr<ClientConnection> findClient(
        const network::Endpoint& ep);
    [[nodiscard]] std::shared_ptr<ClientConnection> findClientByUserId(
        std::uint32_t userId);
    void removeClient(std::uint32_t userId);
    void checkTimeouts();

    static constexpr float kPosQuantScale = 16.0f;
    static constexpr float kVelQuantScale = 16.0f;

    static std::int16_t quantize(float value, float scale) noexcept {
        float scaled = value * scale;
        std::int64_t rounded = std::llround(scaled);
        if (rounded > std::numeric_limits<std::int16_t>::max()) {
            return std::numeric_limits<std::int16_t>::max();
        }
        if (rounded < std::numeric_limits<std::int16_t>::min()) {
            return std::numeric_limits<std::int16_t>::min();
        }
        return static_cast<std::int16_t>(rounded);
    }

    [[nodiscard]] std::uint32_t nextServerTick() noexcept {
        return serverTickCounter_.fetch_add(1, std::memory_order_acq_rel) + 1;
    }

    [[nodiscard]] network::Buffer buildPacket(network::OpCode opcode,
                                              const network::Buffer& payload,
                                              std::uint32_t userId,
                                              std::uint16_t seqId,
                                              std::uint16_t ackId,
                                              bool reliable);
    void sendToClient(const std::shared_ptr<ClientConnection>& client,
                      network::OpCode opcode, const network::Buffer& payload);
    void broadcastToAll(network::OpCode opcode, const network::Buffer& payload);
    [[nodiscard]] std::uint32_t nextUserId();

    Config config_;
    network::Compressor compressor_;

    bool running_{false};

    network::IoContext ioContext_;

    std::unique_ptr<network::IAsyncSocket> socket_;

    network::SecurityContext securityContext_;

    std::unordered_map<std::string, std::shared_ptr<ClientConnection>> clients_;

    std::unordered_map<std::uint32_t, std::string> userIdToKey_;

    std::vector<std::uint32_t> freeUserIds_;

    std::uint32_t nextUserIdCounter_{1};

    std::atomic<std::uint32_t> serverTickCounter_{0};

    std::shared_ptr<network::Buffer> receiveBuffer_;
    std::shared_ptr<network::Endpoint> receiveSender_;
    std::atomic<bool> receiveInProgress_{false};

    mutable std::mutex callbackMutex_;
    std::queue<std::function<void()>> callbackQueue_;

    std::function<void(std::uint32_t)> onClientConnectedCallback_;
    std::function<void(std::uint32_t, network::DisconnectReason)>
        onClientDisconnectedCallback_;
    std::function<void(std::uint32_t, std::uint8_t)> onClientInputCallback_;
    std::function<void(std::uint32_t)> onGetUsersRequestCallback_;
    std::function<void(std::uint32_t, bool)> onClientReadyCallback_;
    std::function<void(std::uint32_t, const std::string&)>
        onClientChatCallback_;
    std::function<void(std::uint32_t, bool)> onBandwidthModeChangedCallback_;

    mutable std::mutex clientsMutex_;

    std::shared_ptr<ServerMetrics> _metrics{nullptr};

    std::weak_ptr<BanManager> banManager_;

    struct PacketStats {
        std::uint64_t count{0};
        std::uint64_t totalBytes{0};

        double getAvgSize() const {
            return count > 0 ? static_cast<double>(totalBytes) / count : 0.0;
        }
    };

    std::unordered_map<std::uint8_t, PacketStats>
        sentPackets_;  // opcode -> stats
    std::unordered_map<std::uint8_t, PacketStats> receivedPackets_;
    mutable std::mutex statsMutex_;

   public:
    void setBanManager(std::shared_ptr<BanManager> bm) { banManager_ = bm; }

    void printPacketStatistics() const;
    void recordPacketSent(std::uint8_t opcode, std::size_t bytes);
    void recordPacketReceived(std::uint8_t opcode, std::size_t bytes);
};

}  // namespace rtype::server

#endif  // SRC_SERVER_NETWORK_NETWORKSERVER_HPP_
