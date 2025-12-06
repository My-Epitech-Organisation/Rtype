/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerNetworkSystem - Implementation
*/

#include "ServerNetworkSystem.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace rtype::server {

ServerNetworkSystem::ServerNetworkSystem(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<NetworkServer> server)
    : registry_(std::move(registry)), server_(std::move(server)) {
    server_->onClientConnected(
        [this](std::uint32_t userId) { handleClientConnected(userId); });

    server_->onClientDisconnected(
        [this](std::uint32_t userId, network::DisconnectReason reason) {
            handleClientDisconnected(userId, reason);
        });

    server_->onClientInput([this](std::uint32_t userId, std::uint8_t input) {
        handleClientInput(userId, input);
    });

    server_->onGetUsersRequest(
        [this](std::uint32_t userId) { handleGetUsersRequest(userId); });
}

void ServerNetworkSystem::registerNetworkedEntity(ECS::Entity entity,
                                                  std::uint32_t networkId,
                                                  EntityType type, float x,
                                                  float y) {
    NetworkedEntity info;
    info.entity = entity;
    info.networkId = networkId;
    info.type = type;
    info.lastX = x;
    info.lastY = y;
    info.lastVx = 0;
    info.lastVy = 0;
    info.dirty = false;

    networkedEntities_[networkId] = info;
    entityToNetworkId_[entity.id] = networkId;

    server_->spawnEntity(networkId, type, x, y);
}

void ServerNetworkSystem::unregisterNetworkedEntity(ECS::Entity entity) {
    auto it = entityToNetworkId_.find(entity.id);
    if (it == entityToNetworkId_.end()) {
        return;
    }

    std::uint32_t networkId = it->second;
    unregisterNetworkedEntityById(networkId);
}

void ServerNetworkSystem::unregisterNetworkedEntityById(
    std::uint32_t networkId) {
    auto it = networkedEntities_.find(networkId);
    if (it == networkedEntities_.end()) {
        return;
    }

    entityToNetworkId_.erase(it->second.entity.id);
    networkedEntities_.erase(it);

    server_->destroyEntity(networkId);
}

void ServerNetworkSystem::setPlayerEntity(std::uint32_t userId,
                                          ECS::Entity entity) {
    userIdToEntity_[userId] = entity;
}

std::optional<ECS::Entity> ServerNetworkSystem::getPlayerEntity(
    std::uint32_t userId) const {
    auto it = userIdToEntity_.find(userId);
    if (it != userIdToEntity_.end()) {
        return it->second;
    }
    return std::nullopt;
}

void ServerNetworkSystem::setInputHandler(InputHandler handler) {
    inputHandler_ = std::move(handler);
}

void ServerNetworkSystem::onClientConnected(
    std::function<void(std::uint32_t userId)> callback) {
    onClientConnectedCallback_ = std::move(callback);
}

void ServerNetworkSystem::onClientDisconnected(
    std::function<void(std::uint32_t userId)> callback) {
    onClientDisconnectedCallback_ = std::move(callback);
}

void ServerNetworkSystem::updateEntityPosition(std::uint32_t networkId, float x,
                                               float y, float vx, float vy) {
    auto it = networkedEntities_.find(networkId);
    if (it == networkedEntities_.end()) {
        return;
    }

    it->second.lastX = x;
    it->second.lastY = y;
    it->second.lastVx = vx;
    it->second.lastVy = vy;
    it->second.dirty = true;
}

void ServerNetworkSystem::correctPlayerPosition(std::uint32_t userId, float x,
                                                float y) {
    server_->correctPosition(userId, x, y);
}

void ServerNetworkSystem::broadcastEntityUpdates() {
    for (auto& [networkId, info] : networkedEntities_) {
        if (info.dirty) {
            server_->moveEntity(networkId, info.lastX, info.lastY, info.lastVx,
                                info.lastVy);
            info.dirty = false;
        }
    }
}

