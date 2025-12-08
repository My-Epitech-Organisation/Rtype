/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AudioManager.cpp
*/

#include "AudioManager.hpp"

void AudioManager::load(const std::string& id, const std::string& filePath) {
    auto music = std::make_shared<sf::Music>();

    if (!music->openFromFile(filePath)) {
        std::cerr << "Error unable to open music: " << filePath << std::endl;
        throw std::runtime_error("Error while loading music: " + filePath);
    }

    this->_assets[id] = std::move(music);
    std::cout << "Audio saved with ID: " << id << std::endl;
}

void AudioManager::load(const std::string& id, unsigned char* fileData,
                       unsigned int fileDataSize) {
    auto music = std::make_shared<sf::Music>();

    if (!music->openFromMemory(fileData, fileDataSize)) {
        std::cerr << "Error unable to load music: " << id << std::endl;
        throw std::runtime_error("Error while loading music: " + id);
    }

    this->_assets[id] = std::move(music);
    std::cout << "Audio saved with ID: " << id << std::endl;
}

std::shared_ptr<sf::Music> AudioManager::get(const std::string& id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        std::cerr << "Error font not found: " << id << std::endl;
        throw std::out_of_range("Error font not found: " + id);
    }

    return it->second;
}
