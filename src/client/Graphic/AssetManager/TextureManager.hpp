/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** AssetManager.hpp
*/

#ifndef R_TYPE_TEXTUREMANAGER_HPP
#define R_TYPE_TEXTUREMANAGER_HPP

#include <SFML/Graphics/Texture.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <iostream>

class TextureManager {
private:
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> _assets;
public:
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    void load(const std::string& id, const std::string &filePath);

    void load(const std::string& id, unsigned char *fileData, unsigned int fileDataSize);

    sf::Texture& get(const std::string& id);

    TextureManager() = default;
};


#endif //R_TYPE_TEXTUREMANAGER_HPP