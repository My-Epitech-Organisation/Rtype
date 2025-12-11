/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** IEntitySpawner - Interface for game-specific entity spawning
*/

#ifndef SRC_SERVER_SHARED_IENTITYSPAWNER_HPP_
#define SRC_SERVER_SHARED_IENTITYSPAWNER_HPP_

#include <cstdint>
#include <memory>
#include <optional>

#include <rtype/ecs.hpp>

namespace rtype::server {

// Forward declarations
class ServerNetworkSystem;

/**
 * @brief Result of spawning a player entity
 */
struct PlayerSpawnResult {
    ECS::Entity entity;       ///< The spawned entity
    std::uint32_t networkId;  ///< Network ID assigned to the player
    float x;                  ///< Spawn X position
    float y;                  ///< Spawn Y position
    std::int32_t health;      ///< Initial health
    std::int32_t maxHealth;   ///< Maximum health
    bool success{false};      ///< Whether spawn succeeded
};

/**
 * @brief Configuration for player spawning
 */
struct PlayerSpawnConfig {
    std::uint32_t userId;     ///< User ID of the connecting player
    std::size_t playerIndex;  ///< Index for calculating spawn position
};

/**
 * @interface IEntitySpawner
 * @brief Abstract interface for game-specific entity spawning
 *
 * This interface decouples the server from game-specific entity creation.
 * Each game implements this interface to handle player spawning with
 * its own components and logic.
 *
 * Example usage:
 * @code
 * class RTypeEntitySpawner : public IEntitySpawner { ... };
 *
 * // In server:
 * auto spawner = createEntitySpawner(registry, networkSystem);
 * auto result = spawner->spawnPlayer({userId, playerIndex});
 * if (result.success) {
 *     // Player spawned successfully
 * }
 * @endcode
 */
class IEntitySpawner {
   public:
    virtual ~IEntitySpawner() = default;

    /**
     * @brief Spawn a player entity
     *
     * Creates a player entity with all necessary components for the game.
     * The spawner handles position calculation, component setup, and
     * network registration.
     *
     * @param config Player spawn configuration
     * @return PlayerSpawnResult containing the spawned entity info
     */
    [[nodiscard]] virtual PlayerSpawnResult spawnPlayer(
        const PlayerSpawnConfig& config) = 0;

    /**
     * @brief Destroy a player entity
     *
     * Removes a player entity and handles cleanup.
     *
     * @param entity The player entity to destroy
     */
    virtual void destroyPlayer(ECS::Entity entity) = 0;

    /**
     * @brief Get player speed from game configuration
     *
     * @return Player movement speed
     */
    [[nodiscard]] virtual float getPlayerSpeed() const noexcept = 0;

    /**
     * @brief Handle player shooting
     *
     * Creates a projectile for the given player.
     *
     * @param playerEntity The player entity that is shooting
     * @param playerNetworkId Network ID of the player
     * @return Network ID of the spawned projectile (0 if failed)
     */
    [[nodiscard]] virtual std::uint32_t handlePlayerShoot(
        ECS::Entity playerEntity, std::uint32_t playerNetworkId) = 0;

    /**
     * @brief Check if a player can shoot (cooldown ready)
     *
     * @param playerEntity The player entity to check
     * @return true if the player can shoot
     */
    [[nodiscard]] virtual bool canPlayerShoot(
        ECS::Entity playerEntity) const = 0;

    /**
     * @brief Get the network ID for an entity
     *
     * @param entity The entity to query
     * @return Network ID if entity has one, std::nullopt otherwise
     */
    [[nodiscard]] virtual std::optional<std::uint32_t> getEntityNetworkId(
        ECS::Entity entity) const = 0;

    /**
     * @brief Get the position of an entity
     *
     * @param entity The entity to query
     * @param outX Output X position
     * @param outY Output Y position
     * @return true if entity has position, false otherwise
     */
    [[nodiscard]] virtual bool getEntityPosition(ECS::Entity entity,
                                                 float& outX,
                                                 float& outY) const = 0;

    /**
     * @brief Update player velocity
     *
     * @param entity The player entity
     * @param vx X velocity
     * @param vy Y velocity
     */
    virtual void updatePlayerVelocity(ECS::Entity entity, float vx,
                                      float vy) = 0;

    /**
     * @brief Trigger player shoot cooldown
     *
     * @param entity The player entity
     */
    virtual void triggerShootCooldown(ECS::Entity entity) = 0;
};

}  // namespace rtype::server

#endif  // SRC_SERVER_SHARED_IENTITYSPAWNER_HPP_
