/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.hpp
*/

#ifndef R_TYPE_SCENEMANAGER_HPP
#define R_TYPE_SCENEMANAGER_HPP

#include <iostream>

class SceneManager {
public:
    enum Scene {
        MAIN_MENU,
        IN_GAME,
        PAUSE_MENU,
        GAME_OVER
    };

private:
    Scene _currentScene = MAIN_MENU;

public:

    [[nodiscard]] Scene getCurrentScene() const { return _currentScene; }
    void setCurrentScene(Scene scene) { _currentScene = scene; }

    bool operator==(const Scene &data) const {
        if (data == this->_currentScene)
            return true;
        return false;
    }

    void pollEventsScene();
    void updateScene();
    void renderScene();

    friend std::ostream &operator<<(std::ostream &os, const SceneManager &sceneManager);

    SceneManager() = default;
    ~SceneManager() = default;
};


#endif //R_TYPE_SCENEMANAGER_HPP