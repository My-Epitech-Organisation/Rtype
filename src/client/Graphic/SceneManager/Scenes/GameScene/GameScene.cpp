/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.cpp
*/

#include "GameScene.hpp"

#include <iostream>
#include <memory>
#include <utility>

void GameScene::update() {
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
    std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem)
    : AScene(ecs, textureManager, window),
      _keybinds(std::move(keybinds)),
      _networkClient(std::move(networkClient)),
      _networkSystem(std::move(networkSystem)),
      _gameScene(std::move(gameScene)) {
    // Initialize game-specific entities
    if (_gameScene) {
        this->_listEntity = _gameScene->initialize();
    }
}

