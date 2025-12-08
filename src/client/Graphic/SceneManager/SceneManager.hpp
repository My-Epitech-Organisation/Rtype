/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEMANAGER_HPP_

#include <iostream>
#include <map>
#include <memory>
#include <optional>

#include "../KeyboardActions.hpp"
#include "AssetManager/AssetManager.hpp"
#include "ECS.hpp"
#include "Scenes/IScene.hpp"

class SceneManager {
   public:
    enum Scene {
        MAIN_MENU,
        IN_GAME,
        SETTINGS_MENU,
        NONE,
    };

   private:
    std::optional<Scene> _nextScene = std::nullopt;  // Scène en attente
    Scene _currentScene = NONE;

    std::map<Scene, std::function<std::unique_ptr<IScene>()>> _sceneList;
    std::unique_ptr<IScene> _activeScene;
    std::shared_ptr<sf::RenderWindow> _window;

    std::function<void(const Scene&)> _switchToScene;

    std::shared_ptr<KeyboardActions> _keybinds;

    void _applySceneChange();

   public:
    [[nodiscard]] Scene getCurrentScene() const { return _currentScene; }
    void setCurrentScene(Scene scene);

    void pollEvents(const sf::Event& e);
    void update();
    void draw();

    bool operator==(const Scene& data) const {
        if (data == this->_currentScene) return true;
        return false;
    }

    SceneManager(std::shared_ptr<ECS::Registry> ecs,
                 std::shared_ptr<AssetManager> assetManager,
                 std::shared_ptr<sf::RenderWindow> window,
                 std::shared_ptr<KeyboardActions> keybinds);
    ~SceneManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEMANAGER_HPP_
