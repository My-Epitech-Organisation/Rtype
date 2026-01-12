/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SoundManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_SOUNDMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_SOUNDMANAGER_HPP_

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "AAssetManager.hpp"
#include "rtype/display/IDisplay.hpp"

class SoundManager : public AAssetManager<::rtype::display::ISoundBuffer> {
   public:
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    void load(const std::string& id, const std::string& filePath) override;

    explicit SoundManager(::rtype::display::IDisplay* display)
        : AAssetManager<::rtype::display::ISoundBuffer>(display, "Sound") {}
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_SOUNDMANAGER_HPP_
