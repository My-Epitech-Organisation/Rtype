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
    std::shared_ptr<TextureManager> textureManager;
    std::shared_ptr<FontManager> fontManager;
    std::shared_ptr<AudioManager> audioManager;
    std::shared_ptr<SoundManager> soundManager;
    explicit AssetManager(
        const rtype::game::config::RTypeGameConfig& configGameAssets,
        std::shared_ptr<rtype::display::IDisplay> display);
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_ASSETMANAGER_HPP_
