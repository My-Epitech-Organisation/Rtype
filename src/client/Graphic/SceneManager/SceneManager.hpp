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

#include "AssetManager/AssetManager.hpp"
#include "Scenes/IScene.hpp"
#include "ecs/ECS.hpp"
#include "../KeyboardActions.hpp"

class SceneManager {
   public:
    enum Scene {
        MAIN_MENU,
        IN_GAME,
        PAUSE_MENU,
        GAME_OVER,
        SETTINGS_MENU,
        QUIT,
        NONE,
    };

   private:
    Scene _currentScene = NONE;

    std::map<Scene, std::function<std::unique_ptr<IScene>()>> _sceneList;
    std::unique_ptr<IScene> _activeScene;

    std::function<void(const Scene&)> _switchToScene =
        std::function<void(const Scene&)>(
            [this](const Scene& scene) { this->setCurrentScene(scene); });

    KeyboardActions& _keybinds;

   public:
    [[nodiscard]] Scene getCurrentScene() const { return _currentScene; }
    void setCurrentScene(Scene scene);

    void pollEvents(const sf::Event& e);
    void update();
    void draw(sf::RenderWindow& window);

    bool operator==(const Scene& data) const {
        if (data == this->_currentScene) return true;
        return false;
    }

    friend std::ostream& operator<<(std::ostream& os,
                                    const SceneManager& sceneManager);

    SceneManager(const std::shared_ptr<ECS::Registry>& ecs,
                 const std::shared_ptr<AssetManager>& assetManager,
                 sf::RenderWindow& window, KeyboardActions& keybinds);
    ~SceneManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENEMANAGER_HPP_
