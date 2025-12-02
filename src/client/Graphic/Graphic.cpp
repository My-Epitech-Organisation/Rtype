/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Graphic.cpp
*/

#include "Graphic.hpp"

#include <iostream>
#include <optional>
#include <utility>

#include "AssetManager/AssetManager.hpp"
#include "System/ButtonUpdateSystem.hpp"
#include "System/EventSystem.hpp"
#include "System/MovementSystem.hpp"
#include "System/ParallaxScrolling.hpp"
#include "System/RenderSystem.hpp"

void Graphic::_handleKeyReleasedEvent(const std::optional<sf::Event>& event) {
    if (!event) {
        return;
    }
    const auto& key = event->getIf<sf::Event::KeyReleased>();
    if (key && key->code == this->_keybinds.getKeyBinding(GameAction::PAUSE)) {
        if (*this->_sceneManager == SceneManager::IN_GAME) {
            this->_sceneManager->setCurrentScene(SceneManager::PAUSE_MENU);
        } else if (*this->_sceneManager == SceneManager::PAUSE_MENU) {
            this->_sceneManager->setCurrentScene(SceneManager::IN_GAME);
        }
    }
}

void Graphic::_pollEvents() {
    while (const std::optional event = this->_window.pollEvent()) {
        if (!event) return;
        if (event->is<sf::Event::Closed>()) {
            this->_window.close();
        }
        if (event->is<sf::Event::KeyReleased>()) {
            this->_handleKeyReleasedEvent(event);
        }
        EventSystem::processEvents(this->_registry, *event);
        this->_sceneManager->pollEvents(*event);
    }
}

void Graphic::_update() {
    ButtonUpdateSystem::update(this->_registry, this->_window);
    float dt = this->_mainClock.getElapsedTime().asSeconds();
    this->_mainClock.restart();
    float scrollSpeed = 30.f;
    sf::Vector2f center = this->_view.getCenter();
    float newX = center.x + (scrollSpeed * dt);
    this->_view.setCenter({newX, center.y});

    ParallaxScrolling::update(this->_registry, this->_view);
    MovementSystem::update(this->_registry, dt);
    this->_sceneManager->update();
}

void Graphic::_display() {
    this->_window.clear();
    RenderSystem::draw(this->_registry, this->_window);
    this->_sceneManager->draw(this->_window);
    this->_window.display();
}

void Graphic::loop() {
    while (this->_window.isOpen()) {
        this->_pollEvents();
        this->_update();
        this->_display();
    }
}

Graphic::Graphic(const std::shared_ptr<ECS::Registry>& registry)
    : _registry(std::move(registry)),
      _keybinds(),
      _window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
              "R-Type - Epitech 2025"),
      _view(sf::FloatRect({0, 0}, {1920, 1080})) {
    this->_window.setView(this->_view);
    this->_assetsManager = std::make_shared<AssetManager>();
    this->_sceneManager = std::make_unique<SceneManager>(
        registry, this->_assetsManager, this->_window);
    this->_mainClock.start();
}
