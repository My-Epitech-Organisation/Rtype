/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeEntityFactory.cpp
*/

#include "RtypeEntityFactory.hpp"

#include <memory>
#include <random>
#include <utility>

#include "../Components/RectangleComponent.hpp"
#include "../Systems/PlayerAnimationSystem.hpp"
#include "../shared/Components/BoundingBoxComponent.hpp"
#include "../shared/Components/HealthComponent.hpp"
#include "../shared/Components/NetworkIdComponent.hpp"
#include "../shared/Components/PlayerIdComponent.hpp"
#include "../shared/Config/GameConfig/RTypeGameConfig.hpp"
#include "AllComponents.hpp"
#include "AudioLib/AudioLib.hpp"
#include "Components/AnnimationComponent.hpp"
#include "Components/LifetimeComponent.hpp"
#include "Components/SoundComponent.hpp"
#include "Components/Tags.hpp"
#include "Graphic/ControllerRumble.hpp"
#include "GraphicsConstants.hpp"
#include "Logger/Macros.hpp"
#include "VisualCueFactory.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Get the texture rectangle (sprite position) for a player based on
 * their ID
 *
 * The player_vessel sprite sheet has 4 rows (colors) and multiple columns.
 * Each row represents a different player color:
 * - Row 0 (Y=0): Blue player
 * - Row 1 (Y=17): Pink/Magenta player
 * - Row 2 (Y=34): Green player
 * - Row 3 (Y=51): Red player
 *
 * @param playerId Player ID (1-MAX_PLAYER_COUNT)
 * @return Pair of (offset_x, offset_y) for the texture rectangle
 */
static std::pair<int, int> getPlayerSpriteOffset(uint32_t playerId) {
    const int SPRITE_HEIGHT = 17;
    uint32_t rowIndex = 0;
    if (playerId >= 1 && playerId <= ::rtype::game::config::MAX_PLAYER_COUNT) {
        rowIndex = playerId - 1;
    }
    int yOffset = static_cast<int>(rowIndex) * SPRITE_HEIGHT;

    return {0, yOffset};
}

::rtype::client::ClientNetworkSystem::EntityFactory
RtypeEntityFactory::createNetworkEntityFactory(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetsManager) {
    return [assetsManager, registry](
               ECS::Registry& reg,
               const ::rtype::client::EntitySpawnEvent& event) -> ECS::Entity {
        LOG_DEBUG("[RtypeEntityFactory] Creating entity type="
                  << static_cast<int>(event.type) << " pos=(" << event.x << ", "
                  << event.y << ")");

        auto entity = reg.spawnEntity();

        reg.emplaceComponent<::rtype::games::rtype::shared::TransformComponent>(
            entity, event.x, event.y);
        reg.emplaceComponent<::rtype::games::rtype::shared::VelocityComponent>(
            entity, 0.f, 0.f);
        reg.emplaceComponent<::rtype::games::rtype::shared::NetworkIdComponent>(
            entity, event.entityId);

        switch (event.type) {
            case ::rtype::network::EntityType::Player:
                setupPlayerEntity(reg, assetsManager, entity, event.userId);
                break;

            case ::rtype::network::EntityType::Bydos:
                setupBydosEntity(reg, assetsManager, entity);
                break;

            case ::rtype::network::EntityType::Missile:
                setupMissileEntity(reg, assetsManager, entity);
                break;

            case ::rtype::network::EntityType::Pickup:
                setupPickupEntity(reg, entity, event.entityId);
                break;

            case ::rtype::network::EntityType::Obstacle:
                setupObstacleEntity(reg, assetsManager, entity, event.entityId);
                break;
        }

        return entity;
    };
}

