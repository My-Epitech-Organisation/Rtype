/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LevelLoader - System for loading and managing level map data
*/

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <unordered_set>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "Components.hpp"
#include "Config/EntityConfig/EntityConfig.hpp"

namespace rtype::games::rtype::shared {

/**
 * @brief Callback type for entity spawning (allows custom component addition)
 *
 * Called when a map element entity is spawned. The callback receives:
 * - registry: The ECS registry
 * - entity: The spawned entity
 * - config: The map element configuration
 */
using MapElementSpawnCallback =
    std::function<void(ECS::Registry&, ECS::Entity, const MapElementConfig&)>;

/**
 * @brief Callback type for starfield layer spawning
 */
using StarfieldSpawnCallback = std::function<void(
    ECS::Registry&, ECS::Entity, const StarfieldLayerConfig&)>;

/**
 * @class LevelLoader
 * @brief Handles loading and spawning of level map elements
 *
 * The LevelLoader manages the lifecycle of map elements:
 * - Loads level configuration from files
 * - Spawns entities as they come into view (viewport culling)
 * - Tracks spawned entities and their states
 * - Provides scroll state management
 *
 * Usage:
 * @code
 *   LevelLoader loader;
 *   loader.loadLevel("level_1");
 *   loader.setMapElementCallback([](auto& reg, auto entity, auto& config) {
 *       // Add sprite components, etc.
 *   });
 *
 *   // In game loop:
 *   loader.update(registry, deltaTime);
 * @endcode
 */
class LevelLoader : public engine::ASystem {
   public:
    LevelLoader();
    ~LevelLoader() override = default;

    /**
     * @brief Load a level by ID from the EntityConfigRegistry
     * @param levelId Level identifier
     * @return true if level was loaded successfully
     */
    bool loadLevel(const std::string& levelId);

    /**
     * @brief Load a level from a LevelConfig directly
     * @param config Level configuration
     */
    void loadLevel(const LevelConfig& config);

    /**
     * @brief Set callback for map element spawning
     * @param callback Function called when map elements are spawned
     */
    void setMapElementCallback(MapElementSpawnCallback callback);

    /**
     * @brief Set callback for starfield layer spawning
     * @param callback Function called when starfield layers are spawned
     */
    void setStarfieldCallback(StarfieldSpawnCallback callback);

    /**
     * @brief Update the level loader (spawn/despawn entities based on scroll)
     * @param registry ECS registry
     * @param deltaTime Time since last update
     */
    void update(ECS::Registry& registry, float deltaTime) override;

    /**
     * @brief Get the current scroll state
     * @return Reference to the scroll state
     */
    [[nodiscard]] LevelScrollState& getScrollState() noexcept {
        return m_scrollState;
    }

    /**
     * @brief Get the current scroll state (const)
     * @return Const reference to the scroll state
     */
    [[nodiscard]] const LevelScrollState& getScrollState() const noexcept {
        return m_scrollState;
    }

    /**
     * @brief Set the scroll speed
     * @param speed Scroll speed in pixels per second
     */
    void setScrollSpeed(float speed) noexcept {
        m_scrollState.scrollSpeed = speed;
    }

    /**
     * @brief Pause or resume scrolling
     * @param paused Whether scrolling should be paused
     */
    void setPaused(bool paused) noexcept { m_scrollState.isPaused = paused; }

    /**
     * @brief Check if level is loaded
     */
    [[nodiscard]] bool isLoaded() const noexcept { return m_isLoaded; }

    /**
     * @brief Check if level has completed
     */
    [[nodiscard]] bool isComplete() const noexcept {
        return m_scrollState.isComplete();
    }

    /**
     * @brief Reset the level to initial state
     * @param registry ECS registry to clean up entities
     */
    void reset(ECS::Registry& registry);

    /**
     * @brief Get the loaded level ID
     */
    [[nodiscard]] const std::string& getLevelId() const noexcept {
        return m_levelId;
    }

    /**
     * @brief Initialize starfield layers (call once after level load)
     * @param registry ECS registry
     */
    void initializeStarfield(ECS::Registry& registry);

   private:
    /**
     * @brief Spawn a map element entity
     * @param registry ECS registry
     * @param config Element configuration
     * @return The spawned entity
     */
    ECS::Entity spawnMapElement(ECS::Registry& registry,
                                const MapElementConfig& config);

    /**
     * @brief Check and spawn elements that are now in view
     * @param registry ECS registry
     */
    void spawnVisibleElements(ECS::Registry& registry);

    /**
     * @brief Check and despawn elements that are out of view
     * @param registry ECS registry
     */
    void despawnOutOfViewElements(ECS::Registry& registry);

    LevelScrollState m_scrollState;
    std::string m_levelId;
    bool m_isLoaded = false;

    // Current level configuration (copy for runtime modifications)
    std::optional<LevelConfig> m_levelConfig;

    // Track spawned obstacle indices (to avoid respawning)
    std::unordered_set<std::size_t> m_spawnedObstacleIndices;
    std::unordered_set<std::size_t> m_spawnedTileIndices;
    std::unordered_set<std::size_t> m_spawnedDecorationIndices;

    // Callbacks for custom entity setup
    MapElementSpawnCallback m_mapElementCallback;
    StarfieldSpawnCallback m_starfieldCallback;

    // Track spawned entities for cleanup
    std::vector<ECS::Entity> m_spawnedEntities;
    std::vector<ECS::Entity> m_starfieldEntities;
    bool m_starfieldInitialized = false;
};

}  // namespace rtype::games::rtype::shared
