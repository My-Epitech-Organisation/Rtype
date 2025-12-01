/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PositionComponent.hpp
*/

#ifndef R_TYPE_POSITIONCOMPONENT_HPP
#define R_TYPE_POSITIONCOMPONENT_HPP

struct Position {
    float x;
    float y;
    Position(const float &x, const float &y) : x(x), y(y) {}
};

#endif //R_TYPE_POSITIONCOMPONENT_HPP