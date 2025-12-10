/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.cpp
*/

#include "GameScene.hpp"

#include <memory>
#include <utility>

#include "Components/TagComponent.hpp"

void GameScene::update(float dt) {
    if (_gameScene) {
        _gameScene->update();
    }
}

void GameScene::render(std::shared_ptr<sf::RenderWindow> window) {
    if (_gameScene) {
        _gameScene->render(window);
    }
}

void GameScene::pollEvents(const sf::Event& e) {
    if (_gameScene) {
        _gameScene->pollEvents(e);
    }
}

GameScene::GameScene(
    std::shared_ptr<ECS::Registry> ecs,
    std::shared_ptr<AssetManager> textureManager,
    std::shared_ptr<sf::RenderWindow> window,
    std::shared_ptr<KeyboardActions> keybinds,
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
    if (_gameScene) {
        this->_listEntity = _gameScene->initialize();
    }
    this->_assetsManager->textureManager->load(
        "bdos_enemy",
        this->_assetsManager->configGameAssets.assets.textures.Enemy);
    this->_assetsManager->textureManager->load(
        "projectile_player_laser",
        this->_assetsManager->configGameAssets.assets.textures.missileLaser);
    if (this->_audio && this->_assetsManager &&
        this->_assetsManager->audioManager) {
        this->_assetsManager->textureManager->load(
            "bydos_spawn",
            this->_assetsManager->configGameAssets.assets.sfx.enemySpawn);
        this->_assetsManager->textureManager->load(
            "bydos_death",
            this->_assetsManager->configGameAssets.assets.sfx.enemyDeath);
        this->_assetsManager->audioManager->load(
            "main_game_music",
            this->_assetsManager->configGameAssets.assets.music.game);
        auto bgMusic =
            this->_assetsManager->audioManager->get("main_game_music");
        if (bgMusic) {
            this->_audio->loadMusic(bgMusic);
            this->_audio->setLoop(true);
            this->_audio->play();
        }
    }
}

GameScene::~GameScene() {
    this->_registry->view<rtype::games::rtype::client::GameTag>().each(
        [this](ECS::Entity entity,
               rtype::games::rtype::client::GameTag& /*tag*/) {
            this->_registry->killEntity(entity);
        });
    if (this->_audio) {
        this->_audio->pauseMusic();
    }
}
