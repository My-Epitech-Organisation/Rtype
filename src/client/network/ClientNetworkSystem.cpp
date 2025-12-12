/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientNetworkSystem - Implementation
*/

#include "ClientNetworkSystem.hpp"

#include <memory>
#include <utility>

#include <SFML/Graphics/Color.hpp>

#include "Components/HealthComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Components/SoundComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Logger/Macros.hpp"
#include "client/Graphic/AudioLib/AudioLib.hpp"
#include "games/rtype/client/Components/BoxingComponent.hpp"
#include "games/rtype/client/Components/ImageComponent.hpp"
#include "games/rtype/client/Components/RectangleComponent.hpp"
#include "games/rtype/client/Components/ZIndexComponent.hpp"
#include "games/rtype/client/GameScene/VisualCueFactory.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/PowerUpComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"

namespace rtype::client {

using Position = rtype::games::rtype::shared::Position;
using Velocity = rtype::games::rtype::shared::VelocityComponent;

ClientNetworkSystem::ClientNetworkSystem(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<NetworkClient> client)
    : registry_(std::move(registry)), client_(std::move(client)) {
    client_->onConnected(
        [this](std::uint32_t userId) { handleConnected(userId); });

    client_->onDisconnected([this](network::DisconnectReason reason) {
        handleDisconnected(reason);
    });

    client_->onEntitySpawn(
        [this](EntitySpawnEvent event) { handleEntitySpawn(event); });

    client_->onEntityMove(
        [this](EntityMoveEvent event) { handleEntityMove(event); });

    client_->onEntityDestroy(
        [this](std::uint32_t entityId) { handleEntityDestroy(entityId); });

    client_->onEntityHealth(
        [this](EntityHealthEvent event) { handleEntityHealth(event); });

    client_->onPowerUpEvent(
        [this](PowerUpEvent event) { handlePowerUpEvent(event); });

    client_->onPositionCorrection(
        [this](float x, float y) { handlePositionCorrection(x, y); });
}

void ClientNetworkSystem::setEntityFactory(EntityFactory factory) {
    entityFactory_ = std::move(factory);
}

void ClientNetworkSystem::onLocalPlayerAssigned(
    std::function<void(std::uint32_t userId, ECS::Entity entity)> callback) {
    onLocalPlayerAssignedCallback_ = std::move(callback);

    if (localUserId_.has_value() && localPlayerEntity_.has_value() &&
        onLocalPlayerAssignedCallback_) {
        LOG_DEBUG(
            "[ClientNetworkSystem] Replaying local player assignment: userId="
            << *localUserId_ << " entity=" << localPlayerEntity_->id);
        onLocalPlayerAssignedCallback_(*localUserId_, *localPlayerEntity_);
    }
}

void ClientNetworkSystem::onHealthUpdate(
    std::function<void(const EntityHealthEvent&)> callback) {
    onHealthUpdateCallback_ = std::move(callback);

    if (localUserId_.has_value() && onHealthUpdateCallback_) {
        auto it = lastKnownHealth_.find(*localUserId_);
        if (it != lastKnownHealth_.end()) {
            EntityHealthEvent event{};
            event.entityId = *localUserId_;
            event.current = it->second.current;
            event.max = it->second.max;
            LOG_DEBUG(
                "[ClientNetworkSystem] Replaying cached health event for local "
                "user "
                << *localUserId_ << ": " << event.current << "/" << event.max);
            onHealthUpdateCallback_(event);
        }
    }
}

void ClientNetworkSystem::update() { client_->poll(); }

void ClientNetworkSystem::sendInput(std::uint8_t inputMask) {
    client_->sendInput(inputMask);
}

std::optional<ECS::Entity> ClientNetworkSystem::getLocalPlayerEntity() const {
    return localPlayerEntity_;
}

std::optional<std::uint32_t> ClientNetworkSystem::getLocalUserId() const {
    return localUserId_;
}

std::optional<ECS::Entity> ClientNetworkSystem::findEntityByNetworkId(
    std::uint32_t networkId) const {
    auto it = networkIdToEntity_.find(networkId);
    if (it != networkIdToEntity_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool ClientNetworkSystem::isConnected() const { return client_->isConnected(); }

void ClientNetworkSystem::handleEntitySpawn(const EntitySpawnEvent& event) {
    LOG_DEBUG(
        "[ClientNetworkSystem] Entity spawn received: entityId=" +
        std::to_string(event.entityId) +
        " type=" + std::to_string(static_cast<int>(event.type)) + " pos=(" +
        std::to_string(event.x) + ", " + std::to_string(event.y) +
        ") localUserId=" +
        (localUserId_.has_value() ? std::to_string(*localUserId_) : "none"));

    if (networkIdToEntity_.find(event.entityId) != networkIdToEntity_.end()) {
        LOG_DEBUG("[ClientNetworkSystem] Entity already exists, skipping");
        return;
    }

    ECS::Entity entity;

    if (entityFactory_) {
        LOG_DEBUG("[ClientNetworkSystem] Using custom entityFactory");
        entity = entityFactory_(*registry_, event);
    } else {
        LOG_DEBUG("[ClientNetworkSystem] Using default entityFactory");
        entity = defaultEntityFactory(*registry_, event);
    }

    networkIdToEntity_[event.entityId] = entity;
    LOG_DEBUG("[ClientNetworkSystem] Created entity id=" +
              std::to_string(entity.id));

    if (event.type == network::EntityType::Player) {
        if (localUserId_.has_value() && event.entityId == *localUserId_) {
            localPlayerEntity_ = entity;
            LOG_DEBUG("[ClientNetworkSystem] This is our local player!");

            if (onLocalPlayerAssignedCallback_) {
                onLocalPlayerAssignedCallback_(*localUserId_, entity);
            }
        } else {
            pendingPlayerSpawns_[event.entityId] = entity;
            LOG_DEBUG(
                "[ClientNetworkSystem] Stored pending player spawn: entityId=" +
                std::to_string(event.entityId));
        }
    }
}

void ClientNetworkSystem::handleEntityMove(const EntityMoveEvent& event) {
    auto it = networkIdToEntity_.find(event.entityId);
    if (it == networkIdToEntity_.end()) {
        return;
    }

    ECS::Entity entity = it->second;

    if (!registry_->isAlive(entity)) {
        networkIdToEntity_.erase(it);
        return;
    }

    if (registry_->hasComponent<Position>(entity)) {
        auto& pos = registry_->getComponent<Position>(entity);
        pos.x = event.x;
        pos.y = event.y;
    }

    if (registry_->hasComponent<Velocity>(entity)) {
        auto& vel = registry_->getComponent<Velocity>(entity);
        vel.vx = event.vx;
        vel.vy = event.vy;

        if (registry_->hasComponent<games::rtype::shared::ProjectileTag>(
                entity) &&
            registry_->hasComponent<games::rtype::client::Image>(entity)) {
            auto& img =
                registry_->getComponent<games::rtype::client::Image>(entity);
            const bool isEnemyShot = vel.vx < 0.0F;
            img.sprite.setColor(isEnemyShot ? sf::Color(255, 80, 80)
                                            : sf::Color(80, 255, 240));

            if (registry_->hasComponent<games::rtype::client::BoxingComponent>(
                    entity)) {
                auto& box =
                    registry_
                        ->getComponent<games::rtype::client::BoxingComponent>(
                            entity);
                box.outlineColor = isEnemyShot ? sf::Color(255, 80, 80)
                                               : sf::Color(0, 220, 180);
                box.fillColor = isEnemyShot ? sf::Color(255, 80, 80, 40)
                                            : sf::Color(0, 220, 180, 35);
            }
        }
    }
}

void ClientNetworkSystem::_playDeathSound(ECS::Entity entity) {
    if (registry_->hasComponent<games::rtype::client::EnemySoundComponent>(
            entity)) {
        auto& soundComp =
            registry_->getComponent<games::rtype::client::EnemySoundComponent>(
                entity);
        auto audioLib = registry_->getSingleton<std::shared_ptr<AudioLib>>();
        audioLib->playSFX(*soundComp.deathSFX);
    }
    if (registry_->hasComponent<games::rtype::shared::Position>(entity)) {
        const auto& pos =
            registry_->getComponent<games::rtype::shared::Position>(entity);
        games::rtype::client::VisualCueFactory::createFlash(
            *registry_, {pos.x, pos.y}, sf::Color(255, 80, 0), 90.f, 0.45f, 20);
    }
    if (registry_->hasComponent<games::rtype::client::PlayerSoundComponent>(
            entity)) {
        auto& soundComp =
            registry_->getComponent<games::rtype::client::PlayerSoundComponent>(
                entity);
        auto audioLib = registry_->getSingleton<std::shared_ptr<AudioLib>>();
        audioLib->playSFX(*soundComp.deathSFX);
    }
}

void ClientNetworkSystem::handleEntityDestroy(std::uint32_t entityId) {
    LOG_DEBUG("[ClientNetworkSystem] Entity destroy received: entityId=" +
              std::to_string(entityId));

    auto it = networkIdToEntity_.find(entityId);
    if (it == networkIdToEntity_.end()) {
        LOG_DEBUG("[ClientNetworkSystem] Entity not found in map, skipping");
        return;
    }

    ECS::Entity entity = it->second;

    if (registry_->isAlive(entity)) {
        _playDeathSound(entity);
        registry_->killEntity(entity);
        LOG_DEBUG("[ClientNetworkSystem] Entity killed");
    }

    this->networkIdToEntity_.erase(it);
    lastKnownHealth_.erase(entityId);

    if (localPlayerEntity_.has_value() && *localPlayerEntity_ == entity) {
        localPlayerEntity_.reset();
        LOG_DEBUG("[ClientNetworkSystem] Local player entity reset!");
    }
}

void ClientNetworkSystem::handlePositionCorrection(float x, float y) {
    if (!localPlayerEntity_.has_value()) {
        return;
    }

    ECS::Entity entity = *localPlayerEntity_;

    if (!registry_->isAlive(entity)) {
        localPlayerEntity_.reset();
        return;
    }

    if (registry_->hasComponent<Position>(entity)) {
        auto& pos = registry_->getComponent<Position>(entity);
        pos.x = x;
        pos.y = y;
    }
}

void ClientNetworkSystem::handleEntityHealth(const EntityHealthEvent& event) {
    LOG_DEBUG("[ClientNetworkSystem] handleEntityHealth: entityId="
              << event.entityId << " current=" << event.current
              << " max=" << event.max);

    const auto prevIt = lastKnownHealth_.find(event.entityId);
    std::optional<int32_t> previousHealth = std::nullopt;
    if (prevIt != lastKnownHealth_.end()) {
        previousHealth = prevIt->second.current;
    }

    auto it = networkIdToEntity_.find(event.entityId);
    if (it != networkIdToEntity_.end()) {
        ECS::Entity entity = it->second;
        if (registry_->isAlive(entity)) {
            if (registry_->hasComponent<
                    rtype::games::rtype::shared::HealthComponent>(entity)) {
                auto& health = registry_->getComponent<
                    rtype::games::rtype::shared::HealthComponent>(entity);
                previousHealth = health.current;
                health.current = event.current;
                health.max = event.max;
            } else {
                registry_->emplaceComponent<
                    rtype::games::rtype::shared::HealthComponent>(
                    entity, event.current, event.max);
            }

            if (previousHealth.has_value() &&
                previousHealth.value() > event.current &&
                registry_->hasComponent<rtype::games::rtype::shared::Position>(
                    entity)) {
                const auto& pos =
                    registry_
                        ->getComponent<rtype::games::rtype::shared::Position>(
                            entity);
                games::rtype::client::VisualCueFactory::createFlash(
                    *registry_, {pos.x, pos.y}, sf::Color(255, 80, 80), 70.f,
                    0.25f, 12);
            }
        }
    } else {
        LOG_DEBUG("[ClientNetworkSystem] Entity "
                  << event.entityId << " not found in networkIdToEntity_ map");
    }

    lastKnownHealth_[event.entityId] = {event.current, event.max};

    if (onHealthUpdateCallback_) {
        LOG_DEBUG("[ClientNetworkSystem] Calling onHealthUpdateCallback_");
        onHealthUpdateCallback_(event);
    } else {
        LOG_DEBUG(
            "[ClientNetworkSystem] No onHealthUpdateCallback_ registered!");
    }
}

void ClientNetworkSystem::handlePowerUpEvent(const PowerUpEvent& event) {
    auto it = networkIdToEntity_.find(event.playerId);
    if (it == networkIdToEntity_.end()) {
        return;
    }

    ECS::Entity entity = it->second;
    if (!registry_->isAlive(entity)) {
        return;
    }

    const auto powerUpType =
        static_cast<rtype::games::rtype::shared::PowerUpType>(
            event.powerUpType);

    auto& active =
        registry_->hasComponent<
            rtype::games::rtype::shared::ActivePowerUpComponent>(entity)
            ? registry_->getComponent<
                  rtype::games::rtype::shared::ActivePowerUpComponent>(entity)
            : registry_->emplaceComponent<
                  rtype::games::rtype::shared::ActivePowerUpComponent>(entity);

    active.type = powerUpType;
    active.remainingTime = event.duration;
    active.speedMultiplier = 1.0F;
    active.fireRateMultiplier = 1.0F;
    active.damageMultiplier = 1.0F;
    active.shieldActive =
        powerUpType == rtype::games::rtype::shared::PowerUpType::Shield;
    active.hasOriginalCooldown = false;

    if (registry_->hasComponent<rtype::games::rtype::shared::Position>(
            entity)) {
        const auto& pos =
            registry_->getComponent<rtype::games::rtype::shared::Position>(
                entity);
        sf::Color cueColor = sf::Color(180, 240, 255);
        switch (powerUpType) {
            case rtype::games::rtype::shared::PowerUpType::Shield:
                cueColor = sf::Color(255, 215, 0);
                break;
            case rtype::games::rtype::shared::PowerUpType::SpeedBoost:
                cueColor = sf::Color(120, 255, 200);
                break;
            case rtype::games::rtype::shared::PowerUpType::RapidFire:
                cueColor = sf::Color(120, 200, 255);
                break;
            case rtype::games::rtype::shared::PowerUpType::DoubleDamage:
                cueColor = sf::Color(255, 150, 150);
                break;
            case rtype::games::rtype::shared::PowerUpType::HealthBoost:
                cueColor = sf::Color(220, 180, 255);
                break;
            default:
                break;
        }

        games::rtype::client::VisualCueFactory::createFlash(
            *registry_, {pos.x, pos.y}, cueColor, 80.f, 0.35f, 14);
    }
}

void ClientNetworkSystem::handleConnected(std::uint32_t userId) {
    LOG_INFO("[ClientNetworkSystem] Connected with userId=" +
             std::to_string(userId));
    localUserId_ = userId;

    auto pendingIt = pendingPlayerSpawns_.find(userId);
    if (pendingIt != pendingPlayerSpawns_.end()) {
        localPlayerEntity_ = pendingIt->second;
        LOG_INFO(
            "[ClientNetworkSystem] Found pending player spawn for our userId=" +
            std::to_string(userId) +
            " -> entity=" + std::to_string(localPlayerEntity_->id));

        if (onLocalPlayerAssignedCallback_) {
            onLocalPlayerAssignedCallback_(*localUserId_, *localPlayerEntity_);
        }

        auto healthIt = lastKnownHealth_.find(userId);
        if (healthIt != lastKnownHealth_.end() && onHealthUpdateCallback_) {
            EntityHealthEvent event{};
            event.entityId = userId;
            event.current = healthIt->second.current;
            event.max = healthIt->second.max;
            LOG_INFO(
                "[ClientNetworkSystem] Replaying cached health for newly "
                "assigned player: "
                << event.current << "/" << event.max);
            onHealthUpdateCallback_(event);
        }
    }

    pendingPlayerSpawns_.clear();
}

void ClientNetworkSystem::handleDisconnected(network::DisconnectReason reason) {
    (void)reason;

    for (auto& [networkId, entity] : networkIdToEntity_) {
        if (registry_->isAlive(entity)) {
            registry_->killEntity(entity);
        }
    }

    networkIdToEntity_.clear();
    localUserId_.reset();
    localPlayerEntity_.reset();
    pendingPlayerSpawns_.clear();
    lastKnownHealth_.clear();
}

ECS::Entity ClientNetworkSystem::defaultEntityFactory(
    ECS::Registry& registry, const EntitySpawnEvent& event) {
    auto entity = registry.spawnEntity();

    registry.emplaceComponent<Position>(entity, event.x, event.y);
    registry.emplaceComponent<Velocity>(entity, 0.f, 0.f);
    registry.emplaceComponent<rtype::games::rtype::shared::NetworkIdComponent>(
        entity, event.entityId);

    switch (event.type) {
        case network::EntityType::Pickup: {
            const sf::Color color(140, 220, 255);
            registry.emplaceComponent<games::rtype::client::Rectangle>(
                entity, std::pair<float, float>{22.f, 22.f}, color, color);
            registry.emplaceComponent<games::rtype::client::BoxingComponent>(
                entity, sf::FloatRect({0, 0}, {22.f, 22.f}));
            registry.emplaceComponent<games::rtype::client::ZIndex>(entity, 0);
            break;
        }
        case network::EntityType::Obstacle: {
            const sf::Color color(160, 160, 170);
            registry.emplaceComponent<games::rtype::client::Rectangle>(
                entity, std::pair<float, float>{48.f, 48.f}, color, color);
            registry.emplaceComponent<games::rtype::client::BoxingComponent>(
                entity, sf::FloatRect({0, 0}, {48.f, 48.f}));
            registry.emplaceComponent<games::rtype::client::ZIndex>(entity, 0);
            break;
        }
        default:
            break;
    }

    return entity;
}

}  // namespace rtype::client
