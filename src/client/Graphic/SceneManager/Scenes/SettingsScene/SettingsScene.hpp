/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SettingsScene.hpp
*/

#ifndef R_TYPE_SETTINGSSCENE_HPP
#define R_TYPE_SETTINGSSCENE_HPP
#include <vector>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../AScene.hpp"
#include "SceneManager/SceneManager.hpp"

class SettingsScene : public AScene {
private:
    std::vector<ECS::Entity> _listEntity;

public:
    void update() override;
    void render(sf::RenderWindow& window) override;
    void pollEvents(const sf::Event& e) override;

    SettingsScene(const std::shared_ptr<ECS::Registry>& ecs,
                  const std::shared_ptr<AssetManager>& textureManager,
                  std::function<void(const SceneManager::Scene&)> switchToScene,
                  sf::RenderWindow& window);
    ~SettingsScene() override;
};


#endif //R_TYPE_SETTINGSSCENE_HPP