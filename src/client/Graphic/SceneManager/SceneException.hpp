/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneException.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEEXCEPTION_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEEXCEPTION_HPP_
#include <exception>

class SceneNotFound final : std::exception {
   public:
    [[nodiscard]] const char* what() const noexcept override {
        return "Scene not found";
    }
};

class SceneNotInitialized final : std::exception {
   public:
    [[nodiscard]] const char* what() const noexcept override {
        return "Scene not initialized";
    }
};
#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEEXCEPTION_HPP_
