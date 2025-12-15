/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Lobby.cpp
*/

#include <utility>

#include "./Lobby.hpp"

#include "SceneException.hpp"
#include "Components/HiddenComponent.hpp"
#include "EntityFactory/EntityFactory.hpp"

void Lobby::update(float dt) {
    if (!this->_networkClient->ping() || !this->_networkClient->isConnected()) {
        this->_switchToScene(SceneManager::Scene::MAIN_MENU);
    }
}

void Lobby::render(std::shared_ptr<sf::RenderWindow> window) {
}

void Lobby::pollEvents(const sf::Event &e) {
}

Lobby::Lobby(std::shared_ptr<ECS::Registry> ecs,
        std::shared_ptr<AssetManager> assetManager,
        std::shared_ptr<sf::RenderWindow> window,
        std::function<void(const SceneManager::Scene&)> switchToScene,
        std::shared_ptr<rtype::client::NetworkClient> networkClient,
        std::shared_ptr<rtype::client::ClientNetworkSystem> networkSystem,
        std::shared_ptr<AudioLib> audioLib
):
    AScene(std::move(ecs), std::move(assetManager), std::move(window), std::move(audioLib)),
    _networkClient(std::move(networkClient)),
    _networkSystem(std::move(networkSystem)),
    _switchToScene(std::move(switchToScene))
{
    this->_listEntity = (EntityFactory::createBackground(
        this->_registry, this->_assetsManager, "Lobby"));

}
