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
 * Values MUST match network::EntityType in Payloads.hpp
 */
enum class EntityType : uint8_t {
    Unknown = 0,   // Unknown/uninitialized entity
    Player = 0,    // Player-controlled ship (same as network::EntityType::Player)
    Enemy = 1,     // Enemy entity (same as network::EntityType::Bydos)
    Projectile = 2,// Projectile (same as network::EntityType::Missile)
    Pickup = 3,    // Collectible item (same as network::EntityType::Pickup)
    Obstacle = 4,  // Static or moving obstacle
    ForcePod = 5,  // Force Pod companion entity
    LaserBeam = 6, // Continuous laser beam weapon
    Boss = 7,      // Boss entity
    BossPart = 8   // Boss weak point / body part
};

}  // namespace rtype::games::rtype::shared
