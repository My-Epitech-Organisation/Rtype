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

#include <SFML/Audio.hpp>
#include <SFML/Audio/Music.hpp>

class AudioManager {
   private:
    std::unordered_map<std::string, std::shared_ptr<sf::Music>> _assets;

   public:
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void load(const std::string& id, const std::string& filePath);

    std::shared_ptr<sf::Music> get(const std::string& id);

    AudioManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_AUDIOMANAGER_HPP_
