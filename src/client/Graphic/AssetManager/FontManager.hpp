/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** FontManager.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_FONTMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_FONTMANAGER_HPP_

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <SFML/Graphics/Font.hpp>

class FontManager {
   private:
    std::unordered_map<std::string, std::unique_ptr<sf::Font>> _assets;

   public:
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    void load(const std::string& id, const std::string& filePath);

    void load(const std::string& id, unsigned char* fileData,
              unsigned int fileDataSize);

    sf::Font& get(const std::string& id);

    FontManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_FONTMANAGER_HPP_
