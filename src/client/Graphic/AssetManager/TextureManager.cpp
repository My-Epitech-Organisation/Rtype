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
    if (!_display) return;
    _display->loadTexture(id, filePath);
    LOG_DEBUG_CAT(::rtype::LogCategory::Graphics,
                  "Texture loaded with ID: " << id);
    auto texture = _display->getTexture(id);

    if (!texture) {
        LOG_ERROR_CAT(::rtype::LogCategory::Graphics,
                      "Unable to open texture: " << filePath);
        throw std::runtime_error("Error while loading texture: " + filePath);
    }

    this->_assets[id] = texture;
}
