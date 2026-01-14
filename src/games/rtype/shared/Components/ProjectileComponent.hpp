/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ProjectileComponent - Defines projectile properties (damage, owner, type)
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @enum ProjectileType
 * @brief Different types of projectiles with unique behaviors
 */
enum class ProjectileType : uint8_t {
    BasicBullet = 0,      ///< Standard player shot
    ChargedShot = 1,      ///< Charged powerful shot
    Missile = 2,          ///< Homing missile
    LaserBeam = 3,        ///< Piercing laser (fast projectile)
    SpreadShot = 4,       ///< Multiple projectiles
    ContinuousLaser = 5,  ///< Continuous beam weapon (hold to fire)
    EnemyBullet = 50,     ///< Standard enemy shot
    HeavyBullet = 51,     ///< Heavy damage shot
    BossBullet = 52,      ///< Boss projectiles
};

/**
 * @enum ProjectileOwner
 * @brief Identifies who fired the projectile
 */
enum class ProjectileOwner : uint8_t { Player = 0, Enemy = 1, Neutral = 2 };

/**
 * @struct ProjectileComponent
 * @brief Component storing projectile properties
 *
 * Contains damage, owner information, projectile type, and behavior flags.
 * Used by collision and damage systems.
 */
struct ProjectileComponent {
    int32_t damage = 25;
    uint32_t ownerNetworkId = 0;
    ProjectileOwner owner = ProjectileOwner::Player;
    ProjectileType type = ProjectileType::BasicBullet;
    bool piercing = false;
    int32_t maxHits = 1;
    int32_t currentHits = 0;

    ProjectileComponent() = default;

    /**
     * @brief Construct a projectile with specific properties
     * @param dmg Damage value
     * @param ownerId Network ID of the owner
     * @param ownerType Player or Enemy
     * @param projType Projectile type
     */
    ProjectileComponent(int32_t dmg, uint32_t ownerId,
                        ProjectileOwner ownerType,
                        ProjectileType projType = ProjectileType::BasicBullet)
        : damage(dmg),
          ownerNetworkId(ownerId),
          owner(ownerType),
          type(projType) {}

    /**
     * @brief Register a hit and check if projectile should be destroyed
     * @return true if projectile should be destroyed
     */
    [[nodiscard]] bool registerHit() noexcept {
        currentHits++;
        return !piercing || currentHits >= maxHits;
    }

    /**
     * @brief Check if this projectile can hit a specific target type
     * @param targetIsPlayer True if target is a player
     * @return true if can damage target
     */
    [[nodiscard]] bool canHit(bool targetIsPlayer) const noexcept {
        if (owner == ProjectileOwner::Neutral) {
            return true;
        }
        return (owner == ProjectileOwner::Player) != targetIsPlayer;
    }
};

/**
 * @struct PlayerProjectileTag
 * @brief Tag for projectiles fired by players
 */
struct PlayerProjectileTag {};

/**
 * @struct EnemyProjectileTag
 * @brief Tag for projectiles fired by enemies
 */
struct EnemyProjectileTag {};

}  // namespace rtype::games::rtype::shared
