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

#include <SFML/Audio.hpp>

class SoundManager {
   private:
    std::unordered_map<std::string, sf::SoundBuffer> _assets;

   public:
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    void load(const std::string& id, const std::string& filePath);

    sf::SoundBuffer get(const std::string& id);

    SoundManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_SOUNDMANAGER_HPP_
