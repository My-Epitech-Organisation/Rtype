/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.hpp
*/

#ifndef R_TYPE_ASSETMANAGER_HPP
#define R_TYPE_ASSETMANAGER_HPP
#include "FontManager.hpp"
#include "TextureManager.hpp"


class AssetManager {
public:
    std::shared_ptr<TextureManager> textureManager = std::make_shared<TextureManager>();
    std::shared_ptr<FontManager> fontManager = std::make_shared<FontManager>();
    AssetManager() = default;
};


#endif //R_TYPE_ASSETMANAGER_HPP