/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ProjectileSpawnerSystem - Server-side projectile spawning system
*/

#pragma once

#include <functional>
#include <random>

#include <rtype/engine.hpp>

#include "../../../shared/Components/ProjectileComponent.hpp"
#include "../../../shared/Components/WeaponComponent.hpp"

namespace rtype::games::rtype::server {

/**
 * @struct ProjectileSpawnConfig
 * @brief Configuration for projectile spawning
 */
struct ProjectileSpawnConfig {
    float playerProjectileOffsetX = 32.0F;
    float playerProjectileOffsetY = 0.0F;
    float enemyProjectileOffsetX = -32.0F;
    float enemyProjectileOffsetY = 0.0F;
};

/**
 * @class ProjectileSpawnerSystem
 * @brief Server-side system that handles projectile spawning
 *
 * This is a server-specific system - clients receive spawn events
 * through the network, they don't spawn projectiles themselves.
 *
 * Handles:
 * - Player shooting (spacebar input)
 * - Enemy shooting (AI controlled)
 * - Cooldown management
 * - Different projectile types
 */
class ProjectileSpawnerSystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    /**
     * @brief Construct ProjectileSpawnerSystem with event emitter
     * @param emitter Function to emit game events
     * @param config Spawner configuration
     */
    explicit ProjectileSpawnerSystem(
        EventEmitter emitter,
        ProjectileSpawnConfig config = ProjectileSpawnConfig{});

    void update(ECS::Registry& registry, float deltaTime) override;

    /**
     * @brief Spawn a projectile from a player
     * @param registry ECS registry
     * @param playerEntity The player entity shooting
     * @param playerNetworkId The player's network ID
     * @param playerX Player X position
     * @param playerY Player Y position
     * @return Network ID of spawned projectile
     */
    uint32_t spawnPlayerProjectile(ECS::Registry& registry,
                                   uint32_t playerNetworkId, float playerX,
                                   float playerY);
    /**
     * @brief Spawn a projectile from an enemy
     * @param registry ECS registry
     * @param enemyEntity The enemy entity shooting
     * @param enemyNetworkId The enemy's network ID
     * @param enemyX Enemy X position
     * @param enemyY Enemy Y position
     * @param targetX Target X position (for aiming)
     * @param targetY Target Y position (for aiming)
     * @return Network ID of spawned projectile
     */
    uint32_t spawnEnemyProjectile(ECS::Registry& registry,
                                  ECS::Entity enemyEntity,
                                  uint32_t enemyNetworkId, float enemyX,
                                  float enemyY, float targetX, float targetY);

    /**
     * @brief Spawn a charged shot projectile from a player
     * @param registry ECS registry
     * @param playerNetworkId The player's network ID
     * @param playerX Player X position
     * @param playerY Player Y position
     * @param chargeLevel Charge level (1-3)
     * @return Network ID of spawned projectile
     */
    uint32_t spawnChargedProjectile(ECS::Registry& registry,
                                    uint32_t playerNetworkId, float playerX,
                                    float playerY, uint8_t chargeLevel);

    /**
     * @brief Get current projectile count
     * @return Number of active projectiles tracked
     */
    [[nodiscard]] std::size_t getProjectileCount() const noexcept {
        return _projectileCount;
    }

    /**
     * @brief Decrement projectile count (called when projectile destroyed)
     */
    void decrementProjectileCount() noexcept {
        if (_projectileCount > 0) --_projectileCount;
    }

   private:
    /**
     * @brief Spawn projectile with specific configuration
     * @param registry ECS registry
     * @param x Spawn X position
     * @param y Spawn Y position
     * @param vx Velocity X
     * @param vy Velocity Y
     * @param config Weapon configuration
     * @param owner Owner type (Player/Enemy)
     * @param ownerNetworkId Network ID of owner
     * @param subTypeOverride Optional override for subType (used for charged shots with level encoding)
     * @return Network ID of spawned projectile
     */
    uint32_t spawnProjectileWithConfig(ECS::Registry& registry, float x,
                                       float y, float vx, float vy,
                                       const shared::WeaponConfig& config,
                                       shared::ProjectileOwner owner,
                                       uint32_t ownerNetworkId,
                                       uint8_t subTypeOverride = 0);

    EventEmitter _emitEvent;
    ProjectileSpawnConfig _config;
    std::size_t _projectileCount = 0;
    uint32_t _nextNetworkId = 100000;

    std::mt19937 _rng;
};

}  // namespace rtype::games::rtype::server
