/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** VelocityComponent - Movement velocity data
*/

#pragma once

namespace rtype::games::rtype::shared {

/**
 * @struct VelocityComponent
 * @brief Component storing entity velocity
 *
 * Used for entities that can move. Combined with TransformComponent
 * by the MovementSystem to update positions.
 */
struct VelocityComponent {
    float vx = 0.0F;  ///< Velocity on X axis (pixels/second)
    float vy = 0.0F;  ///< Velocity on Y axis (pixels/second)
};

}  // namespace rtype::games::rtype::shared
