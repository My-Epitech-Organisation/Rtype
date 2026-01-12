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

#include "../../shared/Components/BoundingBoxComponent.hpp"
#include "../../shared/Components/EnemyTypeComponent.hpp"
#include "../../shared/Components/ForcePodComponent.hpp"
#include "../../shared/Components/HealthComponent.hpp"
#include "../../shared/Components/NetworkIdComponent.hpp"
#include "../../shared/Components/PlayerIdComponent.hpp"
#include "../../shared/Components/PowerUpTypeComponent.hpp"
#include "../../shared/Components/Tags.hpp"
#include "../../shared/Config/EntityConfig/EntityConfig.hpp"
#include "../../shared/Config/GameConfig/RTypeGameConfig.hpp"
#include "../Components/ChaserExplosionComponent.hpp"
#include "../Components/ColorTintComponent.hpp"
#include "../Components/ForcePodVisualComponent.hpp"
#include "../Components/RectangleComponent.hpp"
#include "../Components/RotationComponent.hpp"
#include "../Systems/PlayerAnimationSystem.hpp"
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
 * @brief Map server userId to client playerId for consistent color assignment
 *
 * The server assigns userIds based on connection order (1, 2, 3, 4, ...)
 * We map these to playerId (1-4) for player colors:
 * userId 1 → playerId 1 (blue)
 * userId 2 → playerId 2 (pink)
 * userId 3 → playerId 3 (green)
 * userId 4 → playerId 4 (red)
 * userId 5+ → wraps around
 */
static inline uint32_t userIdToPlayerId(uint32_t userId) {
    if (userId < 1) return 1;
    uint32_t playerId =
        ((userId - 1) % ::rtype::game::config::MAX_PLAYER_COUNT) + 1;
    return playerId;
}

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
        LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                      "[RtypeEntityFactory] Creating entity type="
                          << static_cast<int>(event.type) << " pos=(" << event.x
                          << ", " << event.y << ")");

        auto entity = reg.spawnEntity();

        reg.emplaceComponent<::rtype::games::rtype::shared::TransformComponent>(
            entity, event.x, event.y);
        reg.emplaceComponent<::rtype::games::rtype::shared::VelocityComponent>(
            entity, 0.f, 0.f);
        reg.emplaceComponent<::rtype::games::rtype::shared::NetworkIdComponent>(
            entity, event.entityId);

        switch (event.type) {
            case ::rtype::network::EntityType::Player: {
                uint32_t playerId = userIdToPlayerId(event.userId);
                setupPlayerEntity(reg, assetsManager, entity, playerId);
                break;
            }

            case ::rtype::network::EntityType::Bydos:
                setupBydosEntity(
                    reg, assetsManager, entity,
                    static_cast<shared::EnemyVariant>(event.subType));
                break;

            case ::rtype::network::EntityType::Missile:
                setupMissileEntity(reg, assetsManager, entity);
                break;

            case ::rtype::network::EntityType::Pickup:
                setupPickupEntity(reg, assetsManager, entity, event.entityId,
                                  event.subType);
                break;

            case ::rtype::network::EntityType::Obstacle:
                setupObstacleEntity(reg, assetsManager, entity, event.entityId);
                break;

            case ::rtype::network::EntityType::ForcePod:
                setupForcePodEntity(reg, assetsManager, entity);
                break;
        }

        return entity;
    };
}

