/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RtypeEntityFactory.cpp
*/

#include "RtypeEntityFactory.hpp"

#include <iostream>
#include <utility>

#include "AllComponents.hpp"
#include "protocol/Payloads.hpp"

namespace rtype::games::rtype::client {

::rtype::client::ClientNetworkSystem::EntityFactory
RtypeEntityFactory::createNetworkEntityFactory(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<AssetManager> assetsManager) {
    return [assetsManager, registry](
               ECS::Registry& reg,
               const ::rtype::client::EntitySpawnEvent& event) -> ECS::Entity {
        std::cout << "[RtypeEntityFactory] Creating entity type="
                  << static_cast<int>(event.type) << " pos=(" << event.x << ", "
                  << event.y << ")" << std::endl;

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
                setupMissileEntity(reg, entity);
                break;
        }

        return entity;
    };
}

void RtypeEntityFactory::setupPlayerEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity) {
    std::cout << "[RtypeEntityFactory] Adding Player components" << std::endl;
    reg.emplaceComponent<Image>(
        entity, assetsManager->textureManager->get("player_vessel"));
    reg.emplaceComponent<TextureRect>(entity, std::pair<int, int>({0, 0}),
                                      std::pair<int, int>({33, 17}));
    reg.emplaceComponent<Size>(entity, 4, 4);
    reg.emplaceComponent<PlayerTag>(entity);
    reg.emplaceComponent<ZIndex>(entity, 0);
}

void RtypeEntityFactory::setupBydosEntity(
    ECS::Registry& reg, std::shared_ptr<AssetManager> assetsManager,
    ECS::Entity entity) {
    std::cout << "[RtypeEntityFactory] Adding Bydos components" << std::endl;
    // TODO(Noa): Add Bydos enemy sprite when available
    reg.emplaceComponent<Image>(
        entity, assetsManager->textureManager->get("player_vessel"));
    reg.emplaceComponent<TextureRect>(entity, std::pair<int, int>({0, 0}),
                                      std::pair<int, int>({33, 17}));
    reg.emplaceComponent<Size>(entity, 3, 3);
    reg.emplaceComponent<ZIndex>(entity, 0);
}

void RtypeEntityFactory::setupMissileEntity(ECS::Registry& reg,
                                            ECS::Entity entity) {
    std::cout << "[RtypeEntityFactory] Adding Missile components" << std::endl;
    // TODO(Noa): Add Missile sprite when available
    reg.emplaceComponent<Size>(entity, 1, 1);
    reg.emplaceComponent<ZIndex>(entity, 1);
}

}  // namespace rtype::games::rtype::client
