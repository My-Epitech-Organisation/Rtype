/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ButtonComponent.hpp
*/

#ifndef SRC_CLIENT_COMPONENTS_GRAPHIC_BUTTONCOMPONENT_HPP_
#define SRC_CLIENT_COMPONENTS_GRAPHIC_BUTTONCOMPONENT_HPP_

#include <functional>

template <typename... Args>
struct Button {
    std::function<void(Args...)> callback;
    explicit Button(std::function<void(Args...)> callback)
        : callback(callback) {}
};

#endif  // SRC_CLIENT_COMPONENTS_GRAPHIC_BUTTONCOMPONENT_HPP_