void RtypeEntityFactory::setupPlayerEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity, std::uint32_t userId) {
    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] Adding Player components for entity "
                      << entity.id);

    uint32_t playerId = 1;
    if (userId < 1 || userId > ::rtype::game::config::MAX_PLAYER_COUNT) {
        LOG_ERROR_CAT(::rtype::LogCategory::ECS,
                      "[RtypeEntityFactory] Invalid userId "
                          << userId << ", must be 1-"
                          << ::rtype::game::config::MAX_PLAYER_COUNT
                          << " Defaulting to 1");
        userId = 1;
    }
    playerId = userId;

    std::pair<int, int> spriteOffset = getPlayerSpriteOffset(playerId);
    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] Player "
                      << playerId << " sprite offset: (" << spriteOffset.first
                      << ", " << spriteOffset.second << ")");

    reg.emplaceComponent<shared::PlayerIdComponent>(entity, playerId);

    constexpr int neutralColumn = 2;
    constexpr int width = PlayerAnimationSystem::kFrameWidth;
    constexpr int height = PlayerAnimationSystem::kFrameHeight;

    const int left = neutralColumn * width;
    const int top = spriteOffset.second;

    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] Getting player_vessel texture");
    reg.emplaceComponent<Image>(entity, "player_vessel");
    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] Image component added");
    reg.emplaceComponent<TextureRect>(entity, std::pair<int, int>({left, top}),
                                      std::pair<int, int>({width, height}));
    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] TextureRect set to: left="
                      << left << " top=" << top << " width=" << width
                      << " height=" << height);
    reg.emplaceComponent<Size>(entity, 4.0f, 4.0f);
    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] Size component added");

    auto& configRegistry = shared::EntityConfigRegistry::getInstance();
    auto playerConfigOpt = configRegistry.getPlayer("default_ship");

    float hitboxWidth = 132.0f;
    float hitboxHeight = 68.0f;
    int32_t health = 1;

    if (playerConfigOpt.has_value()) {
        const auto& playerConfig = playerConfigOpt.value().get();
        hitboxWidth = playerConfig.hitboxWidth;
        hitboxHeight = playerConfig.hitboxHeight;
        health = playerConfig.health;
    } else {
        LOG_WARNING(
            "[RtypeEntityFactory] Could not load player config, using fallback "
            "values");
    }

    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, hitboxWidth, hitboxHeight);
    reg.emplaceComponent<shared::HealthComponent>(entity, health, health);
    reg.emplaceComponent<PlayerTag>(entity);
    reg.emplaceComponent<BoxingComponent>(
        entity, ::rtype::display::Vector2f{0, 0},
        ::rtype::display::Vector2f{hitboxWidth, hitboxHeight});
    reg.getComponent<BoxingComponent>(entity).outlineColor =
        ::rtype::display::Color::White();
    reg.getComponent<BoxingComponent>(entity).fillColor = {0, 200, 255, 45};
    reg.emplaceComponent<ZIndex>(entity, 0);
    reg.emplaceComponent<GameTag>(entity);
    reg.emplaceComponent<PlayerSoundComponent>(
        entity, assetsManager->soundManager->get("player_spawn"),
        assetsManager->soundManager->get("player_death"));
    auto lib = reg.getSingleton<std::shared_ptr<AudioLib>>();
    lib->playSFX(assetsManager->soundManager->get("player_spawn"));
}

