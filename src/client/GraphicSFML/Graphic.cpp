/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Graphic.cpp
*/

#include "Graphic.hpp"
#include "SFML/Window/Event.hpp"
#include "./SFML/Graphics.hpp"
#include "../assets/r-typesheet42.h"
#include <optional>
#include <thread>
#include <chrono>

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
    _window(sf::VideoMode({800, 600}), "R-TYPE window SFML"),
    _vesselTexture(r_typesheet42_gif, r_typesheet42_gif_len),
    _vessel(this->_vesselTexture)
{
    this->_vessel.setTextureRect(sf::IntRect({0, 0}, {34, 20}));
    this->_vessel.setScale({4.0f, 4.0f});
    this->_vessel.setPosition({400.0f, 300.0f});
}
