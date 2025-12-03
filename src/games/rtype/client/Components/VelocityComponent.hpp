/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** VelocityComponent.hpp
*/

#ifndef SRC_CLIENT_COMPONENTS_GRAPHIC_VELOCITYCOMPONENT_HPP_
#define SRC_CLIENT_COMPONENTS_GRAPHIC_VELOCITYCOMPONENT_HPP_

struct Velocity {
   public:
    float x = 0.0f;
    float y = 0.0f;
    Velocity(const float& x, const float& y) : x(x), y(y) {}
    Velocity() = default;
};

#endif  // SRC_CLIENT_COMPONENTS_GRAPHIC_VELOCITYCOMPONENT_HPP_
