/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_HPP_

#include <cstdint>
#include <iostream>

class SceneManager {
   public:
    enum Scene : std::uint8_t { MAIN_MENU, IN_GAME, PAUSE_MENU, GAME_OVER };

   private:
    Scene _currentScene = MAIN_MENU;

   public:
    auto getCurrentScene() const -> Scene { return _currentScene; }
    void setCurrentScene(Scene scene) { _currentScene = scene; }

    auto operator==(const Scene& data) const -> bool {
        return data == this->_currentScene;
    }

    void pollEventsScene();
    void updateScene();
    void renderScene();

    friend auto operator<<(std::ostream& stream,
                           const SceneManager& sceneManager) -> std::ostream&;

    SceneManager() = default;
    ~SceneManager() = default;

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;
    SceneManager(SceneManager&&) = delete;
    SceneManager& operator=(SceneManager&&) = delete;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_HPP_
