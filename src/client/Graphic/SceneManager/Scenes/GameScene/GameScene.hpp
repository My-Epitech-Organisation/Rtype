/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_

#include "../AScene.hpp"
#include "SceneManager/SceneManager.hpp"

static constexpr int SIZE_X_PAUSE_MENU = 600;
static constexpr int SIZE_Y_PAUSE_MENU = 600;
static constexpr int SIZE_FONT_PAUSE_MENU = 40;
static constexpr std::string PAUSE_MENU_TITLE = "Pause";
// Player movement speed in pixels per second. Delta time is applied in
// MovementSystem::update() for frame-rate independent movement.
static constexpr float PLAYER_MOVEMENT_SPEED = 300.0f;

class GameScene : public AScene {
   private:
    const std::shared_ptr<KeyboardActions>& _keybinds;

    void _updateUserMovementUp();
    void _updateUserMovementDown();
    void _updateUserMovementLeft();
    void _updateUserMovementRight();

    void _handleKeyReleasedEvent(const sf::Event& event);

   public:
    void update() override;
    void render(const std::shared_ptr<sf::RenderWindow>& window) override;
    void pollEvents(const sf::Event& e) override;

    GameScene(const std::shared_ptr<ECS::Registry>& ecs,
              const std::shared_ptr<AssetManager>& textureManager,
              const std::shared_ptr<sf::RenderWindow>& window,
              const std::shared_ptr<KeyboardActions>& keybinds,
              std::function<void(const SceneManager::Scene&)> switchToScene);
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_GAMESCENE_GAMESCENE_HPP_
