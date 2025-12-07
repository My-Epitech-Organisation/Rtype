/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ButtonComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BUTTONCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BUTTONCOMPONENT_HPP_

#include <functional>

namespace rtype::games::rtype::client {

template <typename... Args>
struct Button {
    std::function<void(Args...)> callback;
    explicit Button(std::function<void(Args...)> callback)
        : callback(callback) {}
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BUTTONCOMPONENT_HPP_
