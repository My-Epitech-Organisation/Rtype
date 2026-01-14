/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerNetworkSystem - Bridges NetworkServer with ECS Registry
*/

#ifndef SRC_SERVER_NETWORK_SERVERNETWORKSYSTEM_HPP_
#define SRC_SERVER_NETWORK_SERVERNETWORKSYSTEM_HPP_

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include <rtype/ecs.hpp>

#include "NetworkServer.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::server {

/**
 * @brief Bridges NetworkServer with ECS Registry for automatic entity
 * replication
 *
 * This system handles:
 * - Broadcasting entity spawns to all clients
 * - Broadcasting entity movement updates
 * - Broadcasting entity destruction
 * - Tracking which entities are networked
 * - Processing client inputs and routing to game logic
 *
 * Usage:
 * @code
 * auto registry = std::make_shared<ECS::Registry>();
 * NetworkServer server;
 * ServerNetworkSystem networkSystem(registry, server);
 *
 * server.start(4242);
 *
 * // When spawning a networked entity:
 * auto entity = registry->spawnEntity();
 * networkSystem.registerNetworkedEntity(entity, networkId,
 *                                       NetworkServer::EntityType::Player,
 *                                       x, y);
 *
 * // In game loop:
 * while (running) {
 *     networkSystem.update();
 *
 *     // After updating entity positions:
 *     networkSystem.broadcastEntityUpdates();
 * }
 * @endcode
 *
 * Thread-safety: Must be called from the same thread as the game loop.
 */
class ServerNetworkSystem {
   public:
    using EntityType = NetworkServer::EntityType;

    /**
     * @brief Input handler function type
     *
     * Called when a client sends input. The handler should apply the input
     * to the appropriate entity in the game world.
     *
     * @param userId The client's user ID
     * @param inputMask The input flags
     * @param entity The client's player entity (if registered)
     */
    using InputHandler =
        std::function<void(std::uint32_t userId, std::uint16_t inputMask,
                           std::optional<ECS::Entity> entity)>;

    /**
     * @brief Construct a new ServerNetworkSystem
     *
     * @param registry Shared pointer to the ECS registry
     * @param server Shared pointer to the NetworkServer
     */
    ServerNetworkSystem(std::shared_ptr<ECS::Registry> registry,
                        std::shared_ptr<NetworkServer> server);

    /**
     * @brief Destructor
     */
    ~ServerNetworkSystem() = default;

    // Non-copyable
    ServerNetworkSystem(const ServerNetworkSystem&) = delete;
    ServerNetworkSystem& operator=(const ServerNetworkSystem&) = delete;

    // Non-movable
    ServerNetworkSystem(ServerNetworkSystem&&) = delete;
    ServerNetworkSystem& operator=(ServerNetworkSystem&&) = delete;

    /**
     * @brief Register an entity for network synchronization
     *
     * Broadcasts the entity spawn to all connected clients.
     * The entity will be tracked for position updates.
     *
     * @param entity The ECS entity
     * @param networkId Unique network ID for this entity
     * @param type Entity type (Player, Enemy, etc.)
     * @param x Initial X position
     * @param y Initial Y position
     */
    void registerNetworkedEntity(ECS::Entity entity, std::uint32_t networkId,
                                 EntityType type, float x, float y);

    /**
     * @brief Unregister an entity from network synchronization
     *
     * Broadcasts entity destruction to all clients.
     *
     * @param entity The ECS entity to unregister
     */
    void unregisterNetworkedEntity(ECS::Entity entity);

    /**
     * @brief Unregister an entity by network ID
     *
     * @param networkId The network ID of the entity
     */
    void unregisterNetworkedEntityById(std::uint32_t networkId);

    /**
     * @brief Associate a user ID with a player entity
     *
     * Used to route input to the correct entity.
     *
     * @param userId The client's user ID
     * @param entity The player's ECS entity
     */
    void setPlayerEntity(std::uint32_t userId, ECS::Entity entity);

    /**
     * @brief Get the entity associated with a user ID
     * @param userId The client's user ID
     * @return The player entity if registered
     */
    [[nodiscard]] std::optional<ECS::Entity> getPlayerEntity(
        std::uint32_t userId) const;

    /**
     * @brief Set the input handler for client inputs
     *
     * @param handler Function to handle client inputs
     */
    void setInputHandler(InputHandler handler);

    /**
     * @brief Register callback for client connection
     *
     * @param callback Function called when a client connects
     */
    void onClientConnected(std::function<void(std::uint32_t userId)> callback);

    /**
     * @brief Register callback for client disconnection
     *
     * @param callback Function called when a client disconnects
     */
    void onClientDisconnected(
        std::function<void(std::uint32_t userId)> callback);

    /**
     * @brief Update a networked entity's position
     *
     * Queues a position update to be broadcast to clients.
     *
     * @param networkId The entity's network ID
     * @param x New X position
     * @param y New Y position
     * @param vx X velocity
     * @param vy Y velocity
     */
    void updateEntityPosition(std::uint32_t networkId, float x, float y,
                              float vx, float vy);

