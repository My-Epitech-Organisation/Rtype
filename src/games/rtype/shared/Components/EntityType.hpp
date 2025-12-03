/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EntityType - Entity type enumeration
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @enum EntityType
 * @brief Entity type enumeration for network serialization
 *
 * Used to identify entity types when synchronizing over network.
 */
enum class EntityType : uint8_t {
    Unknown = 0,  ///< Unknown/uninitialized entity
    Player,       ///< Player-controlled ship
    Enemy,        ///< Enemy entity
    Projectile,   ///< Projectile (bullet, missile, etc.)
    Pickup,       ///< Collectible item (power-up, etc.)
    Obstacle      ///< Static or moving obstacle
};

}  // namespace rtype::games::rtype::shared
