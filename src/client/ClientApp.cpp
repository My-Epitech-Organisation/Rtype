/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ClientApp.cpp
*/

#include "ClientApp.hpp"

ClientApp::ClientApp(const std::shared_ptr<ECS::Registry>& registry)
    : _graphic(registry) {
    this->_graphic.loop();
}
