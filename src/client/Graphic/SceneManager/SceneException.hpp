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
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "Scene not found";
    }
};

class SceneNotInitialized final : std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "Scene not initialized";
    }
};
#endif //R_TYPE_SCENEEXCEPTION_HPP