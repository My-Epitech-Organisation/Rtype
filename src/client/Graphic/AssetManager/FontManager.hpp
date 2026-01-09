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

#include "rtype/display/IDisplay.hpp"

class FontManager {
   private:
    std::unordered_map<std::string, std::shared_ptr<::rtype::display::IFont>>
        _assets;

   public:
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    void load(const std::string& id, const std::string& filePath);

    std::shared_ptr<::rtype::display::IFont> get(const std::string& id);

    /**
     * @brief Check if a font is loaded
     * @param id The font identifier
     * @return true if the font is loaded, false otherwise
     */
    bool isLoaded(const std::string& id) const;

    /**
     * @brief Unload a specific font by id
     * @param id The font identifier to unload
     * @return true if the font was unloaded, false if not found
     */
    bool unload(const std::string& id);

    /**
     * @brief Unload all fonts
     */
    void unloadAll();

    /**
     * @brief Get the number of loaded fonts
     * @return Number of fonts currently loaded
     */
    std::size_t size() const;

    explicit FontManager(::rtype::display::IDisplay* display)
        : _display(display) {}

   private:
    ::rtype::display::IDisplay* _display;
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_FONTMANAGER_HPP_
