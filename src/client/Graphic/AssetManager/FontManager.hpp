/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** FontManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_FONTMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_FONTMANAGER_HPP_

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "AAssetManager.hpp"
#include "rtype/display/IDisplay.hpp"

class FontManager : public AAssetManager<::rtype::display::IFont> {
   public:
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    void load(const std::string& id, const std::string& filePath) override;

    explicit FontManager(::rtype::display::IDisplay* display)
        : AAssetManager<::rtype::display::IFont>(display, "Font") {}
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_FONTMANAGER_HPP_
