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

#include "rtype/display/IDisplay.hpp"

class TextureManager {
   private:
    rtype::display::IDisplay* _display;

   public:
    explicit TextureManager(rtype::display::IDisplay* display)
        : _display(display) {}
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    void load(const std::string& id, const std::string& filePath);

    std::shared_ptr<::rtype::display::ITexture> get(const std::string& id);

    /**
     * @brief Check if a texture is loaded
     * @param id The texture identifier
     * @return true if the texture is loaded, false otherwise
     */
    bool isLoaded(const std::string& id) const;
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_TEXTUREMANAGER_HPP_
