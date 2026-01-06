/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.cpp
*/
#include "AssetManager.hpp"

AssetManager::AssetManager(
    const rtype::game::config::RTypeGameConfig& configGameAssets, std::shared_ptr<rtype::display::IDisplay> display) {
    this->configGameAssets = configGameAssets;
    std::cout << "AssetManager: Initializing Asset Managers..." << std::endl;
    this->textureManager = std::make_shared<TextureManager>(display.get());
    this->fontManager = std::make_shared<FontManager>(display.get());
    this->soundManager = std::make_shared<SoundManager>(display.get());
    this->audioManager = std::make_shared<AudioManager>(display.get());
    std::cout << "AssetManager: Asset Managers initialized." << std::endl;
}
