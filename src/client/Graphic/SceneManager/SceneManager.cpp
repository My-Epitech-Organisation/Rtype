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
#include "Scenes/SettingsScene/SettingsScene.hpp"

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

void SceneManager::draw() {
    if (!this->_sceneList.contains(this->_currentScene)) {
        throw SceneNotFound();
    }
    if (!this->_activeScene) {
        throw SceneNotInitialized();
    }
    this->_activeScene->render(this->_window);
}

SceneManager::SceneManager(std::shared_ptr<ECS::Registry> ecs,
                           std::shared_ptr<AssetManager> texture,
                           std::shared_ptr<sf::RenderWindow> window,
                           std::shared_ptr<KeyboardActions> keybinds)
    : _window(window), _keybinds(keybinds) {
    this->_switchToScene = [this](const Scene& scene) {
        this->setCurrentScene(scene);
    };
    this->_sceneList.emplace(MAIN_MENU, [ecs, texture, window, this]() {
        return std::make_unique<MainMenuScene>(ecs, texture, window,
                                               this->_switchToScene);
    });
    this->_sceneList.emplace(SETTINGS_MENU, [ecs, texture, window, this]() {
        return std::make_unique<SettingsScene>(
            ecs, texture, window, this->_switchToScene, this->_keybinds);
    });
    this->setCurrentScene(MAIN_MENU);
}
