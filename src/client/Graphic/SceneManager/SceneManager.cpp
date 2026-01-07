/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SceneManager.cpp
*/
#include "SceneManager.hpp"

#include <memory>
#include <utility>

#include "../../GameAction.hpp"
#include "GameScene/RtypeEntityFactory.hpp"
#include "GameScene/RtypeGameScene.hpp"
#include "SceneException.hpp"
#include "Scenes/GameOverScene/GameOverScene.hpp"
#include "Scenes/GameScene/GameScene.hpp"
#include "Scenes/HowToPlayScene/HowToPlayScene.hpp"
#include "Scenes/Lobby/Lobby.hpp"
#include "Scenes/MainMenuScene/MainMenuScene.hpp"
#include "Scenes/SettingsScene/SettingsScene.hpp"

void SceneManager::_applySceneChange() {
    if (this->_nextScene.has_value()) {
        Scene scene = this->_nextScene.value();

        if (this->_currentScene == scene) {
            LOG_DEBUG_CAT(
                ::rtype::LogCategory::UI,
                "[SceneManager] Ignoring scene change - already on scene: "
                    << static_cast<int>(scene));
            this->_nextScene = std::nullopt;
            return;
        }

        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[SceneManager] Applying scene change from "
                          << static_cast<int>(this->_currentScene) << " to "
                          << static_cast<int>(scene));
        this->_currentScene = scene;
        this->_activeScene = this->_sceneList[this->_currentScene]();
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[SceneManager] Scene change applied successfully");

        this->_nextScene = std::nullopt;
    }
}

void SceneManager::setCurrentScene(const Scene scene) {
    if (!this->_sceneList.contains(scene)) {
        throw SceneNotFound();
    }
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[SceneManager] Scene change requested to: "
                      << static_cast<int>(scene));
    if ((scene == IN_GAME || scene == LOBBY) && this->_networkSystem &&
        this->_registry && this->_assetManager) {
        LOG_DEBUG_CAT(
            ::rtype::LogCategory::UI,
            "[SceneManager] Pre-configuring entity factory for IN_GAME scene");
        this->_networkSystem->setEntityFactory(
            rtype::games::rtype::client::RtypeEntityFactory::
                createNetworkEntityFactory(this->_registry,
                                           this->_assetManager));
    }
    this->_nextScene = scene;
}

void SceneManager::pollEvents(const rtype::display::Event& e) {
    this->_applySceneChange();

    if (!this->_activeScene) {
        throw SceneNotInitialized();
    }
    this->_activeScene->pollEvents(e);
}

void SceneManager::update(float dt) {
    this->_applySceneChange();

    if (!this->_activeScene) {
        throw SceneNotInitialized();
    }
    this->_activeScene->update(dt);
}

void SceneManager::draw() {
    this->_applySceneChange();

    if (!this->_activeScene) {
        throw SceneNotInitialized();
    }
    this->_activeScene->render(this->_display);
}

SceneManager::SceneManager(
    std::shared_ptr<ECS::Registry> ecs, std::shared_ptr<AssetManager> texture,
    std::shared_ptr<rtype::display::IDisplay> display,
    std::shared_ptr<KeyboardActions> keybinds,
    std::shared_ptr<rtype::client::NetworkClient> networkClient,
    std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem,
    std::shared_ptr<AudioLib> audioLib)
    : _display(display),
      _keybinds(keybinds),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)),
      _registry(ecs),
      _assetManager(texture),
      _audio(audioLib) {
    this->_switchToScene = [this](const Scene& scene) {
        this->setCurrentScene(scene);
    };
    this->_sceneList.emplace(MAIN_MENU, [ecs, texture, this]() {
        return std::make_unique<MainMenuScene>(
            ecs, texture, this->_display, this->_switchToScene,
            this->_networkClient, this->_networkSystem, this->_audio);
    });
    this->_sceneList.emplace(SETTINGS_MENU, [ecs, texture, this]() {
        return std::make_unique<SettingsScene>(ecs, texture, this->_display,
                                               this->_keybinds, this->_audio,
                                               this->_switchToScene);
    });
    this->_sceneList.emplace(HOW_TO_PLAY, [ecs, texture, this]() {
        return std::make_unique<HowToPlayScene>(ecs, texture, this->_display,
                                                this->_keybinds, this->_audio,
                                                this->_switchToScene);
    });
    this->_sceneList.emplace(LOBBY, [ecs, texture, this]() {
        return std::make_unique<Lobby>(
            ecs, texture, this->_display, this->_switchToScene,
            this->_networkClient, this->_networkSystem, this->_audio);
    });
    this->_sceneList.emplace(GAME_OVER, [ecs, texture, this]() {
        return std::make_unique<GameOverScene>(
            ecs, texture, this->_display, this->_audio, this->_switchToScene);
    });
    this->_sceneList.emplace(IN_GAME, [ecs, texture, this]() {
        if (this->_networkSystem) {
            LOG_DEBUG_CAT(
                ::rtype::LogCategory::UI,
                "[SceneManager] Setting up entity factory before scene "
                "creation");
            this->_networkSystem->setEntityFactory(
                rtype::games::rtype::client::RtypeEntityFactory::
                    createNetworkEntityFactory(ecs, texture));
        }
        auto rtypeGameScene =
            std::make_unique<rtype::games::rtype::client::RtypeGameScene>(
                ecs, texture, this->_display, this->_keybinds,
                this->_switchToScene, this->_networkClient,
                this->_networkSystem, this->_audio);
        return std::make_unique<GameScene>(
            ecs, texture, this->_display, this->_keybinds, this->_switchToScene,
            std::move(rtypeGameScene), this->_networkClient,
            this->_networkSystem, this->_audio);
    });
    this->setCurrentScene(MAIN_MENU);
    this->_applySceneChange();
}
