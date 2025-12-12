/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeEntityFactory.cpp
*/

#include "RtypeEntityFactory.hpp"

#include <memory>
#include <utility>

#include "../shared/Components/HealthComponent.hpp"
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
        }

        return entity;
    };
}

void RtypeEntityFactory::setupPlayerEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity) {
    LOG_DEBUG("[RtypeEntityFactory] Adding Player components");
    reg.emplaceComponent<Image>(
        entity, assetsManager->textureManager->get("player_vessel"));
    reg.emplaceComponent<TextureRect>(entity, std::pair<int, int>({0, 0}),
                                      std::pair<int, int>({33, 17}));
    reg.emplaceComponent<Size>(entity, 4, 4);
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
    reg.emplaceComponent<BoxingComponent>(entity,
                                          sf::FloatRect({0, 0}, {33.f, 34.f}));
    reg.getComponent<BoxingComponent>(entity).outlineColor =
        sf::Color(255, 120, 0);
    reg.getComponent<BoxingComponent>(entity).fillColor =
        sf::Color(255, 120, 0, 40);
    reg.emplaceComponent<ZIndex>(entity, 0);
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
    reg.emplaceComponent<Size>(entity, 2, 2);
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
    reg.emplaceComponent<Size>(entity, 1, 1);
    reg.emplaceComponent<GameTag>(entity);
    auto lib = reg.getSingleton<std::shared_ptr<AudioLib>>();
    lib->playSFX(*assetsManager->soundManager->get("laser_sfx"));

    if (reg.hasComponent<shared::Position>(entity)) {
        const auto& pos = reg.getComponent<shared::Position>(entity);
        VisualCueFactory::createFlash(reg, {pos.x, pos.y},
                                      sf::Color(0, 255, 220), 52.f, 0.25f, 10);
    }
}

}  // namespace rtype::games::rtype::client
