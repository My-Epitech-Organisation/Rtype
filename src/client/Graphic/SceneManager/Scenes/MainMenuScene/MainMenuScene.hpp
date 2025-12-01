/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MainMenuScene.hpp
*/

#ifndef R_TYPE_MAINMENUSCENE_HPP
#define R_TYPE_MAINMENUSCENE_HPP

#include "../AScene.hpp"

class MainMenuScene : public AScene {
private:
    ECS::Entity _background;
public:
    void update() override;
    void render(sf::RenderWindow &window) override;
    void pollEvents(const sf::Event &e) override;

    MainMenuScene(const std::shared_ptr<ECS::Registry> &ecs, const std::shared_ptr<AssetManager> &textureManager);
    ~MainMenuScene() override;
};

#endif //R_TYPE_MAINMENUSCENE_HPP
