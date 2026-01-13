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

    _display->loadFont(id, filePath);
    auto font = _display->getFont(id);

    if (!font) {
        LOG_ERROR_CAT(::rtype::LogCategory::Graphics,
                      "Unable to open font: " << filePath);
        throw std::runtime_error("Error while loading font: " + filePath);
    }

    this->_assets[id] = font;
    LOG_DEBUG_CAT(::rtype::LogCategory::Graphics,
                  "Font loaded with ID: " << id);
}
