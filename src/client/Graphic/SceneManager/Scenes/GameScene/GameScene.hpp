/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_

#include "../AScene.hpp"
#include "AudioLib/AudioLib.hpp"
#include "SceneManager/SceneManager.hpp"

static constexpr int SizeXPauseMenu = 600;
static constexpr int SizeYPauseMenu = 600;
static constexpr int SizeFontPauseMenu = 40;
static constexpr std::string PauseMenuTitle = "Pause";

static constexpr float PlayerMovementSpeed = 300.0f;

class GameScene : public AScene {
   private:
    std::shared_ptr<KeyboardActions> _keybinds;

    void _updateUserMovementUp();
    void _updateUserMovementDown();
    void _updateUserMovementLeft();
    void _updateUserMovementRight();

    void _handleKeyReleasedEvent(const sf::Event& event);

   public:
    void update() override;
    void render(std::shared_ptr<sf::RenderWindow> window) override;
    void pollEvents(const sf::Event& e) override;

    GameScene(std::shared_ptr<ECS::Registry> ecs,
              std::shared_ptr<AssetManager> textureManager,
              std::shared_ptr<sf::RenderWindow> window,
              std::shared_ptr<KeyboardActions> keybinds,
              std::shared_ptr<AudioLib> audio,
              std::function<void(const SceneManager::Scene&)> switchToScene);
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_
