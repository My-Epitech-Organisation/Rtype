/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PlayerSpawner - Handles player entity spawning
*/

#ifndef SRC_SERVER_SERVERAPP_PLAYER_PLAYERSPAWNER_PLAYERSPAWNER_HPP_
#define SRC_SERVER_SERVERAPP_PLAYER_PLAYERSPAWNER_PLAYERSPAWNER_HPP_

#include <cstdint>
#include <memory>
#include <optional>

#include <rtype/ecs.hpp>

namespace rtype::server {

class ServerNetworkSystem;

/**
 * @brief Result of spawning a player
 */
struct PlayerSpawnResult {
    bool success{false};
    ECS::Entity entity{};
    std::uint32_t networkId{0};
    float x{0.0F};
    float y{0.0F};
};

/**
 * @brief Spawning configuration
 */
struct SpawnConfig {
    float baseX{100.0F};
    float baseY{150.0F};
    float yOffset{100.0F};
    float playerWidth{33.0F};
    float playerHeight{17.0F};
    int playerLives{3};
    float shootCooldown{0.3F};
};

/**
 * @brief Handles spawning and destroying player entities
 *
 * Creates player entities with all required components:
 * - Position, Transform, Velocity
 * - ShootCooldown, Weapon, BoundingBox
 * - PlayerTag, Health, NetworkId
 */
class PlayerSpawner {
   public:
    /**
     * @brief Construct a PlayerSpawner
     * @param registry ECS registry
     * @param networkSystem Network system for entity registration
     * @param config Spawn configuration
     */
    PlayerSpawner(std::shared_ptr<ECS::Registry> registry,
                  ServerNetworkSystem* networkSystem,
                  const SpawnConfig& config = {});

    ~PlayerSpawner() = default;

    /**
     * @brief Spawn a player entity
     * @param userId User ID for the player
     * @param playerIndex Index for spawn position calculation
     * @return Spawn result with entity info
     */
    [[nodiscard]] PlayerSpawnResult spawnPlayer(std::uint32_t userId,
                                                size_t playerIndex);

    /**
     * @brief Destroy a player entity
     * @param userId User ID
     * @return true if entity was found and destroyed
     */
    bool destroyPlayer(std::uint32_t userId);

    /**
     * @brief Get player entity for a user
     */
    [[nodiscard]] std::optional<ECS::Entity> getPlayerEntity(
        std::uint32_t userId) const;

   private:
    std::shared_ptr<ECS::Registry> _registry;
    ServerNetworkSystem* _networkSystem;
    SpawnConfig _config;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SERVERAPP_PLAYER_PLAYERSPAWNER_PLAYERSPAWNER_HPP_
