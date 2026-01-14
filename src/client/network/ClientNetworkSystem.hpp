/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientNetworkSystem - Bridges NetworkClient with ECS Registry
*/

#ifndef SRC_CLIENT_NETWORK_CLIENTNETWORKSYSTEM_HPP_
#define SRC_CLIENT_NETWORK_CLIENTNETWORKSYSTEM_HPP_

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

#include <rtype/ecs.hpp>

#include "NetworkClient.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::client {

/**
 * @brief Bridges NetworkClient with ECS Registry for automatic entity
 * replication
 *
 * This system handles:
 * - Spawning entities when server sends S_ENTITY_SPAWN
 * - Updating entity positions when server sends S_ENTITY_MOVE
 * - Destroying entities when server sends S_ENTITY_DESTROY
 * - Correcting local player position on S_UPDATE_POS
 *
 * Usage:
 * @code
 * auto registry = std::make_shared<ECS::Registry>();
 * NetworkClient client;
 * ClientNetworkSystem networkSystem(registry, client);
 *
 * // Customize entity creation
 * networkSystem.setEntityFactory([](ECS::Registry& reg, EntitySpawnEvent e) {
 *     auto entity = reg.spawnEntity();
 *     reg.emplaceComponent<TransformComponent>(entity, e.x, e.y);
 *     reg.emplaceComponent<NetworkIdComponent>(entity, e.entityId);
 *     // Add sprites, etc.
 *     return entity;
 * });
 *
 * // In game loop:
 * while (running) {
 *     networkSystem.update();
 * }
 * @endcode
 *
 * Thread-safety: Must be called from the same thread as the game loop.
 */
class ClientNetworkSystem {
   public:
    /**
     * @brief Factory function type for creating entities from spawn events
     *
     * Called when server sends S_ENTITY_SPAWN. Should create the entity
     * with appropriate components (Transform, Sprite, etc.) based on type.
     *
     * @param registry The ECS registry
     * @param event The spawn event with entity data
     * @return The created entity
     */
    using EntityFactory =
        std::function<ECS::Entity(ECS::Registry&, const EntitySpawnEvent&)>;

    /**
     * @brief Construct a new ClientNetworkSystem
     *
     * @param registry Shared pointer to the ECS registry
     * @param client Shared pointer to the NetworkClient
     */
    ClientNetworkSystem(std::shared_ptr<ECS::Registry> registry,
                        std::shared_ptr<NetworkClient> client);

    /**
     * @brief Destructor - unregisters all callbacks from NetworkClient
     */
    ~ClientNetworkSystem();

    ClientNetworkSystem(const ClientNetworkSystem&) = delete;
    ClientNetworkSystem& operator=(const ClientNetworkSystem&) = delete;
    ClientNetworkSystem(ClientNetworkSystem&&) = delete;
    ClientNetworkSystem& operator=(ClientNetworkSystem&&) = delete;

    /**
     * @brief Set custom entity factory
     *
     * Allows customizing how entities are created from spawn events.
     * If not set, uses a default factory that creates entities with
     * basic Transform and NetworkId components.
     *
     * @param factory Custom factory function
     */
    void setEntityFactory(EntityFactory factory);

    /**
     * @brief Register callback for when local player is assigned
     *
     * Called when the client successfully connects and receives a user ID.
     * Use this to identify which entity is the local player.
     *
     * @param callback Function receiving (localUserId, localEntity)
     */
    void onLocalPlayerAssigned(
        std::function<void(std::uint32_t userId, ECS::Entity entity)> callback);

    /**
     * @brief Register callback for health updates (after ECS sync)
     * @param callback Function receiving the health event
     */
    void onHealthUpdate(std::function<void(const EntityHealthEvent&)> callback);

    /**
     * @brief Register callback for disconnection events
     * @param callback Function receiving the disconnect reason
     */
    void onDisconnect(std::function<void(network::DisconnectReason)> callback);

    /**
     * @brief Update the network system
     *
     * Polls the network client and processes any pending events.
     * Should be called once per frame.
     */
    void update();

    /**
     * @brief Re-register network callbacks
     *
     * Call this after another system has overwritten the NetworkClient
     * callbacks. This restores the ClientNetworkSystem's handlers for entity
     * spawn, move, etc.
     */
    void registerCallbacks();

    /**
     * @brief Send player input to server
     *
     * @param inputMask Combined input flags
     */
    void sendInput(std::uint8_t inputMask);

    /**
     * @brief Get the local player's entity (if connected and spawned)
     * @return Local player entity if available
     */
    [[nodiscard]] std::optional<ECS::Entity> getLocalPlayerEntity() const;

    /**
     * @brief Get the local player's user ID (if connected)
     * @return Local user ID if available
     */
    [[nodiscard]] std::optional<std::uint32_t> getLocalUserId() const;

    /**
     * @brief Find entity by network ID
     * @param networkId The network entity ID
     * @return Entity if found
     */
    [[nodiscard]] std::optional<ECS::Entity> findEntityByNetworkId(
        std::uint32_t networkId) const;

    /**
     * @brief Check if connected to server
     * @return true if connected
     */
    [[nodiscard]] bool isConnected() const;

    /**
     * @brief Reset the network system state
     *
     * Clears all tracked entities and resets state.
     * Should be called when disconnecting or switching scenes.
     */
    void reset();

   private:
    void handleEntitySpawn(const EntitySpawnEvent& event);
    void handleEntityMove(const EntityMoveEvent& event);

    void handlePowerUpEvent(const PowerUpEvent& event);

    void _playDeathSound(ECS::Entity entity);

    void handleEntityDestroy(std::uint32_t entityId);
    void handlePositionCorrection(float x, float y);
    void handleEntityHealth(const EntityHealthEvent& event);
    void handleConnected(std::uint32_t userId);
    void handleDisconnected(network::DisconnectReason reason);

    static ECS::Entity defaultEntityFactory(ECS::Registry& registry,
                                            const EntitySpawnEvent& event);

    std::shared_ptr<ECS::Registry> registry_;

    std::shared_ptr<NetworkClient> client_;

    EntityFactory entityFactory_;

    std::unordered_map<std::uint32_t, ECS::Entity> networkIdToEntity_;

    std::optional<std::uint32_t> localUserId_;
    std::optional<ECS::Entity> localPlayerEntity_;

    std::function<void(std::uint32_t, ECS::Entity)>
        onLocalPlayerAssignedCallback_;
    std::function<void(const EntityHealthEvent&)> onHealthUpdateCallback_;
    std::function<void(network::DisconnectReason)> onDisconnectCallback_;

    struct HealthCache {
        std::int32_t current;
        std::int32_t max;
    };
    std::unordered_map<std::uint32_t, HealthCache> lastKnownHealth_;

    std::unordered_map<std::uint32_t, ECS::Entity> pendingPlayerSpawns_;

    bool disconnectedHandled_{false};
};

}  // namespace rtype::client

#endif  // SRC_CLIENT_NETWORK_CLIENTNETWORKSYSTEM_HPP_
