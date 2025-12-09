/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Tags - Tag components for entity classification
*/

#pragma once

namespace rtype::games::rtype::shared {

// =============================================================================
// Entity Classification Tags
// =============================================================================

/**
 * @struct PlayerTag
 * @brief Tag component to mark an entity as a player
 */
struct PlayerTag {};

/**
 * @struct EnemyTag
 * @brief Tag component to mark an entity as an enemy
 */
struct EnemyTag {};

/**
 * @struct ProjectileTag
 * @brief Tag component to mark an entity as a projectile
 */
struct ProjectileTag {};

/**
 * @struct PickupTag
 * @brief Tag component to mark an entity as a pickup item
 */
struct PickupTag {};

// =============================================================================
// Enemy Type Tags
// =============================================================================

/**
 * @struct BydosSlaveTag
 * @brief Tag component for Bydos Slave enemy type
 */
struct BydosSlaveTag {};

/**
 * @struct BydosMasterTag
 * @brief Tag component for Bydos Master enemy type
 */
struct BydosMasterTag {};

// =============================================================================
// State Tags
// =============================================================================

/**
 * @struct DestroyTag
 * @brief Tag component to mark entities for destruction
 *
 * Added to entities that should be destroyed at the end of the frame.
 * Processed by DestroySystem.
 */
struct DestroyTag {};

/**
 * @struct InvincibleTag
 * @brief Tag component for temporary invincibility
 */
struct InvincibleTag {};

/**
 * @struct DisabledTag
 * @brief Tag component for disabled entities (not updated)
 */
struct DisabledTag {};

/**
 * @struct BackgroundTag
 * @brief Tag component to mark background entities
 */
struct BackgroundTag {};

/**
 * @struct ControllableTag
 * @brief Tag component for controllable entities
 */
struct ControllableTag {};

/**
 * @struct StaticTextTag
 * @brief Tag component for static text entities
 */
struct StaticTextTag {};

// =============================================================================
// Map Element Tags
// =============================================================================

/**
 * @struct ObstacleTag
 * @brief Tag component for static map obstacles (indestructible)
 *
 * Used for walls, platforms, and other solid objects that block movement.
 */
struct ObstacleTag {};

/**
 * @struct DestroyableTileTag
 * @brief Tag component for destroyable map tiles
 *
 * These tiles can be destroyed by player projectiles.
 */
struct DestroyableTileTag {};

/**
 * @struct DecorationTag
 * @brief Tag component for visual decorations (no collision)
 *
 * Used for non-interactive visual elements in the map.
 */
struct DecorationTag {};

/**
 * @struct StarfieldTag
 * @brief Tag component for starfield background layers
 *
 * Used for parallax scrolling background elements.
 */
struct StarfieldTag {};

/**
 * @struct MapElementTag
 * @brief Base tag for all map elements (obstacles, tiles, decorations)
 *
 * Useful for querying all scrollable map elements at once.
 */
struct MapElementTag {};

}  // namespace rtype::games::rtype::shared