void RtypeEntityFactory::setupBydosEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity, shared::EnemyVariant subType) {
    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] Adding Bydos components");

    auto variant =
        static_cast<::rtype::games::rtype::shared::EnemyVariant>(subType);
    std::string enemyId =
        ::rtype::games::rtype::shared::EnemyTypeComponent::variantToString(
            variant);

    auto& configRegistry = shared::EntityConfigRegistry::getInstance();
    auto enemyConfigOpt = configRegistry.getEnemy(enemyId);

    float hitboxWidth = 66.0f;
    float hitboxHeight = 68.0f;
    int32_t health = 10;

    if (enemyConfigOpt.has_value()) {
        const auto& enemyConfig = enemyConfigOpt.value().get();
        hitboxWidth = enemyConfig.hitboxWidth;
        hitboxHeight = enemyConfig.hitboxHeight;
        health = enemyConfig.health;
        LOG_DEBUG(
            "[RtypeEntityFactory] Loaded config for enemy type: " << enemyId);
    } else {
        LOG_WARNING("[RtypeEntityFactory] Could not load enemy config for "
                    << enemyId << ", using fallback values");
    }

    reg.emplaceComponent<::rtype::games::rtype::shared::EnemyTypeComponent>(
        entity, variant, enemyId);

    switch (subType) {
        case shared::EnemyVariant::Basic:
            LOG_INFO_CAT(::rtype::LogCategory::ECS,
                         "[RtypeEntityFactory] Setting up Basic Bydos enemy");
            reg.emplaceComponent<Image>(entity, "bdos_enemy_normal");
            reg.emplaceComponent<TextureRect>(entity,
                                              std::pair<int, int>({0, 0}),
                                              std::pair<int, int>({33, 34}));
            reg.emplaceComponent<Animation>(entity, 8, 0.1f, false);
            reg.emplaceComponent<Size>(entity, 2.0f, 2.0f);
            break;
        case shared::EnemyVariant::Shooter:
            LOG_INFO_CAT(::rtype::LogCategory::ECS,
                         "[RtypeEntityFactory] Setting up Shooter Bydos enemy");
            reg.emplaceComponent<Image>(entity, "bdos_enemy_shooter");
            reg.emplaceComponent<TextureRect>(entity,
                                              std::pair<int, int>({0, 0}),
                                              std::pair<int, int>({33, 32}));
            reg.emplaceComponent<Animation>(entity, 7, 0.1f, false);
            reg.emplaceComponent<Size>(entity, 2.0f, 2.0f);
            break;
        case shared::EnemyVariant::Chaser:
            LOG_INFO_CAT(::rtype::LogCategory::ECS,
                         "[RtypeEntityFactory] Setting up Chaser Bydos enemy");
            reg.emplaceComponent<Image>(entity, "bdos_enemy_chaser");
            reg.emplaceComponent<TextureRect>(entity,
                                              std::pair<int, int>({0, 0}),
                                              std::pair<int, int>({113, 369}));
            // 6 frames total: frame 1 = normal, frames 2-6 = explosion sequence
            // oneTime=true so it doesn't loop, but we manually control when it
            // starts
            reg.emplaceComponent<Animation>(entity, 6, 0.12f, true);
            reg.emplaceComponent<Size>(entity, 0.6f, 0.6f);
            reg.emplaceComponent<Rotation>(entity, 0.0f);
            reg.emplaceComponent<ChaserExplosion>(
                entity, false, 0.0f);  // Not exploding at start
            break;
        case shared::EnemyVariant::Wave:
            LOG_INFO_CAT(::rtype::LogCategory::ECS,
                         "[RtypeEntityFactory] Setting up Wave Bydos enemy");
            reg.emplaceComponent<Image>(entity, "bdos_enemy_wave");
            reg.emplaceComponent<TextureRect>(entity,
                                              std::pair<int, int>({0, 0}),
                                              std::pair<int, int>({33, 34}));
            reg.emplaceComponent<Animation>(entity, 8, 0.1f, false);
            reg.emplaceComponent<Size>(entity, 2.0f, 2.0f);
            break;
        case shared::EnemyVariant::Patrol:
            LOG_INFO_CAT(::rtype::LogCategory::ECS,
                         "[RtypeEntityFactory] Setting up Patrol Bydos enemy");
            reg.emplaceComponent<Image>(entity, "bdos_enemy_patrol");
            reg.emplaceComponent<TextureRect>(entity,
                                              std::pair<int, int>({0, 0}),
                                              std::pair<int, int>({33, 36}));
            reg.emplaceComponent<Animation>(entity, 8, 0.1f, false);
            reg.emplaceComponent<Size>(entity, 2.0f, 2.0f);
            break;
        case shared::EnemyVariant::Heavy:
            LOG_INFO_CAT(::rtype::LogCategory::ECS,
                         "[RtypeEntityFactory] Setting up Heavy Bydos enemy");
            reg.emplaceComponent<Image>(entity, "bdos_enemy_heavy");
            reg.emplaceComponent<TextureRect>(entity,
                                              std::pair<int, int>({0, 0}),
                                              std::pair<int, int>({33, 33}));
            reg.emplaceComponent<Size>(entity, 2.0f, 2.0f);
            break;
        default:
            LOG_WARNING_CAT(
                ::rtype::LogCategory::ECS,
                std::string("[RtypeEntityFactory] Unknown Bydos variant, "
                            "defaulting to Bydos normal, type received: " +
                            std::to_string(static_cast<uint8_t>(subType))));
            reg.emplaceComponent<Image>(entity, "bdos_enemy_normal");
            reg.emplaceComponent<TextureRect>(entity,
                                              std::pair<int, int>({0, 0}),
                                              std::pair<int, int>({33, 34}));
            reg.emplaceComponent<Animation>(entity, 8, 0.1f, false);
            reg.emplaceComponent<Size>(entity, 2.0f, 2.0f);
            break;
    }
    if (enemyConfigOpt.has_value()) {
        const auto& enemyConfig = enemyConfigOpt.value().get();
        LOG_INFO("[RtypeEntityFactory] Adding ColorTint: R="
                 << static_cast<int>(enemyConfig.colorR)
                 << " G=" << static_cast<int>(enemyConfig.colorG)
                 << " B=" << static_cast<int>(enemyConfig.colorB)
                 << " A=" << static_cast<int>(enemyConfig.colorA));
        reg.emplaceComponent<ColorTint>(entity, enemyConfig.colorR,
                                        enemyConfig.colorG, enemyConfig.colorB,
                                        enemyConfig.colorA);
    } else {
        LOG_WARNING(
            "[RtypeEntityFactory] Could not load enemy config, adding default "
            "ColorTint (white)");
        reg.emplaceComponent<ColorTint>(entity, 255, 255, 255, 255);
    }
    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, hitboxWidth, hitboxHeight);
    reg.emplaceComponent<BoxingComponent>(
        entity, ::rtype::display::Vector2f{0, 0},
        ::rtype::display::Vector2f{hitboxWidth, hitboxHeight});
    reg.getComponent<BoxingComponent>(entity).outlineColor = {255, 120, 0, 255};
    reg.getComponent<BoxingComponent>(entity).fillColor = {255, 120, 0, 40};
    reg.emplaceComponent<ZIndex>(entity, 0);
    reg.emplaceComponent<shared::HealthComponent>(entity, health, health);
    reg.emplaceComponent<shared::EnemyTag>(entity);
    reg.emplaceComponent<GameTag>(entity);
    reg.emplaceComponent<EnemySoundComponent>(
        entity, assetsManager->soundManager->get("bydos_spawn"),
        assetsManager->soundManager->get("bydos_death"));
    auto lib = reg.getSingleton<std::shared_ptr<AudioLib>>();
    lib->playSFX(assetsManager->soundManager->get("bydos_spawn"));
}

