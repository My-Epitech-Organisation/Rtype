/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SizeComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_SIZECOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_SIZECOMPONENT_HPP_

namespace rtype::games::rtype::client {

struct Size {
    float x;
    float y;
    Size(const float& x, const float& y) : x(x), y(y) {}
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_SIZECOMPONENT_HPP_
