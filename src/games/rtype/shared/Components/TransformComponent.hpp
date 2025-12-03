/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** TransformComponent - Position and rotation data
*/

#pragma once

namespace rtype::games::rtype::shared {

/**
 * @struct TransformComponent
 * @brief Component storing entity position and rotation
 *
 * Used for all entities that have a position in the game world.
 * Shared between client (rendering) and server (physics/logic).
 */
struct TransformComponent {
    float x = 0.0F;         ///< X position in world coordinates
    float y = 0.0F;         ///< Y position in world coordinates
    float rotation = 0.0F;  ///< Rotation in degrees
};

}  // namespace rtype::games::rtype::shared