void RtypeEntityFactory::setupPlayerEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity, std::uint32_t userId) {
    LOG_DEBUG("[RtypeEntityFactory] Adding Player components for entity "
              << entity.id);

    uint32_t playerId = 1;
    if (userId < 1 || userId > ::rtype::game::config::MAX_PLAYER_COUNT) {
        LOG_ERROR("[RtypeEntityFactory] Invalid userId "
                  << userId << ", must be 1-"
                  << ::rtype::game::config::MAX_PLAYER_COUNT
                  << ". Defaulting to 1");
        userId = 1;
    }
    playerId = userId;

    std::pair<int, int> spriteOffset = getPlayerSpriteOffset(playerId);
    LOG_DEBUG("[RtypeEntityFactory] Player " << playerId << " sprite offset: ("
                                             << spriteOffset.first << ", "
                                             << spriteOffset.second << ")");

    reg.emplaceComponent<shared::PlayerIdComponent>(entity, playerId);

    constexpr int neutralColumn = 2;
    constexpr int width = PlayerAnimationSystem::kFrameWidth;
    constexpr int height = PlayerAnimationSystem::kFrameHeight;

    const int left = neutralColumn * width;
    const int top = spriteOffset.second;

    LOG_DEBUG("[RtypeEntityFactory] Getting player_vessel texture");
    auto& playerTexture = assetsManager->textureManager->get("player_vessel");
    LOG_DEBUG("[RtypeEntityFactory] Texture size: "
              << playerTexture.getSize().x << "x" << playerTexture.getSize().y);
    reg.emplaceComponent<Image>(entity, playerTexture);
    LOG_DEBUG("[RtypeEntityFactory] Image component added");
    reg.emplaceComponent<TextureRect>(entity, std::pair<int, int>({left, top}),
                                      std::pair<int, int>({width, height}));
    LOG_DEBUG("[RtypeEntityFactory] TextureRect set to: left="
              << left << " top=" << top << " width=" << width
              << " height=" << height);
    reg.emplaceComponent<Size>(entity, 4, 4);
    LOG_DEBUG("[RtypeEntityFactory] Size component added");
    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, 132.0f, 68.0f);
    reg.emplaceComponent<shared::HealthComponent>(entity, 1, 1);
    reg.emplaceComponent<PlayerTag>(entity);
    reg.emplaceComponent<BoxingComponent>(entity,
                                          sf::FloatRect({0, 0}, {132.f, 68.f}));
    reg.getComponent<BoxingComponent>(entity).outlineColor = sf::Color::White;
    reg.getComponent<BoxingComponent>(entity).fillColor =
        sf::Color(0, 200, 255, 45);
    reg.emplaceComponent<ZIndex>(entity, 0);
    reg.emplaceComponent<GameTag>(entity);
    reg.emplaceComponent<PlayerSoundComponent>(
        entity, assetsManager->soundManager->get("player_spawn"),
        assetsManager->soundManager->get("player_death"));
    auto lib = reg.getSingleton<std::shared_ptr<AudioLib>>();
    lib->playSFX(*assetsManager->soundManager->get("player_spawn"));
}

void RtypeEntityFactory::setupBydosEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity) {
    LOG_DEBUG("[RtypeEntityFactory] Adding Bydos components");
    reg.emplaceComponent<Image>(
        entity, assetsManager->textureManager->get("bdos_enemy"));
    reg.emplaceComponent<TextureRect>(entity, std::pair<int, int>({0, 0}),
                                      std::pair<int, int>({33, 34}));
    reg.emplaceComponent<Size>(entity, 2, 2);
    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, 66.0f, 68.0f);
    reg.emplaceComponent<BoxingComponent>(entity,
                                          sf::FloatRect({0, 0}, {66.f, 68.f}));
    reg.getComponent<BoxingComponent>(entity).outlineColor =
        sf::Color(255, 120, 0);
    reg.getComponent<BoxingComponent>(entity).fillColor =
        sf::Color(255, 120, 0, 40);
    reg.emplaceComponent<ZIndex>(entity, 0);
    reg.emplaceComponent<shared::HealthComponent>(entity, 10, 10);
    reg.emplaceComponent<GameTag>(entity);
    reg.emplaceComponent<EnemySoundComponent>(
        entity, assetsManager->soundManager->get("bydos_spawn"),
        assetsManager->soundManager->get("bydos_death"));
    auto lib = reg.getSingleton<std::shared_ptr<AudioLib>>();
    lib->playSFX(*assetsManager->soundManager->get("bydos_spawn"));
}

