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

void Graphic::_handleKeyReleasedEvent(const std::optional<sf::Event>& event) {
    if (!event) {
        return;
    }
    const auto& key = event->getIf<sf::Event::KeyReleased>();
    if (key && key->code == this->_keybinds.getKeyBinding(GameAction::PAUSE)) {
        if (this->_sceneManager == SceneManager::IN_GAME) {
            this->_sceneManager.setCurrentScene(SceneManager::PAUSE_MENU);
        } else if (this->_sceneManager == SceneManager::PAUSE_MENU) {
            this->_sceneManager.setCurrentScene(SceneManager::IN_GAME);
        }
    }
}

void Graphic::_pollEvents() {
    while (const std::optional event = this->_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            this->_window.close();
        }
        if (event->is<sf::Event::KeyReleased>()) {
            this->_handleKeyReleasedEvent(event);
        }
    }
}

void Graphic::_update() { this->_sceneManager.updateScene(); }

void Graphic::_display() {
    this->_window.clear();
    this->_sceneManager.renderScene();
    this->_window.display();
}

void Graphic::loop() {
    while (this->_window.isOpen()) {
        this->_pollEvents();
        this->_update();
        this->_display();
    }
}

Graphic::Graphic(std::shared_ptr<ECS::Registry> registry)
    : _registry(std::move(registry)),
      _sceneManager(),
      _keybinds(),
      _window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}),
              "R-Type - Epitech 2025") {
    this->_mainClock.start();
}
