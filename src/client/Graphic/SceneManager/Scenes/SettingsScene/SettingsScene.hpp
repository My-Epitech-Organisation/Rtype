/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SettingsScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_SETTINGSSCENE_SETTINGSSCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_SETTINGSSCENE_SETTINGSSCENE_HPP_
#include <map>
#include <optional>
#include <vector>

#include <SFML/Graphics/RenderWindow.hpp>

#include "../AScene.hpp"
#include "GameAction.hpp"
#include "Graphic/KeyboardActions.hpp"
#include "SceneManager/SceneManager.hpp"
#include "SettingsSceneUtils.hpp"

class SettingsScene : public AScene {
   private:
    std::shared_ptr<KeyboardActions> _keybinds;
    std::optional<GameAction> _actionToRebind;
    std::map<GameAction, ECS::Entity> _actionButtons;

    void _initKeybindSection();
    void _initAudioSection();
    void _initWindowSection();

   public:
    void update() override;
    void render(std::shared_ptr<sf::RenderWindow> window) override;
    void pollEvents(const sf::Event& e) override;

    SettingsScene(std::shared_ptr<ECS::Registry> ecs,
                  std::shared_ptr<AssetManager> textureManager,
                  std::shared_ptr<sf::RenderWindow> window,
                  std::function<void(const SceneManager::Scene&)> switchToScene,
                  std::shared_ptr<KeyboardActions> keybinds);
    ~SettingsScene() override;
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_SETTINGSSCENE_SETTINGSSCENE_HPP_
