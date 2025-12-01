/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_TEXTUREMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_TEXTUREMANAGER_HPP_

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <SFML/Graphics/Texture.hpp>

class TextureManager {
   private:
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> _assets;

   public:
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    void load(const std::string& id, const std::string& filePath);

    void load(const std::string& id, unsigned char* fileData,
              unsigned int fileDataSize);

    sf::Texture& get(const std::string& id);

    TextureManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_TEXTUREMANAGER_HPP_
