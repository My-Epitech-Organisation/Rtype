/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SoundManager.cpp
*/

#include "SoundManager.hpp"

#include "Macros.hpp"

void SoundManager::load(const std::string& id, const std::string& filePath) {
    if (this->_assets.contains(id)) return;
    auto sound = std::make_shared<sf::SoundBuffer>();

    if (!sound->loadFromFile(filePath)) {
        LOG_ERROR("Error unable to open sound: " + filePath);
        throw std::runtime_error("Error while loading sound: " + filePath);
    }

    this->_assets[id] = std::move(sound);
    LOG_INFO("Sound saved with ID: " + id);
}

std::shared_ptr<sf::SoundBuffer> SoundManager::get(const std::string& id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        LOG_ERROR("Error sound not found: " + id);
        throw std::out_of_range("Error sound not found: " + id);
    }

    return it->second;
}