void RtypeEntityFactory::setupMissileEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity) {
    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] Adding Missile components");

    auto& configRegistry = shared::EntityConfigRegistry::getInstance();
    auto projectileConfigOpt = configRegistry.getProjectile("basic_bullet");

    float hitboxWidth = 33.0f;
    float hitboxHeight = 34.0f;

    if (projectileConfigOpt.has_value()) {
        const auto& projectileConfig = projectileConfigOpt.value().get();
        hitboxWidth = projectileConfig.hitboxWidth;
        hitboxHeight = projectileConfig.hitboxHeight;
    } else {
        LOG_WARNING(
            "[RtypeEntityFactory] Could not load projectile config, using "
            "fallback values");
    }
    reg.emplaceComponent<Image>(entity, "projectile_player_laser");
    reg.emplaceComponent<TextureRect>(entity, std::pair<int, int>({0, 0}),
                                      std::pair<int, int>({33, 34}));
    reg.emplaceComponent<Size>(entity, 1.75f, 1.75f);
    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, hitboxWidth, hitboxHeight);
    reg.emplaceComponent<shared::ProjectileTag>(entity);
    reg.emplaceComponent<BoxingComponent>(
        entity, ::rtype::display::Vector2f{0, 0},
        ::rtype::display::Vector2f{hitboxWidth, hitboxHeight});
    reg.emplaceComponent<Animation>(entity, 4, 0.1, false);
    reg.getComponent<BoxingComponent>(entity).outlineColor = {0, 220, 180, 255};
    reg.getComponent<BoxingComponent>(entity).fillColor = {0, 220, 180, 35};
    reg.emplaceComponent<ZIndex>(entity, 1);
    reg.emplaceComponent<shared::LifetimeComponent>(
        entity, GraphicsConfig::LIFETIME_PROJECTILE);
    reg.emplaceComponent<GameTag>(entity);
    if (reg.hasComponent<shared::VelocityComponent>(entity)) {
        const auto& vel = reg.getComponent<shared::VelocityComponent>(entity);
        LOG_DEBUG("[RtypeEntityFactory] Projectile velocity: vx="
                  << vel.vx << " vy=" << vel.vy);
        if (vel.vx < 0.0f) {
            reg.emplaceComponent<Rotation>(entity, 180.0f);
            LOG_DEBUG(
                "[RtypeEntityFactory] Added 180° rotation to enemy projectile");
        }
    }
    auto lib = reg.getSingleton<std::shared_ptr<AudioLib>>();
    lib->playSFX(assetsManager->soundManager->get("laser_sfx"));

    if (reg.hasComponent<shared::TransformComponent>(entity)) {
        const auto& pos = reg.getComponent<shared::TransformComponent>(entity);
        VisualCueFactory::createFlash(reg, {pos.x, pos.y},
                                      ::rtype::display::Color{0, 255, 220, 255},
                                      52.f, 0.25f, 10);
    }
}

