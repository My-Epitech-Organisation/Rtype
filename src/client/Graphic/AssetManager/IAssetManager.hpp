/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** IAssetManager
*/

#ifndef SRC_CLIENT_GRAPHIC_IASSETMANAGER_FONTMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_IASSETMANAGER_FONTMANAGER_HPP_

template<typename T>
class IAssetManager {
    public:

    virtual void load(const std::string& id, const std::string& filePath) = 0;

    virtual std::shared_ptr<T> get(const std::string& id) = 0;

    /**
     * @brief Check if a font is loaded
     * @param id The font identifier
     * @return true if the font is loaded, false otherwise
     */
    virtual bool isLoaded(const std::string& id) const = 0;

    /**
     * @brief Unload a specific font by id
     * @param id The font identifier to unload
     * @return true if the font was unloaded, false if not found
     */
    virtual bool unload(const std::string& id) = 0;


    /**
     * @brief Unload all fonts
     */
    virtual void unloadAll() = 0;

        /**
     * @brief Get the number of loaded fonts
     * @return Number of fonts currently loaded
     */
    virtual std::size_t size() const = 0;


    virtual ~IAssetManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_IASSETMANAGER_FONTMANAGER_HPP_
