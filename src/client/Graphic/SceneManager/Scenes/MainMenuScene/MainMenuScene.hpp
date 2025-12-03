/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_MAINMENUSCENE_MAINMENUSCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_MAINMENUSCENE_MAINMENUSCENE_HPP_

#include "../AScene.hpp"
#include "SceneManager/SceneManager.hpp"

class MainMenuScene : public AScene {
   private:
    std::vector<ECS::Entity> _listEntity;

   public:
    void update() override;
    void render(const std::shared_ptr<sf::RenderWindow>& window) override;
    void pollEvents(const sf::Event& e) override;

    MainMenuScene(
        const std::shared_ptr<ECS::Registry>& ecs,
        const std::shared_ptr<AssetManager>& textureManager,
        const std::shared_ptr<sf::RenderWindow>& window,
        std::function<void(const SceneManager::Scene&)> switchToScene);
    ~MainMenuScene() override;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_MAINMENUSCENE_MAINMENUSCENE_HPP_
