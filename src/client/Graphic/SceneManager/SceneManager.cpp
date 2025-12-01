/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.cpp
*/

#include "SceneManager.hpp"
#include "SceneException.hpp"
#include "../../GameAction.hpp"
#include "Scenes/MainMenuScene/MainMenuScene.hpp"

void SceneManager::setCurrentScene(const Scene scene) {
    if (this->_sceneList.contains(scene)) {
        throw SceneNotFound();
    }
    this->_currentScene = scene;
}

void SceneManager::pollEvents(const sf::Event &e) {
    if (!this->_sceneList.contains(this->_currentScene)) {
        throw SceneNotFound();
    }
    this->_sceneList[this->_currentScene]->pollEvents(e);
}

void SceneManager::update() {
    if (!this->_sceneList.contains(this->_currentScene)) {
        throw SceneNotFound();
    }
    this->_sceneList[this->_currentScene]->update();
}

void SceneManager::draw(sf::RenderWindow &window) {
    if (!this->_sceneList.contains(this->_currentScene)) {
        throw SceneNotFound();
    }
    this->_sceneList[this->_currentScene]->render(window);
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

SceneManager::SceneManager(const std::shared_ptr<ECS::Registry> &ecs, const std::shared_ptr<AssetManager> &texture) {
    auto menu = std::make_unique<MainMenuScene>(ecs, texture);

    this->_sceneList.emplace(
        MAIN_MENU,
        std::move(menu)
    );
}
