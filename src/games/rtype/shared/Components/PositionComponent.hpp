/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PositionComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_SHARED_COMPONENTS_POSITIONCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_SHARED_COMPONENTS_POSITIONCOMPONENT_HPP_

namespace rtype::games::rtype::shared {

/**
 * @brief Component storing entity position in 2D world space.
 *
 * Used for all entities that have a position in the game world.
 * Combined with VelocityComponent by MovementSystem to update positions.
 *
 * Coordinate system:
 * - Origin (0,0) is at top-left of the world
 * - X increases to the right
 * - Y increases downward
 */
struct Position {
    float x = 0.0f;
    float y = 0.0f;
};

}  // namespace rtype::games::rtype::shared

#endif  // SRC_GAMES_RTYPE_SHARED_COMPONENTS_POSITIONCOMPONENT_HPP_
