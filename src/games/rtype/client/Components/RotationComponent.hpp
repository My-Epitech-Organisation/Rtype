/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RotationComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_ROTATIONCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_ROTATIONCOMPONENT_HPP_

namespace rtype::games::rtype::client {

/**
 * @brief Component that stores rotation angle in degrees
 */
struct Rotation {
    float angle;

    explicit Rotation(float angle = 0.0f) : angle(angle) {}
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_ROTATIONCOMPONENT_HPP_
