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

}  // namespace rtype::games::rtype::shared
