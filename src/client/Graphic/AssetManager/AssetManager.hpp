/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_ASSETMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_ASSETMANAGER_HPP_
#include "AudioManager.hpp"
#include "Config/Parser/RTypeConfigParser.hpp"
#include "FontManager.hpp"
#include "SoundManager.hpp"
#include "TextureManager.hpp"

class AssetManager {
   public:
    rtype::game::config::RTypeGameConfig configGameAssets;
    std::shared_ptr<TextureManager> textureManager =
        std::make_shared<TextureManager>();
    std::shared_ptr<FontManager> fontManager = std::make_shared<FontManager>();
    std::shared_ptr<AudioManager> audioManager =
        std::make_shared<AudioManager>();
    std::shared_ptr<SoundManager> soundManager =
        std::make_shared<SoundManager>();
    explicit AssetManager(
        const rtype::game::config::RTypeGameConfig& configGameAssets);
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_ASSETMANAGER_HPP_
