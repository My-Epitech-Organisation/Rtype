/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextureManager.cpp
*/

#include "TextureManager.hpp"

#include "Logger/Logger.hpp"
#include "Logger/Macros.hpp"

void TextureManager::load(const std::string& id, const std::string& filePath) {
    if (this->_assets.contains(id)) return;
    auto texture = std::make_unique<sf::Texture>();

    if (!texture->loadFromFile(filePath)) {
        LOG_ERROR("Unable to open texture: " << filePath);
        throw std::runtime_error("Error while loading texture: " + filePath);
    }

    this->_assets[id] = std::move(texture);
    LOG_DEBUG("Texture loaded with ID: " << id);
}

void TextureManager::load(const std::string& id, unsigned char* fileData,
                          unsigned int fileDataSize) {
    if (this->_assets.contains(id)) return;
    auto texture = std::make_unique<sf::Texture>();

    if (!texture->loadFromMemory(fileData, fileDataSize)) {
        LOG_ERROR("Unable to load texture from memory: " << id);
        throw std::runtime_error("Error while loading texture: " + id);
    }

    this->_assets[id] = std::move(texture);
    LOG_DEBUG("Texture loaded from memory with ID: " << id);
}

sf::Texture& TextureManager::get(const std::string& id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        LOG_ERROR("Texture not found: " << id);
        throw std::out_of_range("Texture not found: " + id);
    }

    return *it->second;
}

bool TextureManager::isLoaded(const std::string& id) const {
    return this->_assets.find(id) != this->_assets.end();
}

bool TextureManager::unload(const std::string& id) {
    auto it = this->_assets.find(id);
    if (it == this->_assets.end()) {
        LOG_DEBUG("Texture not found for unloading: " << id);
        return false;
    }
    this->_assets.erase(it);
    LOG_DEBUG("Texture unloaded: " << id);
    return true;
}

void TextureManager::unloadAll() {
    std::size_t count = this->_assets.size();
    this->_assets.clear();
    LOG_DEBUG("All textures unloaded (" << count << " textures)");
}

std::size_t TextureManager::size() const { return this->_assets.size(); }
