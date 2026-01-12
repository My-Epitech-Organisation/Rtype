/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** HowToPlayScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_HOWTOPLAYSCENE_HOWTOPLAYSCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_HOWTOPLAYSCENE_HOWTOPLAYSCENE_HPP_

#include <functional>
#include <memory>
#include <string>

#include "Graphic/KeyboardActions.hpp"
#include "SceneManager/SceneManager.hpp"
#include "SceneManager/Scenes/AScene.hpp"

class HowToPlayScene : public AScene {
   private:
    std::shared_ptr<KeyboardActions> _keybinds;
    std::function<void(const SceneManager::Scene&)> _switchToScene;

    void _initLayout();
    std::string _keyName(GameAction action) const;

   public:
    HowToPlayScene(
        std::shared_ptr<ECS::Registry> ecs,
        std::shared_ptr<AssetManager> assetsManager,
        std::shared_ptr<::rtype::display::IDisplay> window,
        std::shared_ptr<KeyboardActions> keybinds,
        std::shared_ptr<AudioLib> audio,
        std::function<void(const std::string&)> setBackground,
        std::function<void(const SceneManager::Scene&)> switchToScene);

    void pollEvents(const rtype::display::Event& e) override;
    void update(float dt) override;
    void render(std::shared_ptr<::rtype::display::IDisplay> window) override;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_HOWTOPLAYSCENE_HOWTOPLAYSCENE_HPP_
