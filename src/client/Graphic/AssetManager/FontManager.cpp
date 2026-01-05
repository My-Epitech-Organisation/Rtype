/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** FontManager.cpp
*/

#include "FontManager.hpp"

#include "Logger/Logger.hpp"
#include "Logger/Macros.hpp"

void FontManager::load(const std::string& id, const std::string& filePath) {
    if (this->_assets.contains(id)) return;
    auto font = std::make_shared<sf::Font>();

    if (!font->openFromFile(filePath)) {
        LOG_ERROR_CAT(::rtype::LogCategory::Graphics,
                      "Unable to open font: " << filePath);
        throw std::runtime_error("Error while loading font: " + filePath);
    }

    this->_assets[id] = std::move(font);
    LOG_DEBUG_CAT(::rtype::LogCategory::Graphics,
                  "Font loaded with ID: " << id);
}

std::shared_ptr<sf::Font> FontManager::get(const std::string& id) {
    auto it = this->_assets.find(id);

    if (it == this->_assets.end()) {
        LOG_ERROR_CAT(::rtype::LogCategory::Graphics, "Font not found: " << id);
        throw std::out_of_range("Font not found: " + id);
    }

    return it->second;
}

bool FontManager::isLoaded(const std::string& id) const {
    return this->_assets.find(id) != this->_assets.end();
}

bool FontManager::unload(const std::string& id) {
    auto it = this->_assets.find(id);
    if (it == this->_assets.end()) {
        LOG_DEBUG_CAT(::rtype::LogCategory::Graphics,
                      "Font not found for unloading: " << id);
        return false;
    }
    this->_assets.erase(it);
    LOG_DEBUG_CAT(::rtype::LogCategory::Graphics, "Font unloaded: " << id);
    return true;
}

void FontManager::unloadAll() {
    std::size_t count = this->_assets.size();
    this->_assets.clear();
    LOG_DEBUG_CAT(::rtype::LogCategory::Graphics,
                  "All fonts unloaded (" << count << " fonts)");
}

std::size_t FontManager::size() const { return this->_assets.size(); }
