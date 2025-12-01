/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.hpp
*/

#ifndef R_TYPE_MAINMENUSCENE_HPP
#define R_TYPE_MAINMENUSCENE_HPP

#include "../AScene.hpp"
#include "SceneManager/SceneManager.hpp"

class MainMenuScene : public AScene {
private:
    std::vector<ECS::Entity> _listEntity;
public:
    void update() override;
    void render(sf::RenderWindow &window) override;
    void pollEvents(const sf::Event &e) override;

    MainMenuScene(
        const std::shared_ptr<ECS::Registry> &ecs,
        const std::shared_ptr<AssetManager> &textureManager,
        std::function<void(const SceneManager::Scene &)> switchToScene,
        sf::RenderWindow &window);
    ~MainMenuScene() override;
};

#endif //R_TYPE_MAINMENUSCENE_HPP
