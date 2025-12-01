/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.cpp
*/

#include "SceneManager.hpp"

void SceneManager::pollEventsScene() {}

void SceneManager::updateScene() {}

void SceneManager::renderScene() {}

auto operator<<(std::ostream& stream, const SceneManager& sceneManager)
    -> std::ostream& {
    stream << "Current Scene: ";
    switch (sceneManager.getCurrentScene()) {
        case SceneManager::MAIN_MENU:
            stream << "MAIN_MENU";
            break;
        case SceneManager::IN_GAME:
            stream << "IN_GAME";
            break;
        case SceneManager::PAUSE_MENU:
            stream << "PAUSE_MENU";
            break;
        case SceneManager::GAME_OVER:
            stream << "GAME_OVER";
            break;
        default:
            stream << "UNKNOWN_SCENE";
            break;
    }
    return stream;
}