void RtypeEntityFactory::setupPickupEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity, std::uint32_t networkId, uint8_t subType) {
    LOG_INFO("[RtypeEntityFactory] *** CREATING PICKUP ENTITY *** networkId="
             << networkId << " subType=" << static_cast<int>(subType));

    auto variant = static_cast<shared::PowerUpVariant>(subType);
    std::string configId =
        shared::PowerUpTypeComponent::variantToString(variant);
    LOG_INFO("[RtypeEntityFactory] Pickup variant=" << configId);

    auto& configRegistry = shared::EntityConfigRegistry::getInstance();
    auto powerUpConfig = configRegistry.getPowerUp(configId);

    if (!powerUpConfig.has_value()) {
        LOG_WARNING(
            "[RtypeEntityFactory] No config found for power-up: " << configId);
        return;
    }

    const auto& config = powerUpConfig->get();
    sf::Color color =
        sf::Color(config.colorR, config.colorG, config.colorB, config.colorA);
    LOG_INFO("[RtypeEntityFactory] Pickup color: R="
             << static_cast<int>(config.colorR)
             << " G=" << static_cast<int>(config.colorG)
             << " B=" << static_cast<int>(config.colorB));

    if (assetsManager && assetsManager->textureManager) {
        try {
            auto texture = assetsManager->textureManager->get(configId);
            reg.emplaceComponent<Image>(entity, configId);
            auto texSize = texture->getSize();

            if (configId == "force_pod") {
                constexpr int frameWidth = 18;
                constexpr int frameHeight = 18;
                constexpr int frameIndex = 0;

                reg.emplaceComponent<TextureRect>(
                    entity, std::pair<int, int>{frameIndex * frameWidth, 0},
                    std::pair<int, int>{frameWidth, frameHeight});
                reg.emplaceComponent<Size>(entity, 2.0f, 2.0f);

                LOG_INFO(
                    "[RtypeEntityFactory] Using force_pod spritesheet frame 0 ("
                    << frameWidth << "x" << frameHeight << ")");

                reg.emplaceComponent<BoxingComponent>(
                    entity, ::rtype::display::Vector2f{0.0f, 0.0f},
                    ::rtype::display::Vector2f{
                        static_cast<float>(frameWidth) * 2.0f,
                        static_cast<float>(frameHeight) * 2.0f});
            } else {
                reg.emplaceComponent<Size>(entity, 1.0f, 1.0f);
                LOG_INFO("[RtypeEntityFactory] Using texture for powerup: "
                         << configId << " size=" << texSize.x << "x"
                         << texSize.y);

                reg.emplaceComponent<BoxingComponent>(
                    entity, ::rtype::display::Vector2f{0.0f, 0.0f},
                    ::rtype::display::Vector2f{static_cast<float>(texSize.x),
                                               static_cast<float>(texSize.y)});
            }

            reg.emplaceComponent<ColorTint>(entity, config.colorR,
                                            config.colorG, config.colorB,
                                            config.colorA);

            reg.getComponent<BoxingComponent>(entity).outlineColor =
                ::rtype::display::Color{color.r, color.g, color.b, 255};
            reg.getComponent<BoxingComponent>(entity).fillColor =
                ::rtype::display::Color{color.r, color.g, color.b, 45};

            reg.emplaceComponent<
                ::rtype::games::rtype::shared::BoundingBoxComponent>(
                entity, config.hitboxWidth, config.hitboxHeight);
        } catch (const std::exception& e) {
            LOG_WARNING("[RtypeEntityFactory] Texture not found for: "
                        << configId << " - using Rectangle fallback");
            ::rtype::display::Color rtypeColor{color.r, color.g, color.b, 255};
            reg.emplaceComponent<Rectangle>(entity,
                                            std::pair<float, float>{24.f, 24.f},
                                            rtypeColor, rtypeColor);
            reg.getComponent<Rectangle>(entity).outlineThickness = 2.f;
            reg.getComponent<Rectangle>(entity).outlineColor =
                ::rtype::display::Color::White();
            reg.emplaceComponent<BoxingComponent>(
                entity, ::rtype::display::Vector2f{0.0f, 0.0f},
                ::rtype::display::Vector2f{24.f, 24.f});
            reg.getComponent<BoxingComponent>(entity).outlineColor = rtypeColor;
            reg.getComponent<BoxingComponent>(entity).fillColor =
                ::rtype::display::Color{color.r, color.g, color.b, 45};

            reg.emplaceComponent<
                ::rtype::games::rtype::shared::BoundingBoxComponent>(
                entity, 24.0f, 24.0f);
        }
    } else {
        LOG_WARNING(
            "[RtypeEntityFactory] No assetsManager - using Rectangle fallback");
        ::rtype::display::Color rtypeColor{color.r, color.g, color.b, 255};
        reg.emplaceComponent<Rectangle>(entity,
                                        std::pair<float, float>{24.f, 24.f},
                                        rtypeColor, rtypeColor);
        reg.getComponent<Rectangle>(entity).outlineThickness = 2.f;
        reg.getComponent<Rectangle>(entity).outlineColor =
            ::rtype::display::Color::White();
        reg.emplaceComponent<BoxingComponent>(
            entity, ::rtype::display::Vector2f{0.0f, 0.0f},
            ::rtype::display::Vector2f{24.f, 24.f});
        reg.getComponent<BoxingComponent>(entity).outlineColor = rtypeColor;
        reg.getComponent<BoxingComponent>(entity).fillColor =
            ::rtype::display::Color{color.r, color.g, color.b, 45};

        reg.emplaceComponent<
            ::rtype::games::rtype::shared::BoundingBoxComponent>(entity, 24.0f,
                                                                 24.0f);
    }

    reg.emplaceComponent<ZIndex>(entity, 0);
    reg.emplaceComponent<GameTag>(entity);

    LOG_INFO("[RtypeEntityFactory] Pickup entity setup complete");
}

