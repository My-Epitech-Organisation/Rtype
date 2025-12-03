/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.hpp
*/

#ifndef R_TYPE_GAMESCENE_HPP
#define R_TYPE_GAMESCENE_HPP

#include "../AScene.hpp"
#include "SceneManager/SceneManager.hpp"

class GameScene  : public AScene {

public:

    void update() override;
    void render(const std::shared_ptr<sf::RenderWindow>& window) override;
    void pollEvents(const sf::Event& e) override;

    GameScene(
        const std::shared_ptr<ECS::Registry>& ecs,
        const std::shared_ptr<AssetManager>& textureManager,
        const std::shared_ptr<sf::RenderWindow>& window,
        std::function<void(const SceneManager::Scene&)> switchToScene);

};


#endif //R_TYPE_GAMESCENE_HPP