void ServerNetworkSystem::update() {
    server_->poll();

    processExpiredGracePeriods();

    std::vector<std::uint32_t> toRemove;
    for (auto& [networkId, info] : networkedEntities_) {
        if (!registry_->isAlive(info.entity)) {
            toRemove.push_back(networkId);
        }
    }

    for (std::uint32_t networkId : toRemove) {
        unregisterNetworkedEntityById(networkId);
    }
}

std::optional<std::uint32_t> ServerNetworkSystem::getNetworkId(
    ECS::Entity entity) const {
    auto it = entityToNetworkId_.find(entity.id);
    if (it != entityToNetworkId_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::optional<ECS::Entity> ServerNetworkSystem::findEntityByNetworkId(
    std::uint32_t networkId) const {
    auto it = networkedEntities_.find(networkId);
    if (it != networkedEntities_.end()) {
        return it->second.entity;
    }
    return std::nullopt;
}

std::uint32_t ServerNetworkSystem::nextNetworkId() {
    return nextNetworkIdCounter_++;
}

void ServerNetworkSystem::handleClientConnected(std::uint32_t userId) {
    auto pendingIt = pendingDisconnections_.find(userId);
    if (pendingIt != pendingDisconnections_.end()) {
        pendingDisconnections_.erase(pendingIt);
    }

    for (const auto& [networkId, info] : networkedEntities_) {
        server_->spawnEntityToClient(userId, networkId, info.type, info.lastX,
                                     info.lastY);
    }

    if (onClientConnectedCallback_) {
        onClientConnectedCallback_(userId);
    }
}

void ServerNetworkSystem::handleClientDisconnected(
    std::uint32_t userId, network::DisconnectReason reason) {
    bool useGracePeriod =
        (reason == network::DisconnectReason::Timeout ||
         reason == network::DisconnectReason::MaxRetriesExceeded);

    if (useGracePeriod) {
        auto it = userIdToEntity_.find(userId);
        if (it != userIdToEntity_.end()) {
            PendingDisconnection pending;
            pending.disconnectTime = std::chrono::steady_clock::now();
            pending.playerEntity = it->second;
            auto networkIdOpt = getNetworkId(it->second);
            pending.networkId = networkIdOpt.value_or(0);
            pendingDisconnections_[userId] = pending;
        }
        return;
    }

    finalizeDisconnection(userId);
}

void ServerNetworkSystem::processExpiredGracePeriods() {
    auto now = std::chrono::steady_clock::now();
    std::vector<std::uint32_t> expiredUsers;

    for (const auto& [userId, pending] : pendingDisconnections_) {
        auto elapsed = now - pending.disconnectTime;
        if (elapsed >= kDisconnectGracePeriod) {
            expiredUsers.push_back(userId);
        }
    }

    for (std::uint32_t userId : expiredUsers) {
        pendingDisconnections_.erase(userId);
        finalizeDisconnection(userId);
    }
}

void ServerNetworkSystem::finalizeDisconnection(std::uint32_t userId) {
    auto it = userIdToEntity_.find(userId);
    if (it != userIdToEntity_.end()) {
        ECS::Entity entity = it->second;
        auto networkIdOpt = getNetworkId(entity);
        if (networkIdOpt) {
            unregisterNetworkedEntityById(*networkIdOpt);
        }
        userIdToEntity_.erase(it);
    }

    if (onClientDisconnectedCallback_) {
        onClientDisconnectedCallback_(userId);
    }
}

void ServerNetworkSystem::handleClientInput(std::uint32_t userId,
                                            std::uint8_t inputMask) {
    if (!inputHandler_) {
        return;
    }

    auto entity = getPlayerEntity(userId);
    inputHandler_(userId, inputMask, entity);
}

void ServerNetworkSystem::handleGetUsersRequest(std::uint32_t userId) {
    auto connectedClients = server_->getConnectedClients();
    server_->sendUserList(userId, connectedClients);
}

}  // namespace rtype::server
