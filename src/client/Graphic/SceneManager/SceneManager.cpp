/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.cpp
*/

#include "SceneManager.hpp"

#include "../../GameAction.hpp"
#include "SceneException.hpp"
#include "Scenes/GameScene/GameScene.hpp"
#include "Scenes/MainMenuScene/MainMenuScene.hpp"
#include "Scenes/SettingsScene/SettingsScene.hpp"
#include "Scenes/StressTestScene/StressTestScene.hpp"

void SceneManager::_applySceneChange() {
    if (this->_nextScene.has_value()) {
        Scene scene = this->_nextScene.value();

        if (this->_currentScene == scene) {
            this->_nextScene = std::nullopt;
            return;
        }

        this->_currentScene = scene;
        this->_activeScene = this->_sceneList[this->_currentScene]();

        this->_nextScene = std::nullopt;
    }
}

void SceneManager::setCurrentScene(const Scene scene) {
    if (!this->_sceneList.contains(scene)) {
        throw SceneNotFound();
    }
    this->_nextScene = scene;
}

void SceneManager::pollEvents(const sf::Event& e) {
    this->_applySceneChange();

    if (!this->_activeScene) {
        throw SceneNotInitialized();
    }
    this->_activeScene->pollEvents(e);
}

void SceneManager::update() {
    this->_applySceneChange();

    if (!this->_activeScene) {
        throw SceneNotInitialized();
    }
    this->_activeScene->update();
}

void SceneManager::draw() {
    this->_applySceneChange();

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
    this->_sceneList.emplace(MAIN_MENU, [ecs, texture, this]() {
        return std::make_unique<MainMenuScene>(ecs, texture, this->_window,
                                               this->_switchToScene);
    });
    this->_sceneList.emplace(SETTINGS_MENU, [ecs, texture, this]() {
        return std::make_unique<SettingsScene>(
            ecs, texture, this->_window, this->_switchToScene, this->_keybinds);
    });
    this->_sceneList.emplace(IN_GAME, [ecs, texture, this]() {
        return std::make_unique<GameScene>(
            ecs, texture, this->_window, this->_keybinds, this->_switchToScene);
    });
    this->_sceneList.emplace(STRESS_TEST, [ecs, texture, this]() {
        return std::make_unique<StressTestScene>(ecs, texture, this->_window,
                                                 this->_switchToScene);
    });
    this->setCurrentScene(MAIN_MENU);
}
