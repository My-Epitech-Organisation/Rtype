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

    sf::Texture& get(const std::string& id);

    /**
     * @brief Check if a texture is loaded
     * @param id The texture identifier
     * @return true if the texture is loaded, false otherwise
     */
    bool isLoaded(const std::string& id) const;

    /**
     * @brief Unload a specific texture by id
     * @param id The texture identifier to unload
     * @return true if the texture was unloaded, false if not found
     */
    bool unload(const std::string& id);

    /**
     * @brief Unload all textures
     */
    void unloadAll();

    /**
     * @brief Get the number of loaded textures
     * @return Number of textures currently loaded
     */
    std::size_t size() const;

    TextureManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_TEXTUREMANAGER_HPP_
