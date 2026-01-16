/*
** EPITECH PROJECT, 2026
** Rtype
** File description:
** IAssetManager
*/

#ifndef SRC_CLIENT_GRAPHIC_ASSETMANAGER_IASSETMANAGER_HPP_
#define SRC_CLIENT_GRAPHIC_ASSETMANAGER_IASSETMANAGER_HPP_
#include <cstddef>
#include <memory>
#include <string>

template <typename T>
class IAssetManager {
   public:
    virtual void load(const std::string& id, const std::string& filePath) = 0;

    virtual std::shared_ptr<T> get(const std::string& id) = 0;

    /**
     * @brief Check if a asset is loaded
     * @param id The asset identifier
     * @return true if the asset is loaded, false otherwise
     */
    virtual bool isLoaded(const std::string& id) const = 0;

    /**
     * @brief Unload a specific asset by id
     * @param id The asset identifier to unload
     * @return true if the asset was unloaded, false if not found
     */
    virtual bool unload(const std::string& id) = 0;

    /**
     * @brief Unload all assets
     */
    virtual void unloadAll() = 0;

    /**
     * @brief Get the number of loaded asset
     * @return Number of asset currently loaded
     */
    virtual std::size_t size() const = 0;

    virtual ~IAssetManager() = default;
};

#endif  // SRC_CLIENT_GRAPHIC_ASSETMANAGER_IASSETMANAGER_HPP_
