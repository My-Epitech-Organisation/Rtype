/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientNetworkSystem - Implementation
*/

#include "ClientNetworkSystem.hpp"

#include <memory>
#include <utility>

namespace rtype::client {

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
    if (networkIdToEntity_.find(event.entityId) != networkIdToEntity_.end()) {
        return;
    }

    ECS::Entity entity;

    if (entityFactory_) {
        entity = entityFactory_(*registry_, event);
    } else {
        entity = defaultEntityFactory(*registry_, event);
    }

    networkIdToEntity_[event.entityId] = entity;

    if (localUserId_.has_value() && event.entityId == *localUserId_ &&
        event.type == network::EntityType::Player) {
        localPlayerEntity_ = entity;

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

    // TODO(SamTess): Implement component updates for entity movement.
    // The EntityFactory should set up TransformComponent and VelocityComponent.
    // Update them here using event.x, event.y, event.vx, event.vy.
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

    // TODO(SamTess): Implement position correction/reconciliation using x, y.
    (void)x;
    (void)y;
}

void ClientNetworkSystem::handleConnected(std::uint32_t userId) {
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
