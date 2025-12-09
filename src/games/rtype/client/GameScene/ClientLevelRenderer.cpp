/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientLevelRenderer - Implementation of client-side level rendering
*/

#include "ClientLevelRenderer.hpp"

#include "../AllComponents.hpp"
#include "../Components/ImageComponent.hpp"
#include "../Components/ParallaxComponent.hpp"
#include "../Components/SizeComponent.hpp"
#include "../Components/ZIndexComponent.hpp"
#include "../GraphicsConstants.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::client {

namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

ClientLevelRenderer::ClientLevelRenderer(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetManager)
    : m_registry(std::move(registry)), m_assetManager(std::move(assetManager)) {
}

void ClientLevelRenderer::setupLevelLoader(shared::LevelLoader& loader) {
    // Set up map element callback
    loader.setMapElementCallback(
        [this](ECS::Registry& reg, ECS::Entity entity,
               const shared::MapElementConfig& config) {
            onMapElementSpawned(reg, entity, config);
        });

    // Set up starfield callback
    loader.setStarfieldCallback(
        [this](ECS::Registry& reg, ECS::Entity entity,
               const shared::StarfieldLayerConfig& config) {
            onStarfieldSpawned(reg, entity, config);
        });
}

void ClientLevelRenderer::onMapElementSpawned(
    ECS::Registry& registry, ECS::Entity entity,
    const shared::MapElementConfig& config) {
    // Try to load the sprite, fall back to default if not found
    std::string textureName;

    // Use spriteSheet directly as texture name (e.g., "astro_vessel")
    // Textures are loaded with specific names in AssetManager, not file paths
    textureName = config.spriteSheet;

    // Try to get the texture, use default if not found
    const sf::Texture* texture = nullptr;
    if (!textureName.empty() &&
        m_assetManager->textureManager->isLoaded(textureName)) {
        texture = &m_assetManager->textureManager->get(textureName);
    } else {
        // Use default texture based on type
        const char* defaultName = DEFAULT_OBSTACLE_TEXTURE;
        switch (config.type) {
            case shared::MapElementType::DestroyableTile:
                defaultName = DEFAULT_TILE_TEXTURE;
                break;
            case shared::MapElementType::Decoration:
                defaultName = DEFAULT_DECORATION_TEXTURE;
                break;
            default:
                break;
        }

        if (m_assetManager->textureManager->isLoaded(defaultName)) {
            texture = &m_assetManager->textureManager->get(defaultName);
        }
    }

    // Add visual components if we have a texture
    if (texture != nullptr) {
        registry.emplaceComponent<rc::Image>(entity, *texture);

        // Set size if different from texture size
        if (config.width > 0.0F && config.height > 0.0F) {
            registry.emplaceComponent<rc::Size>(
                entity, static_cast<int>(config.width),
                static_cast<int>(config.height));
        }
    }

    // Set Z-index based on element type
    int zIndex = GraphicsConfig::ZINDEX_DEFAULT;
    switch (config.type) {
        case shared::MapElementType::Obstacle:
            zIndex = GraphicsConfig::ZINDEX_MAP_OBSTACLES;
            break;
        case shared::MapElementType::DestroyableTile:
            zIndex = GraphicsConfig::ZINDEX_MAP_TILES;
            break;
        case shared::MapElementType::Decoration:
            zIndex = GraphicsConfig::ZINDEX_MAP_DECORATIONS;
            break;
    }
    registry.emplaceComponent<rc::ZIndex>(entity, zIndex);

    LOG_DEBUG("[ClientLevelRenderer] Spawned map element: "
              << config.id << " at (" << config.x << ", " << config.y << ")");
}

void ClientLevelRenderer::onStarfieldSpawned(
    ECS::Registry& registry, ECS::Entity entity,
    const shared::StarfieldLayerConfig& config) {
    // Use texturePath directly as texture name (e.g., "bg_planet_1")
    // Textures are loaded with specific names in AssetManager, not file paths
    std::string textureName = config.texturePath;

    // Try to get the texture, use default if not found
    const sf::Texture* texture = nullptr;
    if (!textureName.empty() && m_assetManager->textureManager->isLoaded(textureName)) {
        texture = &m_assetManager->textureManager->get(textureName);
    } else if (m_assetManager->textureManager->isLoaded(DEFAULT_STARFIELD_TEXTURE)) {
        texture = &m_assetManager->textureManager->get(DEFAULT_STARFIELD_TEXTURE);
    }

    if (texture != nullptr) {
        registry.emplaceComponent<rc::Image>(entity, *texture);
    }

    // Add parallax component for scrolling
    registry.emplaceComponent<rc::Parallax>(entity, config.scrollFactor,
                                            config.isRepeating);

    // Position at origin (parallax system will handle positioning)
    registry.emplaceComponent<rs::Position>(entity, 0.0F, 0.0F);

    // Set Z-index from config
    registry.emplaceComponent<rc::ZIndex>(entity, config.zIndex);

    LOG_DEBUG("[ClientLevelRenderer] Spawned starfield layer: "
              << config.id << " (scroll factor: " << config.scrollFactor
              << ", z-index: " << config.zIndex << ")");
}

}  // namespace rtype::games::rtype::client
