/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** FontManager.cpp
*/

#include "FontManager.hpp"

void FontManager::load(const std::string &id, const std::string &filePath)  {
    auto font = std::make_unique<sf::Font>();

    if (!font->openFromFile(filePath)) {
        std::cerr << "Erreur: Impossible de charger la police: " << filePath << std::endl;
        throw std::runtime_error("Erreur de chargement de police: " + filePath);
    }

    this->_assets[id] = std::move(font);
    std::cout << "Police chargée et stockée sous l'ID: " << id << std::endl;
}

void FontManager::load(const std::string &id, unsigned char *fileData, unsigned int fileDataSize)  {
    auto font = std::make_unique<sf::Font>();

    if (!font->openFromMemory(fileData, fileDataSize)) {
        std::cerr << "Erreur: Impossible de charger la police: " << id << std::endl;
        throw std::runtime_error("Erreur de chargement de police: " + id);
    }

    this->_assets[id] = std::move(font);
    std::cout << "Police chargée et stockée sous l'ID: " << id << std::endl;
}

sf::Font &FontManager::get(const std::string &id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        std::cerr << "Erreur: Police ID non trouvée: " << id << std::endl;
        throw std::out_of_range("Police non trouvée: " + id);
    }

    return *it->second;
}
