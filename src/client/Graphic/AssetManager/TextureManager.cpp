/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.cpp
*/

#include "TextureManager.hpp"

void TextureManager::load(const std::string &id, const std::string &filePath)  {
    auto texture = std::make_unique<sf::Texture>();

    if (!texture->loadFromFile(filePath)) {
        std::cerr << "Erreur: Impossible de charger la texture: " << filePath << std::endl;
        throw std::runtime_error("Erreur de chargement de texture: " + filePath);
    }

    this->_assets[id] = std::move(texture);
    std::cout << "Texture chargée et stockée sous l'ID: " << id << std::endl;
}

void TextureManager::load(const std::string &id, unsigned char *fileData, unsigned int fileDataSize)  {
    auto texture = std::make_unique<sf::Texture>();

    if (!texture->loadFromMemory(fileData, fileDataSize)) {
        std::cerr << "Erreur: Impossible de charger la texture: " << id << std::endl;
        throw std::runtime_error("Erreur de chargement de texture: " + id);
    }

    this->_assets[id] = std::move(texture);
    std::cout << "Texture chargée et stockée sous l'ID: " << id << std::endl;
}

sf::Texture & TextureManager::get(const std::string &id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        std::cerr << "Erreur: Texture ID non trouvée: " << id << std::endl;
        throw std::out_of_range("Texture non trouvée: " + id);
    }

    return *it->second;
}
