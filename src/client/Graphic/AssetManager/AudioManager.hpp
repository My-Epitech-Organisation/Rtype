/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AudioManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_AUDIOMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_AUDIOMANAGER_HPP_

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "rtype/display/IDisplay.hpp"
#include "AAssetManager.hpp"

class AudioManager : public AAssetManager<::rtype::display::IMusic> {
   public:
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void load(const std::string& id, const std::string& filePath) override;

    explicit AudioManager(::rtype::display::IDisplay* display)
        : AAssetManager<::rtype::display::IMusic>(display, "Audio") {}

};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_AUDIOMANAGER_HPP_