void RtypeEntityFactory::setupMissileEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity) {
    LOG_DEBUG("[RtypeEntityFactory] Adding Missile components");
    reg.emplaceComponent<Image>(
        entity, assetsManager->textureManager->get("projectile_player_laser"));
    reg.emplaceComponent<TextureRect>(entity, std::pair<int, int>({0, 0}),
                                      std::pair<int, int>({33, 34}));
    reg.emplaceComponent<Size>(entity, 1.75, 1.75);
    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, 33.0f, 34.0f);
    reg.emplaceComponent<shared::ProjectileTag>(entity);
    reg.emplaceComponent<BoxingComponent>(entity,
                                          sf::FloatRect({0, 0}, {33.f, 34.f}));
    reg.emplaceComponent<Animation>(entity, 4, 0.5, true);
    reg.getComponent<BoxingComponent>(entity).outlineColor =
        sf::Color(0, 220, 180);
    reg.getComponent<BoxingComponent>(entity).fillColor =
        sf::Color(0, 220, 180, 35);
    reg.emplaceComponent<ZIndex>(entity, 1);
    reg.emplaceComponent<shared::LifetimeComponent>(
        entity, GraphicsConfig::LIFETIME_PROJECTILE);
    reg.emplaceComponent<GameTag>(entity);
    auto lib = reg.getSingleton<std::shared_ptr<AudioLib>>();
    lib->playSFX(*assetsManager->soundManager->get("laser_sfx"));

    if (reg.hasComponent<shared::TransformComponent>(entity)) {
        const auto& pos = reg.getComponent<shared::TransformComponent>(entity);
        VisualCueFactory::createFlash(reg, {pos.x, pos.y},
                                      sf::Color(0, 255, 220), 52.f, 0.25f, 10);
    }
}

void RtypeEntityFactory::setupPickupEntity(ECS::Registry& reg,
                                           ECS::Entity entity,
                                           std::uint32_t networkId) {
    LOG_DEBUG("[RtypeEntityFactory] Adding Pickup components");
    static const std::array<sf::Color, 4> kColors = {
        sf::Color(120, 200, 255), sf::Color(170, 120, 255),
        sf::Color(120, 255, 170), sf::Color(255, 200, 120)};
    const sf::Color color = kColors[networkId % kColors.size()];

    reg.emplaceComponent<Rectangle>(entity, std::pair<float, float>{24.f, 24.f},
                                    color, color);
    reg.getComponent<Rectangle>(entity).outlineThickness = 2.f;
    reg.getComponent<Rectangle>(entity).outlineColor = sf::Color::White;
    reg.emplaceComponent<BoxingComponent>(entity,
                                          sf::FloatRect({0, 0}, {24.f, 24.f}));
    reg.getComponent<BoxingComponent>(entity).outlineColor = color;
    reg.getComponent<BoxingComponent>(entity).fillColor =
        sf::Color(color.r, color.g, color.b, 45);
    reg.emplaceComponent<ZIndex>(entity, 0);
    reg.emplaceComponent<GameTag>(entity);
    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, 24.0f, 24.0f);
}

void RtypeEntityFactory::setupObstacleEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity, std::uint32_t /*networkId*/) {
    LOG_DEBUG("[RtypeEntityFactory] Adding Obstacle components");

    std::random_device rd;   // Seed
    std::mt19937 gen(rd());  // Mersenne Twister engine
    std::uniform_int_distribution<> dist(1, GraphicsConfig::NBR_MAX_OBSTACLES);

    int value = dist(gen);
    std::string textureName = "projectile" + std::to_string(value);

    auto& projectileTexture = assetsManager->textureManager->get(textureName);
    reg.emplaceComponent<Image>(entity, projectileTexture);
    reg.emplaceComponent<Size>(entity, 0.5, 0.5);
    reg.emplaceComponent<ZIndex>(entity, 0);
    reg.emplaceComponent<GameTag>(entity);
}

}  // namespace rtype::games::rtype::client
