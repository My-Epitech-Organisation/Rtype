/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** GameScene.cpp
*/

#include "GameScene.hpp"

#include "EntityFactory/EntityFactory.hpp"

void GameScene::update() {
}

void GameScene::render(const std::shared_ptr<sf::RenderWindow> &window) {
}

void GameScene::pollEvents(const sf::Event &e) {
}

GameScene::GameScene(
    const std::shared_ptr<ECS::Registry> &ecs,
    const std::shared_ptr<AssetManager> &textureManager,
    const std::shared_ptr<sf::RenderWindow> &window,
    std::function<void(const SceneManager::Scene &)> switchToScene) : AScene(ecs, textureManager, window)
{
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, ""));
}
