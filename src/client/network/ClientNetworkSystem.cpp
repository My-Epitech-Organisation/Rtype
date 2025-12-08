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
#include "Components/VelocityComponent.hpp"

namespace rtype::client {

// Type aliases for convenience
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
    std::cout << "[ClientNetworkSystem] Entity spawn received: entityId="
              << event.entityId << " type=" << static_cast<int>(event.type)
              << " pos=(" << event.x << ", " << event.y << ")"
              << " localUserId="
              << (localUserId_.has_value() ? std::to_string(*localUserId_)
                                           : "none")
              << std::endl;

    if (networkIdToEntity_.find(event.entityId) != networkIdToEntity_.end()) {
        std::cout << "[ClientNetworkSystem] Entity already exists, skipping"
                  << std::endl;
        return;
    }

    ECS::Entity entity;

    if (entityFactory_) {
        std::cout << "[ClientNetworkSystem] Using custom entityFactory"
                  << std::endl;
        entity = entityFactory_(*registry_, event);
    } else {
        std::cout << "[ClientNetworkSystem] Using default entityFactory"
                  << std::endl;
        entity = defaultEntityFactory(*registry_, event);
    }

    networkIdToEntity_[event.entityId] = entity;
    std::cout << "[ClientNetworkSystem] Created entity id=" << entity.id
              << std::endl;

    if (localUserId_.has_value() && event.entityId == *localUserId_ &&
        event.type == network::EntityType::Player) {
        localPlayerEntity_ = entity;
        std::cout << "[ClientNetworkSystem] This is our local player!"
                  << std::endl;

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

    // Update Position component if present
    if (registry_->hasComponent<Position>(entity)) {
        auto& pos = registry_->getComponent<Position>(entity);
        pos.x = event.x;
        pos.y = event.y;
    }

    // Update Velocity component if present
    if (registry_->hasComponent<Velocity>(entity)) {
        auto& vel = registry_->getComponent<Velocity>(entity);
        vel.vx = event.vx;
        vel.vy = event.vy;
    }
}

void ClientNetworkSystem::handleEntityDestroy(std::uint32_t entityId) {
    auto it = networkIdToEntity_.find(entityId);
    if (it == networkIdToEntity_.end()) {
        return;
    }

    ECS::Entity entity = it->second;

    if (registry_->isAlive(entity)) {
        registry_->killEntity(entity);
    }

    networkIdToEntity_.erase(it);

    if (localPlayerEntity_.has_value() && *localPlayerEntity_ == entity) {
        localPlayerEntity_.reset();
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

    // Apply server authoritative position correction
    if (registry_->hasComponent<Position>(entity)) {
        auto& pos = registry_->getComponent<Position>(entity);
        pos.x = x;
        pos.y = y;
    }
}

void ClientNetworkSystem::handleConnected(std::uint32_t userId) {
    std::cout << "[ClientNetworkSystem] Connected with userId=" << userId
              << std::endl;
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
