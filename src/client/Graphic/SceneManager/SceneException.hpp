/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneException.hpp
*/

#ifndef R_TYPE_SCENEEXCEPTION_HPP
#define R_TYPE_SCENEEXCEPTION_HPP
#include <exception>

class SceneNotFound final : std::exception {
    [[nodiscard]] const char *what() const noexcept override {
        return "Scene not found";
    }
};
#endif //R_TYPE_SCENEEXCEPTION_HPP