/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.cpp
*/
#include "AssetManager.hpp"

AssetManager::AssetManager(
    const rtype::game::config::RTypeGameConfig& configGameAssets) {
    this->configGameAssets = configGameAssets;
    this->fontManager->load("title_font",
                            configGameAssets.assets.fonts.MainFont);
    this->textureManager->load("bg_menu",
                               configGameAssets.assets.textures.background);
    this->textureManager->load("bg_planet_1",
                               configGameAssets.assets.textures.planet1);
    this->textureManager->load("bg_planet_2",
                               configGameAssets.assets.textures.planet2);
    this->textureManager->load("bg_planet_3",
                               configGameAssets.assets.textures.planet3);
    this->textureManager->load("astro_vessel",
                               configGameAssets.assets.textures.astroVessel);
    this->textureManager->load("player_vessel",
                               configGameAssets.assets.textures.Player);
    this->textureManager->get("bg_menu").setRepeated(true);
    this->textureManager->get("bg_planet_1").setRepeated(true);
    this->textureManager->get("bg_planet_2").setRepeated(true);
    this->textureManager->get("bg_planet_3").setRepeated(true);
}
