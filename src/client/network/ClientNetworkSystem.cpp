/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientNetworkSystem - Implementation
*/

#include "ClientNetworkSystem.hpp"

#include <memory>
#include <utility>

#include "Components/PositionComponent.hpp"
#include "Components/SoundComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Logger/Macros.hpp"
#include "client/Graphic/AudioLib/AudioLib.hpp"

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

    client_->onPositionCorrection(
        [this](float x, float y) { handlePositionCorrection(x, y); });
}

void ClientNetworkSystem::setEntityFactory(EntityFactory factory) {
    entityFactory_ = std::move(factory);
}

void ClientNetworkSystem::onLocalPlayerAssigned(
    std::function<void(std::uint32_t userId, ECS::Entity entity)> callback) {
    onLocalPlayerAssignedCallback_ = std::move(callback);
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

    if (localUserId_.has_value() && event.entityId == *localUserId_ &&
        event.type == network::EntityType::Player) {
        localPlayerEntity_ = entity;
        LOG_DEBUG("[ClientNetworkSystem] This is our local player!");

        if (onLocalPlayerAssignedCallback_) {
            onLocalPlayerAssignedCallback_(*localUserId_, entity);
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
    }
}

void ClientNetworkSystem::handleEntityDestroy(std::uint32_t entityId) {
    auto it = this->networkIdToEntity_.find(entityId);
    if (it == this->networkIdToEntity_.end()) {
        return;
    }

    ECS::Entity entity = it->second;

    if (this->registry_->isAlive(entity)) {
        if (this->registry_
                ->hasComponent<games::rtype::client::EnemiesSoundComponent>(
                    entity)) {
            auto& soundComp =
                this->registry_
                    ->getComponent<games::rtype::client::EnemiesSoundComponent>(
                        entity);
            auto audioLib =
                this->registry_->getSingleton<std::shared_ptr<AudioLib>>();
            audioLib->playSFX(*soundComp.deathSFX);
        }
        this->registry_->killEntity(entity);
    }

    networkIdToEntity_.erase(it);

    if (this->localPlayerEntity_.has_value() &&
        *this->localPlayerEntity_ == entity) {
        this->localPlayerEntity_.reset();
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

void ClientNetworkSystem::handleConnected(std::uint32_t userId) {
    LOG_INFO("[ClientNetworkSystem] Connected with userId=" +
             std::to_string(userId));
    localUserId_ = userId;
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
}

ECS::Entity ClientNetworkSystem::defaultEntityFactory(
    ECS::Registry& registry, const EntitySpawnEvent& event) {
    // TODO(SamTess): Add components based on event.type (Player, Bydos, etc.)
    auto entity = registry.spawnEntity();
    (void)event;
    return entity;
}

}  // namespace rtype::client
