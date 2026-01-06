/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOXINGCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOXINGCOMPONENT_HPP_

#include "rtype/display/DisplayTypes.hpp"

namespace rtype::games::rtype::client {
struct BoxingComponent {
    ::rtype::display::Vector2f position;
    ::rtype::display::Vector2f size;
    ::rtype::display::Color outlineColor = {255, 0, 0, 255};
    ::rtype::display::Color fillColor = {255, 255, 255, 30};
    float outlineThickness = 2.f;

    explicit BoxingComponent(::rtype::display::Vector2f pos,
                             ::rtype::display::Vector2f sz)
        : position(pos), size(sz) {}
};
}  // namespace rtype::games::rtype::client
#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOXINGCOMPONENT_HPP_
