/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerNetworkSystem - Implementation
*/

#include "ServerNetworkSystem.hpp"

#include <cmath>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "Logger/Macros.hpp"
#include "games/rtype/server/GameEngine.hpp"
#include "games/rtype/shared/Components.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"

namespace rtype::server {

static constexpr float VIEWPORT_WIDTH =
    rtype::games::rtype::server::GameConfig::SCREEN_WIDTH;
static constexpr float VIEWPORT_HEIGHT =
    rtype::games::rtype::server::GameConfig::SCREEN_HEIGHT;
static constexpr float VIEWPORT_MARGIN = 100.0F;

// ============================================================================
// NORMAL BANDWIDTH MODE - Full update rates for good connections
// ============================================================================
namespace NormalMode {
static constexpr std::uint32_t PLAYER_UPDATE_INTERVAL = 1;
static constexpr std::uint32_t ENEMY_UPDATE_INTERVAL = 4;
static constexpr std::uint32_t PROJECTILE_UPDATE_INTERVAL = 2;
static constexpr float PLAYER_POSITION_DELTA = 20.0F;
static constexpr float PLAYER_VELOCITY_DELTA = 50.0F;
static constexpr float ENEMY_POSITION_DELTA = 30.0F;
static constexpr float ENEMY_VELOCITY_DELTA = 60.0F;
static constexpr float PROJECTILE_POSITION_DELTA = 40.0F;
static constexpr float PROJECTILE_VELOCITY_DELTA = 80.0F;
}  // namespace NormalMode

// ============================================================================
// LOW BANDWIDTH MODE - Reduced rates for constrained connections (~5 KB/s)
// ============================================================================
namespace LowBandwidthMode {
static constexpr std::uint32_t PLAYER_UPDATE_INTERVAL = 6;
static constexpr std::uint32_t ENEMY_UPDATE_INTERVAL = 180;
static constexpr std::uint32_t PROJECTILE_UPDATE_INTERVAL = 360;
static constexpr float PLAYER_POSITION_DELTA = 40.0F;
static constexpr float PLAYER_VELOCITY_DELTA = 80.0F;
static constexpr float ENEMY_POSITION_DELTA = 200.0F;
static constexpr float ENEMY_VELOCITY_DELTA = 200.0F;
static constexpr float PROJECTILE_POSITION_DELTA = 300.0F;
static constexpr float PROJECTILE_VELOCITY_DELTA = 250.0F;
}  // namespace LowBandwidthMode

static bool isEntityVisible(float x, float y) {
    return x >= -VIEWPORT_MARGIN && x <= VIEWPORT_WIDTH + VIEWPORT_MARGIN &&
           y >= -VIEWPORT_MARGIN && y <= VIEWPORT_HEIGHT + VIEWPORT_MARGIN;
}

ServerNetworkSystem::ServerNetworkSystem(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<NetworkServer> server)
    : registry_(std::move(registry)), server_(std::move(server)) {
    if (server_) {
        server_->onClientConnected(
            [this](std::uint32_t userId) { handleClientConnected(userId); });

        server_->onClientDisconnected(
            [this](std::uint32_t userId, network::DisconnectReason reason) {
                handleClientDisconnected(userId, reason);
            });

        server_->onClientInput(
            [this](std::uint32_t userId, std::uint8_t input) {
                handleClientInput(userId, input);
            });

        server_->onGetUsersRequest(
            [this](std::uint32_t userId) { handleGetUsersRequest(userId); });

        server_->onBandwidthModeChanged([this](std::uint32_t userId,
                                               bool lowBandwidth) {
            if (lowBandwidth) {
                auto count = lowBandwidthClientCount_.fetch_add(
                                 1, std::memory_order_relaxed) +
                             1;
                lowBandwidthModeActive_.store(true, std::memory_order_release);
                LOG_INFO("[ServerNetworkSystem] Low bandwidth mode ENABLED "
                         << "(client " << userId << ", " << count
                         << " clients requesting)");
            } else {
                std::uint32_t old =
                    lowBandwidthClientCount_.load(std::memory_order_acquire);
                std::uint32_t count = 0;
                for (;;) {
                    if (old == 0) {
                        count = 0;
                        break;
                    }
                    std::uint32_t desired = old - 1;
                    if (lowBandwidthClientCount_.compare_exchange_weak(
                            old, desired, std::memory_order_acq_rel,
                            std::memory_order_acquire)) {
                        count = desired;
                        break;
                    }
                }

                if (count == 0) {
                    lowBandwidthModeActive_.store(false,
                                                  std::memory_order_release);
                    LOG_INFO(
                        "[ServerNetworkSystem] Low bandwidth mode DISABLED "
                        << "(no clients requesting)");
                } else {
                    LOG_INFO("[ServerNetworkSystem] Client "
                             << userId << " disabled low bandwidth, but "
                             << count << " clients still requesting");
                }
            }
        });

        server_->onClientChat(
            [this](std::uint32_t userId, const std::string& msg) {
                handleClientChat(userId, msg);
            });
    }
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

    std::uint8_t subType = 0;
    if (registry_
            ->hasComponent<rtype::games::rtype::shared::EnemyTypeComponent>(
                entity)) {
        const auto& enemyType =
            registry_
                ->getComponent<rtype::games::rtype::shared::EnemyTypeComponent>(
                    entity);
        subType = static_cast<std::uint8_t>(enemyType.variant);
    } else if (registry_->hasComponent<
                   rtype::games::rtype::shared::PowerUpTypeComponent>(entity)) {
        const auto& powerUpType = registry_->getComponent<
            rtype::games::rtype::shared::PowerUpTypeComponent>(entity);
        subType = static_cast<std::uint8_t>(powerUpType.variant);
    }

    if (server_) {
        server_->spawnEntity(networkId, type, subType, x, y);
    }
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

    ECS::Entity entity = it->second.entity;

    if (!entity.isNull()) {
        entityToNetworkId_.erase(entity.id);
    }

    networkedEntities_.erase(it);

    if (server_) {
        server_->destroyEntity(networkId);
    }

    if (registry_ && !entity.isNull() && registry_->isAlive(entity)) {
        registry_->killEntity(entity);
    }
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
    if (server_) {
        server_->correctPosition(userId, x, y);
    }
}

void ServerNetworkSystem::updateEntityHealth(std::uint32_t networkId,
                                             std::int32_t current,
                                             std::int32_t max) {
    if (server_) {
        server_->updateEntityHealth(networkId, current, max);
    }
}

void ServerNetworkSystem::broadcastPowerUp(std::uint32_t playerNetworkId,
                                           std::uint8_t powerUpType,
                                           float duration) {
    if (server_) {
        server_->broadcastPowerUp(playerNetworkId, powerUpType, duration);
    }
}

void ServerNetworkSystem::broadcastEntityUpdates() {
    std::vector<std::tuple<std::uint32_t, float, float, float, float>>
        dirtyEntities;

    bool lowBandwidth = lowBandwidthModeActive_.load(std::memory_order_acquire);

    std::uint32_t playerInterval, enemyInterval, projectileInterval;
    float playerPosDelta, playerVelDelta;
    float enemyPosDelta, enemyVelDelta;
    float projectilePosDelta, projectileVelDelta;

    if (lowBandwidth) {
        playerInterval = LowBandwidthMode::PLAYER_UPDATE_INTERVAL;
        enemyInterval = LowBandwidthMode::ENEMY_UPDATE_INTERVAL;
        projectileInterval = LowBandwidthMode::PROJECTILE_UPDATE_INTERVAL;
        playerPosDelta = LowBandwidthMode::PLAYER_POSITION_DELTA;
        playerVelDelta = LowBandwidthMode::PLAYER_VELOCITY_DELTA;
        enemyPosDelta = LowBandwidthMode::ENEMY_POSITION_DELTA;
        enemyVelDelta = LowBandwidthMode::ENEMY_VELOCITY_DELTA;
        projectilePosDelta = LowBandwidthMode::PROJECTILE_POSITION_DELTA;
        projectileVelDelta = LowBandwidthMode::PROJECTILE_VELOCITY_DELTA;
    } else {
        playerInterval = NormalMode::PLAYER_UPDATE_INTERVAL;
        enemyInterval = NormalMode::ENEMY_UPDATE_INTERVAL;
        projectileInterval = NormalMode::PROJECTILE_UPDATE_INTERVAL;
        playerPosDelta = NormalMode::PLAYER_POSITION_DELTA;
        playerVelDelta = NormalMode::PLAYER_VELOCITY_DELTA;
        enemyPosDelta = NormalMode::ENEMY_POSITION_DELTA;
        enemyVelDelta = NormalMode::ENEMY_VELOCITY_DELTA;
        projectilePosDelta = NormalMode::PROJECTILE_POSITION_DELTA;
        projectileVelDelta = NormalMode::PROJECTILE_VELOCITY_DELTA;
    }

    for (auto& [networkId, info] : networkedEntities_) {
        info.ticksSinceLastSend++;

        if (!info.dirty) {
            continue;
        }

        if (!isEntityVisible(info.lastX, info.lastY)) {
            info.dirty = false;
            continue;
        }

        float posDelta = std::abs(info.lastX - info.lastSentX) +
                         std::abs(info.lastY - info.lastSentY);
        float velDelta = std::abs(info.lastVx - info.lastSentVx) +
                         std::abs(info.lastVy - info.lastSentVy);

        std::uint32_t updateInterval = playerInterval;
        float posThreshold = playerPosDelta;
        float velThreshold = playerVelDelta;
        if (info.type == EntityType::Bydos ||
            info.type == EntityType::Obstacle) {
            updateInterval = enemyInterval;
            posThreshold = enemyPosDelta;
            velThreshold = enemyVelDelta;
        } else if (info.type == EntityType::Missile) {
            updateInterval = projectileInterval;
            posThreshold = projectilePosDelta;
            velThreshold = projectileVelDelta;
        }

        bool shouldSend = (info.ticksSinceLastSend >= updateInterval) ||
                          (posDelta > posThreshold) ||
                          (velDelta > velThreshold);

        if (shouldSend) {
            dirtyEntities.emplace_back(networkId, info.lastX, info.lastY,
                                       info.lastVx, info.lastVy);
            info.lastSentX = info.lastX;
            info.lastSentY = info.lastY;
            info.lastSentVx = info.lastVx;
            info.lastSentVy = info.lastVy;
            info.ticksSinceLastSend = 0;
        }

        info.dirty = false;
    }

    if (dirtyEntities.empty()) {
        return;
    }

    constexpr std::size_t maxPerBatch = network::kMaxEntitiesPerBatch;

    for (std::size_t offset = 0; offset < dirtyEntities.size();
         offset += maxPerBatch) {
        auto end = std::min(offset + maxPerBatch, dirtyEntities.size());
        std::vector<std::tuple<std::uint32_t, float, float, float, float>>
            batch(dirtyEntities.begin() + static_cast<std::ptrdiff_t>(offset),
                  dirtyEntities.begin() + static_cast<std::ptrdiff_t>(end));
        server_->moveEntitiesBatch(batch);
    }
}

void ServerNetworkSystem::broadcastEntitySpawn(std::uint32_t networkId,
                                               EntityType type,
                                               std::uint8_t subType, float x,
                                               float y) {
    NetworkedEntity info{};

    auto existing = networkedEntities_.find(networkId);
    if (existing != networkedEntities_.end()) {
        info = existing->second;
    }

    if (info.entity.isNull()) {
        auto view =
            registry_->view<rtype::games::rtype::shared::NetworkIdComponent>();
        view.each(
            [&](ECS::Entity ent,
                const rtype::games::rtype::shared::NetworkIdComponent& net) {
                if (net.networkId == networkId) {
                    info.entity = ent;
                }
            });
    }

    info.networkId = networkId;
    info.type = type;
    info.lastX = x;
    info.lastY = y;
    info.lastVx = 0;
    info.lastVy = 0;
    info.dirty = false;

    networkedEntities_[networkId] = info;
    if (server_) {
        server_->spawnEntity(networkId, type, subType, x, y);

        if (registry_ && !info.entity.isNull() &&
            registry_->isAlive(info.entity) &&
            registry_
                ->hasComponent<rtype::games::rtype::shared::HealthComponent>(
                    info.entity)) {
            const auto& health = registry_->getComponent<
                rtype::games::rtype::shared::HealthComponent>(info.entity);
            LOG_DEBUG_CAT(::rtype::LogCategory::Network,
                          "[NetworkServer] Sending initial health for entity " +
                              std::to_string(networkId) + ": " +
                              std::to_string(health.current) + "/" +
                              std::to_string(health.max));
            server_->updateEntityHealth(networkId, health.current, health.max);
        } else {
            LOG_DEBUG_CAT(::rtype::LogCategory::Network,
                          "[NetworkServer] No health component for entity " +
                              std::to_string(networkId));
        }
    } else {
        LOG_DEBUG_CAT(::rtype::LogCategory::Network,
                      "[NetworkServer] No health component for entity " +
                          std::to_string(networkId));
    }
}

void ServerNetworkSystem::broadcastGameStart() {
    if (server_) {
        server_->updateGameState(NetworkServer::GameState::Running);
    }
}

void ServerNetworkSystem::broadcastGameState(NetworkServer::GameState state) {
    if (server_) {
        server_->updateGameState(state);
    }
}

void ServerNetworkSystem::broadcastGameOver(std::uint32_t finalScore,
                                            bool isVictory) {
    if (server_) {
        server_->sendGameOver(finalScore, isVictory);
    }
}

void ServerNetworkSystem::resetState() {
    for (const auto& [networkId, _] : networkedEntities_) {
        if (server_) {
            server_->destroyEntity(networkId);
        }
    }

    networkedEntities_.clear();
    entityToNetworkId_.clear();
    userIdToEntity_.clear();
    pendingDisconnections_.clear();
    nextNetworkIdCounter_ = 1;

    lowBandwidthClientCount_.store(0, std::memory_order_release);
    lowBandwidthModeActive_.store(false, std::memory_order_release);
}

void ServerNetworkSystem::update() {
    if (server_) {
        server_->poll();
    }

    processExpiredGracePeriods();

    std::vector<std::uint32_t> toRemove;
    for (auto& [networkId, info] : networkedEntities_) {
        if (!info.entity.isNull() && !registry_->isAlive(info.entity)) {
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
        const auto& pending = pendingIt->second;
        if (pending.networkId != 0) {
            unregisterNetworkedEntityById(pending.networkId);
        }
        pendingDisconnections_.erase(pendingIt);
    }

    for (const auto& [networkId, info] : networkedEntities_) {
        std::uint8_t subType = 0;
        if (registry_->isAlive(info.entity) &&
            registry_
                ->hasComponent<rtype::games::rtype::shared::EnemyTypeComponent>(
                    info.entity)) {
            const auto& enemyType = registry_->getComponent<
                rtype::games::rtype::shared::EnemyTypeComponent>(info.entity);
            subType = static_cast<std::uint8_t>(enemyType.variant);
        } else if (registry_->isAlive(info.entity) &&
                   registry_->hasComponent<
                       rtype::games::rtype::shared::PowerUpTypeComponent>(
                       info.entity)) {
            const auto& powerUpType = registry_->getComponent<
                rtype::games::rtype::shared::PowerUpTypeComponent>(info.entity);
            subType = static_cast<std::uint8_t>(powerUpType.variant);
        }

        if (server_) {
            server_->spawnEntityToClient(userId, networkId, info.type, subType,
                                         info.lastX, info.lastY);

            if (registry_->isAlive(info.entity) &&
                registry_->hasComponent<
                    rtype::games::rtype::shared::HealthComponent>(
                    info.entity)) {
                const auto& health = registry_->getComponent<
                    rtype::games::rtype::shared::HealthComponent>(info.entity);
                server_->updateEntityHealthToClient(userId, networkId,
                                                    health.current, health.max);
            }
        }
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

    LOG_INFO_CAT(::rtype::LogCategory::Network,
                 "[NetworkServer] Client disconnected userId="
                     << userId << " reason=" << static_cast<int>(reason)
                     << (useGracePeriod ? " (grace)" : ""));

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

    LOG_INFO_CAT(
        ::rtype::LogCategory::Network,
        "[ServerNetworkSystem] Finalized disconnection for userId=" << userId);

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
    if (server_) {
        auto connectedClients = server_->getConnectedClients();
        server_->sendUserList(userId, connectedClients);
    } else {
        LOG_DEBUG(
            "[ServerNetworkSystem] No server available to send user list");
    }
}

void ServerNetworkSystem::handleClientChat(std::uint32_t userId,
                                           const std::string& message) {
    if (server_) {
        LOG_INFO("[ServerNetworkSystem] Chat from " + std::to_string(userId) +
                 ": " + message);
        server_->broadcastChat(userId, message);
    }
}

}  // namespace rtype::server
