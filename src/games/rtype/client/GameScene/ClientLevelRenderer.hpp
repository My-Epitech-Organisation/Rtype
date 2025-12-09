/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientLevelRenderer - Client-side level rendering helper
*/

#pragma once

#include <memory>
#include <string>

#include "ECS.hpp"
#include "Graphic/AssetManager/AssetManager.hpp"
#include "games/rtype/shared/Config/EntityConfig/EntitiesStructs/MapElementConfig.hpp"
#include "games/rtype/shared/Systems/LevelLoader/LevelLoader.hpp"

namespace rtype::games::rtype::client {

/**
 * @class ClientLevelRenderer
 * @brief Helper class for setting up level rendering on the client
 *
 * This class provides callbacks for the LevelLoader to create visual
 * representations of map elements (sprites, etc.) on the client side.
 *
 * Usage:
 * @code
 *   LevelLoader loader;
 *   ClientLevelRenderer renderer(registry, assetManager);
 *   renderer.setupLevelLoader(loader);
 *   loader.loadLevel("level_1");
 *   loader.initializeStarfield(registry);
 * @endcode
 */
class ClientLevelRenderer {
   public:
    /**
     * @brief Constructor
     * @param registry ECS registry
     * @param assetManager Asset manager for textures
     */
    ClientLevelRenderer(std::shared_ptr<ECS::Registry> registry,
                        std::shared_ptr<AssetManager> assetManager);

    /**
     * @brief Set up the level loader with rendering callbacks
     * @param loader Level loader to configure
     */
    void setupLevelLoader(shared::LevelLoader& loader);

    /**
     * @brief Callback for spawning map elements (obstacles, tiles, decorations)
     * @param registry ECS registry
     * @param entity The spawned entity
     * @param config Map element configuration
     */
    void onMapElementSpawned(ECS::Registry& registry, ECS::Entity entity,
                             const shared::MapElementConfig& config);

    /**
     * @brief Callback for spawning starfield layers
     * @param registry ECS registry
     * @param entity The spawned entity
     * @param config Starfield layer configuration
     */
    void onStarfieldSpawned(ECS::Registry& registry, ECS::Entity entity,
                            const shared::StarfieldLayerConfig& config);

   private:
    std::shared_ptr<ECS::Registry> m_registry;
    std::shared_ptr<AssetManager> m_assetManager;

    // Fallback texture names when specific sprites are not found
    static constexpr const char* DEFAULT_OBSTACLE_TEXTURE = "obstacle_default";
    static constexpr const char* DEFAULT_TILE_TEXTURE = "tile_default";
    static constexpr const char* DEFAULT_DECORATION_TEXTURE = "decoration_default";
    static constexpr const char* DEFAULT_STARFIELD_TEXTURE = "bg_menu";
};

}  // namespace rtype::games::rtype::client
