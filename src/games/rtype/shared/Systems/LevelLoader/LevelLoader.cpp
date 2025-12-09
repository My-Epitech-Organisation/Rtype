/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LevelLoader - Implementation of level loading and map management
*/

#include "LevelLoader.hpp"

#include "Logger/Macros.hpp"

namespace rtype::games::rtype::shared {

LevelLoader::LevelLoader() : engine::ASystem("LevelLoader") {}

bool LevelLoader::loadLevel(const std::string& levelId) {
    const auto& configRegistry = EntityConfigRegistry::getInstance();
    auto levelOpt = configRegistry.getLevel(levelId);

    if (!levelOpt.has_value()) {
        LOG_ERROR("[LevelLoader] Level not found: " << levelId);
        return false;
    }

    loadLevel(levelOpt->get());
    return true;
}

void LevelLoader::loadLevel(const LevelConfig& config) {
    m_levelId = config.id;
    m_levelConfig = config;
    m_isLoaded = true;

    // Initialize scroll state from config
    m_scrollState.scrollOffset = 0.0F;
    m_scrollState.scrollSpeed = config.scrollSpeed;
    m_scrollState.levelWidth = config.map.levelWidth;
    m_scrollState.viewportWidth = config.map.viewportWidth;
    m_scrollState.isPaused = false;

    // Clear spawn tracking
    m_spawnedObstacleIndices.clear();
    m_spawnedTileIndices.clear();
    m_spawnedDecorationIndices.clear();
    m_spawnedEntities.clear();
    m_starfieldInitialized = false;

    LOG_INFO("[LevelLoader] Loaded level: "
             << config.id << " (width: " << config.map.levelWidth
             << ", speed: " << config.scrollSpeed << ")");
}

void LevelLoader::setMapElementCallback(MapElementSpawnCallback callback) {
    m_mapElementCallback = std::move(callback);
}

void LevelLoader::setStarfieldCallback(StarfieldSpawnCallback callback) {
    m_starfieldCallback = std::move(callback);
}

void LevelLoader::update(ECS::Registry& registry, float deltaTime) {
    if (!m_isLoaded || m_scrollState.isPaused) {
        return;
    }

    // Update scroll position (timer-based, independent of CPU speed)
    if (!m_scrollState.isComplete()) {
        m_scrollState.scrollOffset += m_scrollState.scrollSpeed * deltaTime;

        // Clamp to level bounds
        float maxScroll = m_scrollState.levelWidth - m_scrollState.viewportWidth;
        if (m_scrollState.scrollOffset > maxScroll) {
            m_scrollState.scrollOffset = maxScroll;
        }
    }

    // Spawn elements coming into view
    spawnVisibleElements(registry);

    // Despawn elements that went out of view
    despawnOutOfViewElements(registry);
}

void LevelLoader::initializeStarfield(ECS::Registry& registry) {
    if (!m_isLoaded || m_starfieldInitialized || !m_levelConfig) {
        return;
    }

    const auto& starfieldLayers = m_levelConfig->map.starfieldLayers;

    for (const auto& layer : starfieldLayers) {
        ECS::Entity entity = registry.spawnEntity();

        // Add base components
        registry.emplaceComponent<StarfieldTag>(entity);

        // Call user callback for additional setup (sprites, etc.)
        if (m_starfieldCallback) {
            m_starfieldCallback(registry, entity, layer);
        }

        m_starfieldEntities.push_back(entity);
        LOG_DEBUG("[LevelLoader] Spawned starfield layer: " << layer.id);
    }

    m_starfieldInitialized = true;
    LOG_INFO("[LevelLoader] Initialized " << starfieldLayers.size()
                                          << " starfield layers");
}

ECS::Entity LevelLoader::spawnMapElement(ECS::Registry& registry,
                                          const MapElementConfig& config) {
    ECS::Entity entity = registry.spawnEntity();

    // Add transform component (position in screen coordinates)
    float screenX = m_scrollState.levelToScreenX(config.x);
    registry.emplaceComponent<TransformComponent>(entity, screenX, config.y,
                                                   config.rotation);

    // Add scroll component to track original position
    registry.emplaceComponent<ScrollComponent>(entity, config.x);

    // Add bounding box for collision
    if (config.hasCollision()) {
        registry.emplaceComponent<BoundingBoxComponent>(
            entity, config.getHitboxWidth(), config.getHitboxHeight());
    }

    // Add tag based on type
    registry.emplaceComponent<MapElementTag>(entity);

    switch (config.type) {
        case MapElementType::Obstacle:
            registry.emplaceComponent<ObstacleTag>(entity);
            break;
        case MapElementType::DestroyableTile:
            registry.emplaceComponent<DestroyableTileTag>(entity);
            // Add health for destroyable tiles
            registry.emplaceComponent<HealthComponent>(entity, config.health,
                                                        config.health);
            break;
        case MapElementType::Decoration:
            registry.emplaceComponent<DecorationTag>(entity);
            break;
    }

    // Call user callback for additional setup (sprites, etc.)
    if (m_mapElementCallback) {
        m_mapElementCallback(registry, entity, config);
    }

    m_spawnedEntities.push_back(entity);
    return entity;
}

void LevelLoader::spawnVisibleElements(ECS::Registry& registry) {
    if (!m_levelConfig) {
        return;
    }

    float spawnThreshold = m_scrollState.getSpawnThreshold();

    // Check obstacles
    const auto& obstacles = m_levelConfig->map.obstacles;
    for (std::size_t i = 0; i < obstacles.size(); ++i) {
        if (m_spawnedObstacleIndices.contains(i)) {
            continue;
        }

        const auto& obstacle = obstacles[i];
        if (obstacle.x <= spawnThreshold) {
            spawnMapElement(registry, obstacle);
            m_spawnedObstacleIndices.insert(i);
            LOG_DEBUG("[LevelLoader] Spawned obstacle: " << obstacle.id
                                                         << " at x=" << obstacle.x);
        }
    }

    // Check destroyable tiles
    const auto& tiles = m_levelConfig->map.destroyableTiles;
    for (std::size_t i = 0; i < tiles.size(); ++i) {
        if (m_spawnedTileIndices.contains(i)) {
            continue;
        }

        const auto& tile = tiles[i];
        if (tile.x <= spawnThreshold) {
            spawnMapElement(registry, tile);
            m_spawnedTileIndices.insert(i);
            LOG_DEBUG("[LevelLoader] Spawned tile: " << tile.id
                                                     << " at x=" << tile.x);
        }
    }

    // Check decorations
    const auto& decorations = m_levelConfig->map.decorations;
    for (std::size_t i = 0; i < decorations.size(); ++i) {
        if (m_spawnedDecorationIndices.contains(i)) {
            continue;
        }

        const auto& deco = decorations[i];
        if (deco.x <= spawnThreshold) {
            spawnMapElement(registry, deco);
            m_spawnedDecorationIndices.insert(i);
            LOG_DEBUG("[LevelLoader] Spawned decoration: " << deco.id
                                                           << " at x=" << deco.x);
        }
    }
}

void LevelLoader::despawnOutOfViewElements(ECS::Registry& registry) {
    float despawnThreshold = m_scrollState.getDespawnThreshold();

    // Check all spawned entities
    auto it = m_spawnedEntities.begin();
    while (it != m_spawnedEntities.end()) {
        ECS::Entity entity = *it;

        // Check if entity still exists and has scroll component
        if (!registry.isAlive(entity) ||
            !registry.hasComponent<ScrollComponent>(entity)) {
            it = m_spawnedEntities.erase(it);
            continue;
        }

        const auto& scroll = registry.getComponent<ScrollComponent>(entity);
        float currentLevelX = scroll.initialX;

        // Despawn if past the despawn threshold
        if (currentLevelX < despawnThreshold) {
            registry.killEntity(entity);
            it = m_spawnedEntities.erase(it);
            LOG_DEBUG("[LevelLoader] Despawned entity at level x="
                      << currentLevelX);
        } else {
            ++it;
        }
    }
}

void LevelLoader::reset(ECS::Registry& registry) {
    // Kill all spawned entities
    for (ECS::Entity entity : m_spawnedEntities) {
        if (registry.isAlive(entity)) {
            registry.killEntity(entity);
        }
    }
    m_spawnedEntities.clear();

    // Kill starfield entities
    for (ECS::Entity entity : m_starfieldEntities) {
        if (registry.isAlive(entity)) {
            registry.killEntity(entity);
        }
    }
    m_starfieldEntities.clear();

    // Reset tracking
    m_spawnedObstacleIndices.clear();
    m_spawnedTileIndices.clear();
    m_spawnedDecorationIndices.clear();
    m_starfieldInitialized = false;

    // Reset scroll state
    m_scrollState.scrollOffset = 0.0F;
    m_scrollState.isPaused = false;

    LOG_INFO("[LevelLoader] Level reset: " << m_levelId);
}

}  // namespace rtype::games::rtype::shared
