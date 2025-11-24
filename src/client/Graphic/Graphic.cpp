/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Graphic.cpp
*/

#include "Graphic.hpp"
#include <optional>

void RTypeClient::Graphic::pollEvents() {
    while (const std::optional event = this->_window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            this->_window.close();
    }
}

void RTypeClient::Graphic::display() {
    this->_window.clear();

    this->_window.draw(this->_vessel);
    this->_window.display();
}

void RTypeClient::Graphic::loop() {
    while (this->_appRunning && this->_window.isOpen()) {
        this->pollEvents();
        this->display();
    }
}

RTypeClient::Graphic::~Graphic() {
    this->_appRunning = false;
}

RTypeClient::Graphic::Graphic() :
    _window(sf::VideoMode({800, 600}), "R-TYPE window SFML")
{
    this->_vesselTexture = sf::Texture("assets/r-typesheet42.gif");
    this->_vessel.setTexture(this->_vesselTexture);
    this->loop();
}
