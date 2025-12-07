/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_ASSETMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_ASSETMANAGER_HPP_
#include "FontManager.hpp"
#include "TextureManager.hpp"

class AssetManager {
   public:
    std::shared_ptr<TextureManager> textureManager =
        std::make_shared<TextureManager>();
    std::shared_ptr<FontManager> fontManager = std::make_shared<FontManager>();
    AssetManager();
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_ASSETMANAGER_HPP_
