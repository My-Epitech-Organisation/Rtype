/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_TEXTUREMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_TEXTUREMANAGER_HPP_

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "AAssetManager.hpp"
#include "rtype/display/IDisplay.hpp"

class TextureManager : public AAssetManager<::rtype::display::ITexture> {
   public:
    TextureManager(rtype::display::IDisplay* display, std::string typeName = "Texture")
        : AAssetManager<::rtype::display::ITexture>(display, std::move(typeName)) {}
    TextureManager(const TextureManager&);
    TextureManager& operator=(const TextureManager&) = delete;

    void load(const std::string& id, const std::string& filePath) override;
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_TEXTUREMANAGER_HPP_
