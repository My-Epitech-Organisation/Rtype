/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.cpp
*/

#include "TextureManager.hpp"

void TextureManager::load(const std::string& id, const std::string& filePath) {
    if (this->_assets.contains(id)) return;
    auto texture = std::make_unique<sf::Texture>();

    if (!texture->loadFromFile(filePath)) {
        std::cerr << "Error unable to open texture: " << filePath << std::endl;
        throw std::runtime_error("Error while loading texture: " + filePath);
    }

    this->_assets[id] = std::move(texture);
    std::cout << "Texture saved with ID: " << id << std::endl;
}

void TextureManager::load(const std::string& id, unsigned char* fileData,
                          unsigned int fileDataSize) {
    if (this->_assets.contains(id)) return;
    auto texture = std::make_unique<sf::Texture>();

    if (!texture->loadFromMemory(fileData, fileDataSize)) {
        std::cerr << "Error unable to load texture: " << id << std::endl;
        throw std::runtime_error("Error while loading texture: " + id);
    }

    this->_assets[id] = std::move(texture);
    std::cout << "Texture saved with ID:: " << id << std::endl;
}

sf::Texture& TextureManager::get(const std::string& id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        std::cerr << "Error Texture not found: " << id << std::endl;
        throw std::out_of_range("Error Texture not found: " + id);
    }

    return *it->second;
}
