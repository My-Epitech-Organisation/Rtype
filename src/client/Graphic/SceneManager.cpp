/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.cpp
*/

#include "SceneManager.hpp"

void SceneManager::pollEventsScene() {
}

void SceneManager::updateScene() {
}

void SceneManager::renderScene() {
}

std::ostream & operator<<(std::ostream &os, const SceneManager &sceneManager) {
    os << "Current Scene: ";
    switch (sceneManager.getCurrentScene()) {
        case SceneManager::MAIN_MENU:
            os << "MAIN_MENU";
            break;
        case SceneManager::IN_GAME:
            os << "IN_GAME";
            break;
        case SceneManager::PAUSE_MENU:
            os << "PAUSE_MENU";
            break;
        case SceneManager::GAME_OVER:
            os << "GAME_OVER";
            break;
        default:
            os << "UNKNOWN_SCENE";
            break;
    }
    return os;
}