void RtypeEntityFactory::setupObstacleEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity, std::uint32_t /*networkId*/) {
    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] Adding Obstacle components");

    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, GraphicsConfig::NBR_MAX_OBSTACLES);

    int value = dist(gen);
    std::string textureName = "projectile" + std::to_string(value);

    reg.emplaceComponent<Image>(entity, textureName);
    reg.emplaceComponent<Size>(entity, 0.5f, 0.5f);
    reg.emplaceComponent<ZIndex>(entity, 0);
    reg.emplaceComponent<GameTag>(entity);
}

void RtypeEntityFactory::setupForcePodEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity) {
    LOG_DEBUG_CAT(::rtype::LogCategory::ECS,
                  "[RtypeEntityFactory] Adding Force Pod components");

    constexpr int frameWidth = 17;
    constexpr int frameHeight = 18;
    constexpr int frameCount = 12;

    auto forcePodTexture = assetsManager->textureManager->get("force_pod");
    reg.emplaceComponent<Image>(entity, "force_pod");
    reg.emplaceComponent<TextureRect>(
        entity, std::pair<int, int>({0, 0}),
        std::pair<int, int>({frameWidth, frameHeight}));
    reg.emplaceComponent<Animation>(entity, frameCount, 0.08f, false);
    reg.emplaceComponent<Size>(entity, 2.0, 2.0);

    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, 32.0f, 32.0f);
    reg.emplaceComponent<BoxingComponent>(
        entity, ::rtype::display::Vector2f{0.0f, 0.0f},
        ::rtype::display::Vector2f{32.0f, 32.0f});
    reg.getComponent<BoxingComponent>(entity).outlineColor =
        ::rtype::display::Color{100, 200, 255, 255};
    reg.getComponent<BoxingComponent>(entity).fillColor =
        ::rtype::display::Color{100, 200, 255, 40};

    reg.emplaceComponent<ForcePodVisual>(entity);
    reg.emplaceComponent<ZIndex>(entity, 1);
    reg.emplaceComponent<GameTag>(entity);
    reg.emplaceComponent<shared::ForcePodTag>(entity);

    LOG_DEBUG_CAT(
        ::rtype::LogCategory::ECS,
        "[RtypeEntityFactory] Force Pod entity created with animation");
}

}  // namespace rtype::games::rtype::client
