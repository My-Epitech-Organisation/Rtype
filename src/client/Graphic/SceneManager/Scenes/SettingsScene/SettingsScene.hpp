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
#include "Graphic/KeyboardActions.hpp"
#include "GameAction.hpp"
#include <map>
#include <optional>

class SettingsScene : public AScene {
private:
    std::vector<ECS::Entity> _listEntity;
    KeyboardActions& _keybinds;
    std::optional<GameAction> _actionToRebind;
    std::map<GameAction, ECS::Entity> _actionButtons;

    void _initKeybindSection();
    void _initAudioSection();
    void _initWindowSection();

public:
    void update() override;
    void render(sf::RenderWindow& window) override;
    void pollEvents(const sf::Event& e) override;

    SettingsScene(const std::shared_ptr<ECS::Registry>& ecs,
                  const std::shared_ptr<AssetManager>& textureManager,
                  std::function<void(const SceneManager::Scene&)> switchToScene,
                  sf::RenderWindow& window,
                  KeyboardActions& keybinds);
    ~SettingsScene() override;
};


#endif //R_TYPE_SETTINGSSCENE_HPP