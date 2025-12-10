/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeGameScene.cpp
*/

#include "RtypeGameScene.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "AllComponents.hpp"
#include "Components/CountdownComponent.hpp"
#include "Components/TextComponent.hpp"
#include "Components/ZIndexComponent.hpp"
#include "Graphic/EntityFactory/EntityFactory.hpp"
#include "GraphicsConstants.hpp"
#include "Logger/Macros.hpp"
#include "RtypeEntityFactory.hpp"
#include "RtypeInputHandler.hpp"
#include "RtypePauseMenu.hpp"

namespace rtype::games::rtype::client {

RtypeGameScene::RtypeGameScene(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetsManager,
    std::shared_ptr<sf::RenderWindow> window,
    std::shared_ptr<KeyboardActions> keybinds,
    std::function<void(const SceneManager::Scene&)> switchToScene,
    std::shared_ptr<::rtype::client::NetworkClient> networkClient,
    std::shared_ptr<::rtype::client::ClientNetworkSystem> networkSystem,
    std::shared_ptr<AudioLib> audioLib)
    : AGameScene(std::move(registry), std::move(assetsManager),
                 std::move(window), std::move(keybinds),
                 std::move(switchToScene), std::move(networkClient),
                 std::move(networkSystem)) {}

std::vector<ECS::Entity> RtypeGameScene::initialize() {
    std::vector<ECS::Entity> entities;

    auto bgEntities = EntityFactory::createBackground(this->_registry,
                                                      this->_assetsManager, "");
    entities.insert(entities.end(), bgEntities.begin(), bgEntities.end());
    if (_networkSystem) {
        setupEntityFactory();
        setupLocalPlayerCallback();
        _networkSystem->onHealthUpdate(
            [this](const ::rtype::client::EntityHealthEvent& event) {
                handleHealthUpdate(event);
            });
    }
    setupHud();
    auto pauseEntities = RtypePauseMenu::createPauseMenu(
        _registry, _assetsManager, _switchToScene);
    entities.insert(entities.end(), pauseEntities.begin(), pauseEntities.end());

    return entities;
}

void RtypeGameScene::update() {
    if (_networkSystem) {
        _networkSystem->update();
    }

    std::uint8_t inputMask = getInputMask();

    if (_networkSystem && _networkSystem->isConnected()) {
        if (inputMask != _lastInputMask) {
            _networkSystem->sendInput(inputMask);
            _lastInputMask = inputMask;
        }
    }
}

void RtypeGameScene::render(std::shared_ptr<sf::RenderWindow> window) {
    // R-Type specific rendering if needed
}

void RtypeGameScene::pollEvents(const sf::Event& event) {
    if (event.is<sf::Event::KeyReleased>()) {
        RtypeInputHandler::handleKeyReleasedEvent(event, _keybinds, _registry);
    }
}

std::uint8_t RtypeGameScene::getInputMask() const {
    return RtypeInputHandler::getInputMask(_keybinds);
}

void RtypeGameScene::setupEntityFactory() {
    LOG_DEBUG("[RtypeGameScene] Setting up entityFactory");
    _networkSystem->setEntityFactory(
        RtypeEntityFactory::createNetworkEntityFactory(_registry,
                                                       _assetsManager));
}

void RtypeGameScene::setupLocalPlayerCallback() {
    auto registry = this->_registry;
    _networkSystem->onLocalPlayerAssigned(
        [this, registry](std::uint32_t userId, ECS::Entity entity) {
            if (registry->isAlive(entity)) {
                registry->emplaceComponent<ControllableTag>(entity);
                LOG_DEBUG("[RtypeGameScene] Local player entity assigned");
            }
            _localPlayerEntity = entity;
            _localPlayerId = userId;
        });
}

void RtypeGameScene::setupHud() {
    auto lives = EntityFactory::createStaticText(
        _registry, _assetsManager, "Lives: --", "title_font",
        sf::Vector2f{20.f, 20.f}, 28.f);
    _registry->emplaceComponent<ZIndex>(lives, GraphicsConfig::ZINDEX_UI);
    _livesTextEntity = lives;
}

void RtypeGameScene::updateLivesDisplay(int current, int max) {
    if (!_livesTextEntity.has_value()) return;
    auto ent = *_livesTextEntity;
    if (!_registry->isAlive(ent)) {
        _livesTextEntity.reset();
        return;
    }

    if (_registry->hasComponent<Text>(ent)) {
        auto& text = _registry->getComponent<Text>(ent);
        text.textContent =
            "Lives: " + std::to_string(current) + "/" + std::to_string(max);
    }
    _lastKnownLives = current;
    _lastKnownMaxLives = max;
}

void RtypeGameScene::handleHealthUpdate(
    const ::rtype::client::EntityHealthEvent& event) {
    if (!_localPlayerId.has_value()) {
        return;
    }
    if (event.entityId != *_localPlayerId) {
        return;
    }
    updateLivesDisplay(event.current, event.max);
}

}  // namespace rtype::games::rtype::client
