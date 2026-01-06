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
    LOG_DEBUG("Texture loaded with ID: " << id);
}

std::shared_ptr<::rtype::display::ITexture> TextureManager::get(
    const std::string& id) {
    if (!_display) return nullptr;
    auto texture = _display->getTexture(id);

    if (!texture) {
        LOG_ERROR("Texture not found: " << id);
        throw std::out_of_range("Texture not found: " + id);
    }

    return texture;
}

bool TextureManager::isLoaded(const std::string& id) const {
    if (!_display) return false;
    return _display->getTexture(id) != nullptr;
}
