/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeEntityFactory.cpp
*/

#include "RtypeEntityFactory.hpp"

#include <memory>
#include <utility>

#include "../Components/RectangleComponent.hpp"
#include "../Systems/PlayerAnimationSystem.hpp"
#include "../shared/Components/BoundingBoxComponent.hpp"
#include "../shared/Components/HealthComponent.hpp"
#include "../shared/Components/NetworkIdComponent.hpp"
#include "AllComponents.hpp"
#include "AudioLib/AudioLib.hpp"
#include "Components/LifetimeComponent.hpp"
#include "Components/SoundComponent.hpp"
#include "Components/Tags.hpp"
#include "Graphic/ControllerRumble.hpp"
#include "GraphicsConstants.hpp"
#include "Logger/Macros.hpp"
#include "VisualCueFactory.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::games::rtype::client {

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

        reg.emplaceComponent<::rtype::games::rtype::shared::Position>(
            entity, event.x, event.y);
        reg.emplaceComponent<::rtype::games::rtype::shared::VelocityComponent>(
            entity, 0.f, 0.f);
        reg.emplaceComponent<::rtype::games::rtype::shared::NetworkIdComponent>(
            entity, event.entityId);

        switch (event.type) {
            case ::rtype::network::EntityType::Player:
                setupPlayerEntity(reg, assetsManager, entity);
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
                setupObstacleEntity(reg, entity, event.entityId);
                break;
        }

        return entity;
    };
}

void RtypeEntityFactory::setupPlayerEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity) {
    LOG_DEBUG("[RtypeEntityFactory] Adding Player components");
    const auto networkId =
        reg.getComponent<shared::NetworkIdComponent>(entity).networkId;
    const int colorRow =
        (PlayerAnimationSystem::kColorRows > 0)
            ? static_cast<int>(networkId % PlayerAnimationSystem::kColorRows)
            : 0;
    constexpr int neutralColumn = 2;
    constexpr int width = PlayerAnimationSystem::kFrameWidth;
    constexpr int height = PlayerAnimationSystem::kFrameHeight;

    const int left = neutralColumn * width;
    const int top = colorRow * height;

    reg.emplaceComponent<Image>(
        entity, assetsManager->textureManager->get("player_vessel"));
    reg.emplaceComponent<TextureRect>(entity, std::pair<int, int>({left, top}),
                                      std::pair<int, int>({width, height}));
    reg.emplaceComponent<Size>(entity, 4, 4);
    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, 128.0f, 64.0f);
    reg.emplaceComponent<shared::HealthComponent>(entity, 1, 1);
    reg.emplaceComponent<PlayerTag>(entity);
    reg.emplaceComponent<BoxingComponent>(entity,
                                          sf::FloatRect({0, 0}, {33.f, 17.f}));
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
        entity, 32.0f, 32.0f);
    reg.emplaceComponent<BoxingComponent>(entity,
                                          sf::FloatRect({0, 0}, {33.f, 34.f}));
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
    reg.emplaceComponent<Size>(entity, 1, 1);
    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, 8.0f, 4.0f);
    reg.emplaceComponent<shared::ProjectileTag>(entity);
    reg.emplaceComponent<BoxingComponent>(entity,
                                          sf::FloatRect({0, 0}, {33.f, 34.f}));
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

    if (reg.hasComponent<shared::Position>(entity)) {
        const auto& pos = reg.getComponent<shared::Position>(entity);
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

void RtypeEntityFactory::setupObstacleEntity(ECS::Registry& reg,
                                             ECS::Entity entity,
                                             std::uint32_t /*networkId*/) {
    LOG_DEBUG("[RtypeEntityFactory] Adding Obstacle components");
    const sf::Color main = sf::Color(110, 110, 120);
    const sf::Color outline = sf::Color(200, 200, 210);

    reg.emplaceComponent<Rectangle>(entity, std::pair<float, float>{64.f, 64.f},
                                    main, main);
    reg.getComponent<Rectangle>(entity).outlineThickness = 2.f;
    reg.getComponent<Rectangle>(entity).outlineColor = outline;
    reg.emplaceComponent<BoxingComponent>(entity,
                                          sf::FloatRect({0, 0}, {64.f, 64.f}));
    reg.getComponent<BoxingComponent>(entity).outlineColor = outline;
    reg.getComponent<BoxingComponent>(entity).fillColor =
        sf::Color(outline.r, outline.g, outline.b, 35);
    reg.emplaceComponent<ZIndex>(entity, 0);
    reg.emplaceComponent<GameTag>(entity);

    reg.emplaceComponent<::rtype::games::rtype::shared::BoundingBoxComponent>(
        entity, 64.0f, 64.0f);
}

}  // namespace rtype::games::rtype::client
