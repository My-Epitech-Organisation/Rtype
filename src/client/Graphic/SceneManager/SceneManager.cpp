/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.cpp
*/

#include "SceneManager.hpp"

#include "../../GameAction.hpp"
#include "SceneException.hpp"
#include "Scenes/MainMenuScene/MainMenuScene.hpp"

void SceneManager::setCurrentScene(const Scene scene) {
    if (!this->_sceneList.contains(scene)) {
        throw SceneNotFound();
    }
    if (this->_currentScene == scene) return;
    this->_currentScene = scene;
    this->_activeScene = this->_sceneList[this->_currentScene]();
}

void SceneManager::pollEvents(const sf::Event& e) {
    if (!this->_sceneList.contains(this->_currentScene)) {
        throw SceneNotFound();
    }
    if (!this->_activeScene) {
        throw SceneNotInitialized();
    }
    this->_activeScene->pollEvents(e);
}

void SceneManager::update() {
    if (!this->_sceneList.contains(this->_currentScene)) {
        throw SceneNotFound();
    }
    if (!this->_activeScene) {
        throw SceneNotInitialized();
    }
    this->_activeScene->update();
}

void SceneManager::draw(sf::RenderWindow& window) {
    if (!this->_sceneList.contains(this->_currentScene)) {
        throw SceneNotFound();
    }
    if (!this->_activeScene) {
        throw SceneNotInitialized();
    }
    this->_activeScene->render(window);
}

std::ostream& operator<<(std::ostream& os, const SceneManager& sceneManager) {
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
        case SceneManager::SETTINGS_MENU:
            os << "SETTINGS_MENU";
            break;
        default:
            os << "UNKNOWN_SCENE";
            break;
    }
    return os;
}

SceneManager::SceneManager(const std::shared_ptr<ECS::Registry>& ecs,
                           const std::shared_ptr<AssetManager>& texture,
                           sf::RenderWindow& window) {
    this->_sceneList.emplace(MAIN_MENU, [ecs, texture, &window, this]() {
        return std::make_unique<MainMenuScene>(
            ecs, texture, this->_switchToScene,
            window);  // CRÉATION 2 (La lambda)
    });
    this->setCurrentScene(MAIN_MENU);
}
