/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.cpp
*/

#include "GameScene.hpp"

#include <memory>
#include <utility>

#include "Accessibility.hpp"
#include "Components/TagComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"
#include "Graphic.hpp"
#include "Logger/Macros.hpp"
#include "games/rtype/client/GraphicsConstants.hpp"
#include "games/rtype/client/PauseState.hpp"

void GameScene::update(float dt) {
    if (_gameScene) {
        _gameScene->update();
    }
}

void GameScene::render(std::shared_ptr<rtype::display::IDisplay> window) {
    if (_gameScene) {
        _gameScene->render(*window);
    }
}

void GameScene::pollEvents(const rtype::display::Event& e) {
    if (_gameScene) {
        _gameScene->pollEvents(e);
    }
}

GameScene::GameScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> textureManager,
    std::shared_ptr<rtype::display::IDisplay> window,
    std::shared_ptr<KeyboardActions> keybinds,
    std::function<void(const std::string&)> setBackground,
    std::function<void(const SceneManager::Scene&)> switchToScene,
    std::unique_ptr<IGameScene> gameScene,
    std::shared_ptr<rtype::client::NetworkClient> networkClient,
    std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem,
    std::shared_ptr<AudioLib> audio)
    : AScene(ecs, textureManager, window, audio),
      _keybinds(std::move(keybinds)),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)),
      _gameScene(std::move(gameScene)) {
    LOG_DEBUG_CAT(::rtype::LogCategory::UI, "[GameScene] Constructor started");

    if (_gameScene) {
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[GameScene] Calling initialize on game scene");
        this->_listEntity = _gameScene->initialize();
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[GameScene] Game scene initialized, entities created: "
                          << _listEntity.size());
    }
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[GameScene] Loading game textures");
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[GameScene] Loading bdos_enemy_normal from: "
                      << this->_assetsManager->configGameAssets.assets.textures
                             .EnemyNormal);
    this->_assetsManager->textureManager->load(
        "bdos_enemy_normal",
        this->_assetsManager->configGameAssets.assets.textures.EnemyNormal);
    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[GameScene] Loading projectile_player_laser from: "
                      << this->_assetsManager->configGameAssets.assets.textures
                             .missileLaser);
    this->_assetsManager->textureManager->load(
        "projectile_player_laser",
        this->_assetsManager->configGameAssets.assets.textures.missileLaser);
    LOG_DEBUG_CAT(::rtype::LogCategory::UI, "[GameScene] Game textures loaded");
    LOG_DEBUG_CAT(::rtype::LogCategory::UI, "[GameScene] Setting up audio");
    if (!this->_audio) {
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[GameScene] No audio library available");
        return;
    }
    if (this->_assetsManager && this->_assetsManager->audioManager) {
        this->_assetsManager->audioManager->load(
            "main_game_music",
            this->_assetsManager->configGameAssets.assets.music.game);
        auto bgMusic =
            this->_assetsManager->audioManager->get("main_game_music");
        if (bgMusic) {
            LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                          "[GameScene] Playing game music");
            this->_audio->loadMusic(bgMusic);
            this->_audio->setLoop(true);
            this->_audio->play();
        }
    }

    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[GameScene] Constructor completed successfully");
}

GameScene::~GameScene() {
    LOG_DEBUG_CAT(::rtype::LogCategory::UI, "[GameScene] Destructor called");

    if (_networkClient) {
        _networkClient->onLevelAnnounce(nullptr);
        _networkClient->onGameStart(nullptr);
        _networkClient->onGameOver(nullptr);
        _networkClient->onBandwidthModeChanged(nullptr);
    }

    if (_networkSystem) {
        _networkSystem->onLocalPlayerAssigned(nullptr);
        _networkSystem->onHealthUpdate(nullptr);
        _networkSystem->onDisconnect(nullptr);
        _networkSystem->setEntityFactory({});
        _networkSystem->reset();
    }

    if (_networkClient && _networkClient->isConnected()) {
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[GameScene] Disconnecting from server");
        _networkClient->disconnect();
    }

    std::vector<ECS::Entity> gameEntitiesToDestroy;
    this->_registry->view<rtype::games::rtype::client::GameTag>().each(
        [&](ECS::Entity entity, rtype::games::rtype::client::GameTag& /*tag*/) {
            gameEntitiesToDestroy.push_back(entity);
        });

    for (const auto& entity : gameEntitiesToDestroy) {
        this->_registry->killEntity(entity);
    }

    LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                  "[GameScene] Cleaning up pause menu entities");
    std::vector<ECS::Entity> pauseEntitiesToDestroy;
    this->_registry->view<rtype::games::rtype::client::PauseMenuTag>().each(
        [&](ECS::Entity entity,
            rtype::games::rtype::client::PauseMenuTag& /*tag*/) {
            pauseEntitiesToDestroy.push_back(entity);
        });

    for (const auto& entity : pauseEntitiesToDestroy) {
        this->_registry->killEntity(entity);
    }

    if (this->_registry
            ->hasSingleton<rtype::games::rtype::client::PauseState>()) {
        LOG_DEBUG_CAT(::rtype::LogCategory::UI,
                      "[GameScene] Removing PauseState singleton");
        this->_registry
            ->removeSingleton<rtype::games::rtype::client::PauseState>();
    }

    if (this->_audio) {
        LOG_DEBUG_CAT(::rtype::LogCategory::UI, "[GameScene] Pausing music");
        this->_audio->pauseMusic();
    }
    LOG_DEBUG_CAT(::rtype::LogCategory::UI, "[GameScene] Destructor completed");
}
