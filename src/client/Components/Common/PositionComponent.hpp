/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PositionComponent.hpp
*/

#ifndef SRC_CLIENT_COMPONENTS_COMMON_POSITIONCOMPONENT_HPP_
#define SRC_CLIENT_COMPONENTS_COMMON_POSITIONCOMPONENT_HPP_

struct Position {
    float x = 0;
    float y = 0;
    float initialX = x;
    float initialY = y;
    Position(const float& x, const float& y) : x(x), y(y) {}
};

#endif  // SRC_CLIENT_COMPONENTS_COMMON_POSITIONCOMPONENT_HPP_
