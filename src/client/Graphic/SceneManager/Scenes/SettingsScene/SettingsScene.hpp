/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SettingsScene.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_SETTINGSSCENE_SETTINGSSCENE_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_SETTINGSSCENE_SETTINGSSCENE_HPP_
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "../AScene.hpp"
#include "GameAction.hpp"
#include "Graphic/Accessibility.hpp"
#include "Graphic/KeyboardActions.hpp"
#include "SceneManager/SceneManager.hpp"
#include "SettingsSceneUtils.hpp"

class SettingsScene : public AScene {
   private:
    std::shared_ptr<KeyboardActions> _keybinds;
    std::optional<GameAction> _actionToRebind;
    std::map<GameAction, ECS::Entity> _actionButtons;
    std::optional<ECS::Entity> _intensityLabel;
    ECS::Entity _inputModeLabel;
    std::vector<ECS::Entity> _keybindSectionEntities;

    void _initKeybindSection();
    void _initAudioSection();
    void _initWindowSection();
    void _initAccessibilitySection();
    void _initInputModeSection();
    void _refreshKeybindSection();
    void _setColorMode(ColorBlindMode mode);
    void _adjustColorIntensity(float delta);
    void _refreshIntensityLabel();
    void _refreshInputModeLabel();

   public:
    void update(float dt) override;
    void render(std::shared_ptr<rtype::display::IDisplay> window) override;
    void pollEvents(const rtype::display::Event& e) override;

    SettingsScene(
        std::shared_ptr<ECS::Registry> ecs,
        std::shared_ptr<AssetManager> textureManager,
        std::shared_ptr<rtype::display::IDisplay> window,
        std::shared_ptr<KeyboardActions> keybinds,
        std::shared_ptr<AudioLib> audio,
        std::function<void(const SceneManager::Scene&)> switchToScene);
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_SETTINGSSCENE_SETTINGSSCENE_HPP_
