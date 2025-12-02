/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.cpp
*/
#include "AssetManager.hpp"
#include "assets/Audiowide_Regular.h"
#include "assets/astroVessel.h"
#include "assets/bgMainMenu.h"
#include "assets/planet1.h"
#include "assets/planet2.h"
#include "assets/planet3.h"
#include "assets/playerVessel.h"

AssetManager::AssetManager() {
    this->fontManager->load("title_font", Audiowide_Regular_ttf,
                                         Audiowide_Regular_ttf_len);
    this->textureManager->load("bg_menu", bgMainMenu_png,
                                               bgMainMenu_png_len);
    this->textureManager->load("bg_planet_1", planet1_png,
                                               planet1_png_len);
    this->textureManager->load("bg_planet_2", planet2_png,
                                               planet2_png_len);
    this->textureManager->load("bg_planet_3", planet3_png,
                                               planet3_png_len);
    this->textureManager->load("astro_vessel", astroVessel_png,
                                               astroVessel_png_len);
    this->textureManager->load(
    "player_vessel", playerVessel_gif, playerVessel_gif_len);

    this->textureManager->get("bg_menu").setRepeated(true);
    this->textureManager->get("bg_planet_1").setRepeated(true);
    this->textureManager->get("bg_planet_2").setRepeated(true);
    this->textureManager->get("bg_planet_3").setRepeated(true);
}
