/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Graphic.cpp
*/

#include <optional>
#include <utility>
#include <iostream>
#include "Graphic.hpp"

void Graphic::_handleKeyReleasedEvent(const std::optional<sf::Event> &event) {
    if (event->getIf<sf::Event::KeyReleased>()->code == this->_keybinds.getKeyBinding(GameAction::PAUSE)) {
        if (this->_sceneManager == SceneManager::IN_GAME) {
            this->_sceneManager.setCurrentScene(SceneManager::PAUSE_MENU);
        } else if (this->_sceneManager == SceneManager::PAUSE_MENU) {
            this->_sceneManager.setCurrentScene(SceneManager::IN_GAME);
        }
        std::cout << this->_sceneManager << std::endl;
    }
}

void Graphic::_poolEvents() {
    while (const std::optional event = this->_window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            this->_window.close();
        if (event->is<sf::Event::KeyReleased>()) {
            std::cout << "Key Released: " << static_cast<int>(event->getIf<sf::Event::KeyReleased>()->code) << std::endl;
            this->_handleKeyReleasedEvent(event);
        }
    }
}

void Graphic::_update() {
    this->_sceneManager.updateScene();
}

void Graphic::_display() {
    this->_window.clear();
    this->_sceneManager.renderScene();
    this->_window.display();
}

void Graphic::loop() {
    while (this->_window.isOpen()) {
        this->_poolEvents();
        this->_update();
        this->_display();
    }
}

Graphic::Graphic(std::shared_ptr<ECS::Registry> registry) :
    _registry(std::move(registry)),
    _window(sf::VideoMode({800, 600}), "R-Type - Epitech 2025")
{
    this->_mainClock.start();
    this->loop();
}
