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

#include "../../games/rtype/client/Systems/BoxingSystem.hpp"
#include "../../games/rtype/client/Systems/ButtonUpdateSystem.hpp"
#include "../../games/rtype/client/Systems/EventSystem.hpp"
#include "../../games/rtype/client/Systems/MovementSystem.hpp"
#include "../../games/rtype/client/Systems/ParallaxScrolling.hpp"
#include "../../games/rtype/client/Systems/RenderSystem.hpp"
#include "../../games/rtype/client/Systems/ResetTriggersSystem.hpp"
#include "AssetManager/AssetManager.hpp"
#include "SceneManager/SceneException.hpp"

void Graphic::_pollEvents() {
    while (const std::optional event = this->_window->pollEvent()) {
        if (!event) return;
        if (event->is<sf::Event::Closed>()) {
            this->_window->close();
        }
        EventSystem::processEvents(this->_registry, *event, this->_window);
        this->_sceneManager->pollEvents(*event);
    }
}

void Graphic::_update() {
    ButtonUpdateSystem::update(this->_registry, this->_window);
    float dt = this->_mainClock.getElapsedTime().asSeconds();
    this->_mainClock.restart();
    float scrollSpeed = 50.f;
    sf::Vector2f center = this->_view.getCenter();
    float newX = center.x + (scrollSpeed * dt);
    this->_view.setCenter({newX, center.y});

    ParallaxScrolling::update(this->_registry, this->_view);
    MovementSystem::update(this->_registry, dt);
    this->_sceneManager->update();
}

void Graphic::_display() {
    this->_window->clear();
    RenderSystem::draw(this->_registry, this->_window);
    BoxingSystem::draw(this->_registry, this->_window);
    this->_sceneManager->draw();
    this->_window->display();
}

void Graphic::loop() {
    while (this->_window->isOpen()) {
        ResetTriggersSystem::update(this->_registry);
        this->_pollEvents();
        this->_update();
        this->_display();
    }
}

Graphic::Graphic(std::shared_ptr<ECS::Registry> registry)
    : _registry(registry),
      _view(sf::FloatRect({0, 0}, {WINDOW_WIDTH, WINDOW_HEIGHT})) {
    this->_keybinds = std::make_shared<KeyboardActions>();
    this->_window = std::make_shared<sf::RenderWindow>(
        sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "R-Type - Epitech 2025");
    this->_window->setView(this->_view);
    this->_assetsManager = std::make_shared<AssetManager>();
    this->_sceneManager = std::make_unique<SceneManager>(
        registry, this->_assetsManager, this->_window, this->_keybinds);
    this->_mainClock.start();
}