    /**
     * @brief Broadcast entity health to all clients
     * @param networkId The entity's network ID
     * @param current Current health/lives value
     * @param max Maximum health/lives value
     */
    void updateEntityHealth(std::uint32_t networkId, std::int32_t current,
                            std::int32_t max);

    void broadcastPowerUp(std::uint32_t playerNetworkId,
                          std::uint8_t powerUpType, float duration);

    /**
     * @brief Send position correction to a specific player
     *
     * Used for server-authoritative reconciliation.
     *
     * @param userId Target client's user ID
     * @param x Corrected X position
     * @param y Corrected Y position
     */
    void correctPlayerPosition(std::uint32_t userId, float x, float y);

    /**
     * @brief Broadcast all pending entity updates
     *
     * Call this after updating entity positions in your game loop.
     */
    void broadcastEntityUpdates();

    /**
     * @brief Broadcast entity spawn to all connected clients
     *
     * Called when a new entity is spawned by the game engine.
     * This broadcasts directly without requiring an ECS entity.
     *
     * @param networkId The entity's network ID
     * @param type Entity type for rendering
     * @param subType Entity subtype (enemy variant, etc.)
     * @param x Initial X position
     * @param y Initial Y position
     */
    void broadcastEntitySpawn(std::uint32_t networkId, EntityType type,
                              std::uint8_t subType, float x, float y);

    /**
     * @brief Broadcast game start signal to all clients
     *
     * Called when the server transitions to Playing state.
     */
    void broadcastGameStart();

    /**
     * @brief Broadcast a game state update to all clients
     */
    void broadcastGameState(NetworkServer::GameState state);

    /**
     * @brief Broadcast final score when the game ends
     * @param finalScore The final game score
     * @param isVictory True if player won, false if defeated
     */
    void broadcastGameOver(std::uint32_t finalScore, bool isVictory = false);

    /**
     * @brief Reset all tracked network state
     *
     * Clears entity/user mappings and resets network id counter.
     * Use when returning to lobby between games.
     */
    void resetState();

    /**
     * @brief Update the network system
     *
     * Polls the network server and processes any pending events.
     * Should be called once per frame.
     */
    void update();

    /**
     * @brief Get the network ID for an entity
     * @param entity The ECS entity
     * @return Network ID if registered
     */
    [[nodiscard]] std::optional<std::uint32_t> getNetworkId(
        ECS::Entity entity) const;

    /**
     * @brief Find entity by network ID
     * @param networkId The network entity ID
     * @return Entity if found
     */
    [[nodiscard]] std::optional<ECS::Entity> findEntityByNetworkId(
        std::uint32_t networkId) const;

    /**
     * @brief Generate the next available network ID
     * @return Unique network ID
     */
    [[nodiscard]] std::uint32_t nextNetworkId();

   private:
    void handleClientConnected(std::uint32_t userId);
    void handleClientDisconnected(std::uint32_t userId,
                                  network::DisconnectReason reason);
    void processExpiredGracePeriods();
    void finalizeDisconnection(std::uint32_t userId);
    void handleClientInput(std::uint32_t userId, std::uint8_t inputMask);
    void handleClientChat(std::uint32_t userId, const std::string& message);
    void handleGetUsersRequest(std::uint32_t userId);

    struct NetworkedEntity {
        ECS::Entity entity;
        std::uint32_t networkId;
        EntityType type;
        float lastX{0};
        float lastY{0};
        float lastVx{0};
        float lastVy{0};
        bool dirty{false};
        float lastSentX{0};
        float lastSentY{0};
        float lastSentVx{0};
        float lastSentVy{0};
        std::uint32_t ticksSinceLastSend{0};
    };

    std::shared_ptr<ECS::Registry> registry_;
    std::shared_ptr<NetworkServer> server_;
    std::unordered_map<std::uint32_t, NetworkedEntity> networkedEntities_;
    std::unordered_map<std::uint64_t, std::uint32_t> entityToNetworkId_;
    std::unordered_map<std::uint32_t, ECS::Entity> userIdToEntity_;
    std::uint32_t nextNetworkIdCounter_{1};
    InputHandler inputHandler_;
    std::function<void(std::uint32_t)> onClientConnectedCallback_;
    std::function<void(std::uint32_t)> onClientDisconnectedCallback_;

    static constexpr std::chrono::milliseconds kDisconnectGracePeriod{5000};

    struct PendingDisconnection {
        std::chrono::steady_clock::time_point disconnectTime;
        ECS::Entity playerEntity;
        std::uint32_t networkId;
    };
    std::unordered_map<std::uint32_t, PendingDisconnection>
        pendingDisconnections_;

    std::atomic<bool> lowBandwidthModeActive_{false};
    std::atomic<std::uint32_t> lowBandwidthClientCount_{0};
};

}  // namespace rtype::server

#endif  // SRC_SERVER_NETWORK_SERVERNETWORKSYSTEM_HPP_
