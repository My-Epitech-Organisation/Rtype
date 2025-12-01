/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ButtonComponent.hpp
*/

#ifndef R_TYPE_BUTTONCOMPONENT_HPP
#define R_TYPE_BUTTONCOMPONENT_HPP

#include <functional>

template<typename ... Args>
struct Button {
    std::function<void(Args...)> callback;
    explicit Button(std::function<void(Args...)> callback) : callback(callback) {};
};

#endif //R_TYPE_BUTTONCOMPONENT_HPP