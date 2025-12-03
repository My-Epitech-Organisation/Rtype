/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** FontManager.cpp
*/

#include "FontManager.hpp"

void FontManager::load(const std::string& id, const std::string& filePath) {
    auto font = std::make_unique<sf::Font>();

    if (!font->openFromFile(filePath)) {
        std::cerr << "Error unable to open font: " << filePath
                  << std::endl;
        throw std::runtime_error("Error while loading font: " + filePath);
    }

    this->_assets[id] = std::move(font);
    std::cout << "Font save with ID: " << id << std::endl;
}

void FontManager::load(const std::string& id, unsigned char* fileData,
                       unsigned int fileDataSize) {
    auto font = std::make_unique<sf::Font>();

    if (!font->openFromMemory(fileData, fileDataSize)) {
        std::cerr << "Error unable to load font: " << id
                  << std::endl;
        throw std::runtime_error("Error while loading font: " + id);
    }

    this->_assets[id] = std::move(font);
    std::cout << "Font save with ID: " << id << std::endl;
}

sf::Font& FontManager::get(const std::string& id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        std::cerr << "Error font not found: " << id << std::endl;
        throw std::out_of_range("Error font not found: " + id);
    }

    return *it->second;
}
