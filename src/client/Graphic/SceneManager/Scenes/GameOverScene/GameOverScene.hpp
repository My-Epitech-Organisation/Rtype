/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameOverScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMEOVERSCENE_GAMEOVERSCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMEOVERSCENE_GAMEOVERSCENE_HPP_

#include <functional>
#include <memory>
#include <vector>

#include "AudioLib/AudioLib.hpp"
#include "SceneManager/SceneManager.hpp"
#include "SceneManager/Scenes/AScene.hpp"

class GameOverScene : public AScene {
   public:
    GameOverScene(
        std::shared_ptr<ECS::Registry> registry,
        std::shared_ptr<AssetManager> assetsManager,
        std::shared_ptr<rtype::display::IDisplay> window,
        std::shared_ptr<AudioLib> audio,
        std::function<void(const std::string&)> setBackground,
        std::function<void(const SceneManager::Scene&)> switchToScene);

    void pollEvents(const rtype::display::Event& e) override;
    void update(float dt) override;
    void render(std::shared_ptr<rtype::display::IDisplay> window) override;

   private:
    std::function<void(const SceneManager::Scene&)> _switchToScene;

    void _buildLayout();
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMEOVERSCENE_GAMEOVERSCENE_HPP_
