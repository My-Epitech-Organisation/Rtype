/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MapElementConfig - Map element configuration structures for obstacles, tiles, and starfield
*/

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace rtype::games::rtype::shared {

/**
 * @enum MapElementType
 * @brief Type of map element
 */
enum class MapElementType {
    Obstacle,        ///< Static obstacle that blocks movement (indestructible)
    DestroyableTile, ///< Tile that can be destroyed by player projectiles
    Decoration       ///< Visual-only element (no collision)
};

/**
 * @struct MapElementConfig
 * @brief Configuration for a single map element (obstacle, tile, decoration)
 *
 * Elements are positioned in world coordinates and scroll with the level.
 * The x position is relative to the level start (0 = left edge when level starts).
 */
struct MapElementConfig {
    std::string id;                               ///< Unique identifier
    MapElementType type = MapElementType::Obstacle;
    std::string spriteSheet;                      ///< Path to sprite/texture
    
    // Position (in world coordinates, relative to level start)
    float x = 0.0F;                               ///< X position in level
    float y = 0.0F;                               ///< Y position
    float rotation = 0.0F;                        ///< Rotation in degrees
    
    // Dimensions
    float width = 32.0F;                          ///< Element width
    float height = 32.0F;                         ///< Element height
    
    // Collision (for obstacles and destroyable tiles)
    float hitboxWidth = 0.0F;                     ///< Hitbox width (0 = use width)
    float hitboxHeight = 0.0F;                    ///< Hitbox height (0 = use height)
    
    // Destroyable properties
    int32_t health = 0;                           ///< Health (0 = indestructible)
    int32_t scoreValue = 0;                       ///< Score when destroyed
    
    /**
     * @brief Get effective hitbox width
     */
    [[nodiscard]] float getHitboxWidth() const noexcept {
        return hitboxWidth > 0.0F ? hitboxWidth : width;
    }
    
    /**
     * @brief Get effective hitbox height
     */
    [[nodiscard]] float getHitboxHeight() const noexcept {
        return hitboxHeight > 0.0F ? hitboxHeight : height;
    }
    
    /**
     * @brief Check if element has collision
     */
    [[nodiscard]] bool hasCollision() const noexcept {
        return type != MapElementType::Decoration;
    }
    
    /**
     * @brief Check if element is destroyable
     */
    [[nodiscard]] bool isDestroyable() const noexcept {
        return type == MapElementType::DestroyableTile && health > 0;
    }
    
    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && width > 0.0F && height > 0.0F;
    }
};

/**
 * @struct StarfieldLayerConfig
 * @brief Configuration for a single starfield/background layer
 *
 * Starfield layers scroll at different speeds for parallax effect.
 * Scroll factor determines how fast the layer moves relative to the level.
 */
struct StarfieldLayerConfig {
    std::string id;                               ///< Unique identifier
    std::string texturePath;                      ///< Path to texture
    float scrollFactor = 0.0F;                    ///< Parallax scroll factor (0 = static, 1 = same as level)
    int32_t zIndex = 0;                           ///< Render order (lower = further back)
    bool isRepeating = true;                      ///< Whether texture repeats horizontally
    float scale = 1.0F;                           ///< Scale factor for the texture
    
    [[nodiscard]] bool isValid() const noexcept {
        return !texturePath.empty();
    }
};

/**
 * @struct MapConfig
 * @brief Complete map configuration for a level
 *
 * Contains all map elements and starfield layers for a level.
 * Elements are spawned as they come into view (viewport culling).
 */
struct MapConfig {
    std::vector<MapElementConfig> obstacles;      ///< Static obstacles
    std::vector<MapElementConfig> destroyableTiles; ///< Destroyable tiles
    std::vector<MapElementConfig> decorations;    ///< Visual decorations
    std::vector<StarfieldLayerConfig> starfieldLayers; ///< Background layers
    
    // Level dimensions
    float levelWidth = 10000.0F;                  ///< Total level width in pixels
    float viewportWidth = 1920.0F;                ///< Viewport width
    float viewportHeight = 1080.0F;               ///< Viewport height
    
    /**
     * @brief Get total number of map elements
     */
    [[nodiscard]] std::size_t getTotalElements() const noexcept {
        return obstacles.size() + destroyableTiles.size() + decorations.size();
    }
    
    /**
     * @brief Check if map has any content
     */
    [[nodiscard]] bool hasContent() const noexcept {
        return !obstacles.empty() || !destroyableTiles.empty() ||
               !decorations.empty() || !starfieldLayers.empty();
    }
};

/**
 * @brief Convert string to MapElementType enum
 * @param str String representation of the type
 * @return Corresponding MapElementType
 */
inline MapElementType stringToMapElementType(const std::string& str) {
    if (str == "obstacle" || str == "Obstacle") {
        return MapElementType::Obstacle;
    }
    if (str == "destroyable" || str == "Destroyable" ||
        str == "destroyable_tile" || str == "DestroyableTile") {
        return MapElementType::DestroyableTile;
    }
    if (str == "decoration" || str == "Decoration") {
        return MapElementType::Decoration;
    }
    return MapElementType::Obstacle;  // Default
}

}  // namespace rtype::games::rtype::shared
