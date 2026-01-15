/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientNetworkSystem - Implementation
*/

#include "ClientNetworkSystem.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <utility>

#include "Components/HealthComponent.hpp"
#include "Components/SoundComponent.hpp"
#include "Components/VelocityComponent.hpp"
#include "Logger/Macros.hpp"
#include "client/Graphic/AudioLib/AudioLib.hpp"
#include "games/rtype/client/Components/BoxingComponent.hpp"
#include "games/rtype/client/Components/HiddenComponent.hpp"
#include "games/rtype/client/Components/ImageComponent.hpp"
#include "games/rtype/client/Components/LaserBeamAnimationComponent.hpp"
#include "games/rtype/client/Components/RectangleComponent.hpp"
#include "games/rtype/client/Components/TagComponent.hpp"
#include "games/rtype/client/Components/ZIndexComponent.hpp"
#include "games/rtype/client/GameScene/VisualCueFactory.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/PlayerIdComponent.hpp"
#include "games/rtype/shared/Components/PowerUpComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/WeakPointComponent.hpp"

namespace rtype::client {

using Transform = rtype::games::rtype::shared::TransformComponent;
using Velocity = rtype::games::rtype::shared::VelocityComponent;
using Clock = std::chrono::steady_clock;

namespace {

double nowSeconds() {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
               Clock::now().time_since_epoch())
        .count();
}

void updateProjectileVisuals(ECS::Registry& registry, ECS::Entity entity) {
    if (!registry.hasComponent<Velocity>(entity) ||
        !registry.hasComponent<games::rtype::shared::ProjectileTag>(entity) ||
        !registry.hasComponent<games::rtype::client::Image>(entity)) {
        return;
    }

    const auto& vel = registry.getComponent<Velocity>(entity);
    auto& img = registry.getComponent<games::rtype::client::Image>(entity);
    const bool isEnemyShot = vel.vx < 0.0F;
    img.color = (isEnemyShot ? rtype::display::Color(255, 80, 80, 255)
                             : rtype::display::Color(80, 255, 240, 255));

    if (registry.hasComponent<games::rtype::client::BoxingComponent>(entity)) {
        auto& box =
            registry.getComponent<games::rtype::client::BoxingComponent>(
                entity);
        box.outlineColor = isEnemyShot
                               ? rtype::display::Color(255, 80, 80, 255)
                               : rtype::display::Color(0, 220, 180, 255);
        box.fillColor = isEnemyShot ? rtype::display::Color(255, 80, 80, 40)
                                    : rtype::display::Color(0, 220, 180, 35);
    }
}

}  // namespace

ClientNetworkSystem::ClientNetworkSystem(
    std::shared_ptr<ECS::Registry> registry,
    std::shared_ptr<NetworkClient> client)
    : registry_(std::move(registry)), client_(std::move(client)) {
    registerCallbacks();
}

ClientNetworkSystem::~ClientNetworkSystem() {
    if (client_) {
        client_->clearPendingCallbacks();
        client_->clearConnectedCallbacks();
        client_->clearDisconnectedCallbacks();
        client_->onEntitySpawn(nullptr);
        client_->onEntityMove(nullptr);
        client_->onEntityMoveBatch(nullptr);
        client_->clearEntityDestroyCallbacks();
        client_->onEntityHealth(nullptr);
        client_->onPowerUpEvent(nullptr);
        client_->onPositionCorrection(nullptr);
    }
}

void ClientNetworkSystem::registerCallbacks() {
    client_->onConnected(
        [this](std::uint32_t userId) { handleConnected(userId); });

    client_->onDisconnected([this](network::DisconnectReason reason) {
        handleDisconnected(reason);
    });

    client_->onEntitySpawn(
        [this](EntitySpawnEvent event) { handleEntitySpawn(event); });

    client_->onEntityMove(
        [this](EntityMoveEvent event) { handleEntityMove(event); });

    client_->onEntityMoveBatch([this](EntityMoveBatchEvent event) {
        for (const auto& moveEvent : event.entities) {
            handleEntityMove(moveEvent);
        }
    });

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
        LOG_DEBUG_CAT(
            rtype::LogCategory::Network,
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
            LOG_DEBUG_CAT(
                rtype::LogCategory::Network,
                "[ClientNetworkSystem] Replaying cached health event for local "
                "user "
                    << *localUserId_ << ": " << event.current << "/"
                    << event.max);
            onHealthUpdateCallback_(event);
        }
    }
}

void ClientNetworkSystem::update() { client_->poll(); }

void ClientNetworkSystem::sendInput(std::uint16_t inputMask) {
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

void ClientNetworkSystem::reset() {
    LOG_DEBUG_CAT(rtype::LogCategory::Network,
                  "[ClientNetworkSystem] Resetting network system state");
    for (const auto& [networkId, entity] : networkIdToEntity_) {
        if (registry_->isAlive(entity)) {
            LOG_DEBUG_CAT(
                rtype::LogCategory::Network,
                "[ClientNetworkSystem] Destroying network entity: networkId="
                    << networkId);
            registry_->killEntity(entity);
        }
    }
    networkIdToEntity_.clear();
    localUserId_.reset();
    localPlayerEntity_.reset();
    pendingPlayerSpawns_.clear();
    lastKnownHealth_.clear();
    disconnectedHandled_ = false;
    debugNotFoundLogCount_ = 0;
    debugBossPartLogCount_ = 0;

    LOG_DEBUG_CAT(rtype::LogCategory::Network,
                  "[ClientNetworkSystem] Network system state reset complete");
}

void ClientNetworkSystem::handleEntitySpawn(const EntitySpawnEvent& event) {
    LOG_DEBUG_CAT(rtype::LogCategory::Network,
                  "[ClientNetworkSystem] Entity spawn received: entityId=" +
                      std::to_string(event.entityId) +
                      " type=" + std::to_string(static_cast<int>(event.type)) +
                      " pos=(" + std::to_string(event.x) + ", " +
                      std::to_string(event.y) + ") localUserId=" +
                      (localUserId_.has_value() ? std::to_string(*localUserId_)
                                                : "none"));

    if (event.type == network::EntityType::LaserBeam) {
        using LaserAnim = games::rtype::client::LaserBeamAnimationComponent;
        std::vector<ECS::Entity> toDestroy;
        registry_->view<LaserAnim>().each(
            [&toDestroy](ECS::Entity entity, const LaserAnim& anim) {
                if (anim.pendingDestroy) {
                    toDestroy.push_back(entity);
                }
            });
        for (auto entity : toDestroy) {
            registry_->killEntity(entity);
            LOG_DEBUG_CAT(
                rtype::LogCategory::Network,
                "[ClientNetworkSystem] Cleaned up old laser beam entity");
        }
    }

    auto existingIt = networkIdToEntity_.find(event.entityId);
    if (existingIt != networkIdToEntity_.end()) {
        if (registry_->isAlive(existingIt->second)) {
            LOG_INFO("[ClientNetworkSystem] Entity already exists (id="
                     << event.entityId
                     << "), updating position and ensuring visible");

            ECS::Entity existingEntity = existingIt->second;

            if (registry_->hasComponent<Transform>(existingEntity)) {
                auto& pos = registry_->getComponent<Transform>(existingEntity);
                LOG_INFO("[ClientNetworkSystem] Updating position from ("
                         << pos.x << "," << pos.y << ") to (" << event.x << ","
                         << event.y << ")");
                pos.x = event.x;
                pos.y = event.y;
            }

            if (registry_->hasComponent<games::rtype::client::HiddenComponent>(
                    existingEntity)) {
                auto& hidden =
                    registry_
                        ->getComponent<games::rtype::client::HiddenComponent>(
                            existingEntity);
                if (hidden.isHidden) {
                    hidden.isHidden = false;
                    LOG_INFO(
                        "[ClientNetworkSystem] Unhiding existing entity on "
                        "spawn");
                }
            } else {
                LOG_INFO("[ClientNetworkSystem] Entity has no HiddenComponent");
            }

            if (event.type == network::EntityType::Player) {
                if (localUserId_.has_value() &&
                    event.entityId == *localUserId_) {
                    localPlayerEntity_ = existingEntity;
                    LOG_INFO(
                        "[ClientNetworkSystem] Existing entity is our local "
                        "player!");
                    if (onLocalPlayerAssignedCallback_) {
                        onLocalPlayerAssignedCallback_(*localUserId_,
                                                       existingEntity);
                    }
                }
            }
            return;
        } else {
            LOG_DEBUG_CAT(
                rtype::LogCategory::Network,
                "[ClientNetworkSystem] Entity exists but is dead, removing and "
                "recreating");
            networkIdToEntity_.erase(existingIt);
        }
    }

    ECS::Entity entity;

    if (entityFactory_) {
        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[ClientNetworkSystem] Using custom entityFactory");
        entity = entityFactory_(*registry_, event);
    } else {
        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[ClientNetworkSystem] Using default entityFactory");
        entity = defaultEntityFactory(*registry_, event);
    }

    networkIdToEntity_[event.entityId] = entity;
    LOG_DEBUG_CAT(
        rtype::LogCategory::Network,
        "[ClientNetworkSystem] Created entity id=" + std::to_string(entity.id));

    if (event.type == network::EntityType::Player) {
        if (localUserId_.has_value() && event.entityId == *localUserId_) {
            localPlayerEntity_ = entity;
            LOG_DEBUG_CAT(rtype::LogCategory::Network,
                          "[ClientNetworkSystem] This is our local player!");

            if (onLocalPlayerAssignedCallback_) {
                onLocalPlayerAssignedCallback_(*localUserId_, entity);
            }
        } else {
            pendingPlayerSpawns_[event.entityId] = entity;
            LOG_DEBUG_CAT(
                rtype::LogCategory::Network,
                "[ClientNetworkSystem] Stored pending player spawn: entityId=" +
                    std::to_string(event.entityId));
        }
    }
}

void ClientNetworkSystem::handleEntityMove(const EntityMoveEvent& event) {
    auto it = networkIdToEntity_.find(event.entityId);
    if (it == networkIdToEntity_.end()) {
        if (debugNotFoundLogCount_ < 100) {
            LOG_DEBUG_CAT(rtype::LogCategory::Network,
                          "[ClientNetworkSystem] handleEntityMove: networkId="
                              << event.entityId << " NOT FOUND in map (size="
                              << networkIdToEntity_.size() << ")");
            debugNotFoundLogCount_++;
        }
        return;
    }

    ECS::Entity entity = it->second;

    if (!registry_->isAlive(entity)) {
        networkIdToEntity_.erase(it);
        return;
    }

    if (registry_->hasComponent<rtype::games::rtype::client::LobbyTag>(
            entity)) {
        return;
    }

    const bool isLocalPlayer =
        localPlayerEntity_.has_value() && *localPlayerEntity_ == entity;

    if (isLocalPlayer) {
        if (registry_->hasComponent<Transform>(entity)) {
            auto& pos = registry_->getComponent<Transform>(entity);
            pos.x = event.x;
            pos.y = event.y;
        }
        if (registry_->hasComponent<Velocity>(entity)) {
            auto& vel = registry_->getComponent<Velocity>(entity);
            vel.vx = event.vx;
            vel.vy = event.vy;
        }
        if (registry_->hasComponent<games::rtype::client::HiddenComponent>(
                entity)) {
            auto& hidden =
                registry_->getComponent<games::rtype::client::HiddenComponent>(
                    entity);
            if (hidden.isHidden) {
                hidden.isHidden = false;
                LOG_DEBUG_CAT(rtype::LogCategory::Network,
                              "[ClientNetworkSystem] Unhiding local player "
                              "entity after receiving position");
            }
        }
        return;
    }

    if (registry_->hasComponent<Transform>(entity)) {
        auto& pos = registry_->getComponent<Transform>(entity);

        if (debugBossPartLogCount_ < 60 &&
            registry_->hasComponent<rtype::games::rtype::shared::WeakPointTag>(
                entity)) {
            LOG_DEBUG_CAT(rtype::LogCategory::Network,
                          "[ClientNetworkSystem] BossPart move: netId="
                              << event.entityId << " entity=" << entity.id
                              << " pos (" << pos.x << "," << pos.y << ") -> ("
                              << event.x << "," << event.y << ")");
            debugBossPartLogCount_++;
        }

        pos.x = event.x;
        pos.y = event.y;

        if (registry_->hasComponent<games::rtype::shared::PlayerIdComponent>(
                entity) &&
            registry_->hasComponent<games::rtype::client::HiddenComponent>(
                entity)) {
            auto& hidden =
                registry_->getComponent<games::rtype::client::HiddenComponent>(
                    entity);
            if (hidden.isHidden) {
                hidden.isHidden = false;
                LOG_DEBUG_CAT(rtype::LogCategory::Network,
                              "[ClientNetworkSystem] Unhiding player entity "
                              "after receiving position from server");
            }
        }
    }

    if (registry_->hasComponent<Velocity>(entity)) {
        auto& vel = registry_->getComponent<Velocity>(entity);
        vel.vx = event.vx;
        vel.vy = event.vy;
    }

    updateProjectileVisuals(*registry_, entity);
}

void ClientNetworkSystem::_playDeathSound(ECS::Entity entity) {
    if (registry_->hasComponent<games::rtype::client::EnemySoundComponent>(
            entity)) {
        auto& soundComp =
            registry_->getComponent<games::rtype::client::EnemySoundComponent>(
                entity);
        auto audioLib = registry_->getSingleton<std::shared_ptr<AudioLib>>();
        audioLib->playSFX(soundComp.deathSFX);
    }
    if (registry_->hasComponent<games::rtype::shared::TransformComponent>(
            entity)) {
        const auto& pos =
            registry_->getComponent<games::rtype::shared::TransformComponent>(
                entity);
        games::rtype::client::VisualCueFactory::createFlash(
            *registry_, {pos.x, pos.y}, rtype::display::Color(255, 80, 0, 255),
            90.f, 0.45f, 20);
    }
    if (registry_->hasComponent<games::rtype::client::PlayerSoundComponent>(
            entity)) {
        auto& soundComp =
            registry_->getComponent<games::rtype::client::PlayerSoundComponent>(
                entity);
        auto audioLib = registry_->getSingleton<std::shared_ptr<AudioLib>>();
        audioLib->playSFX(soundComp.deathSFX);
    }
}

void ClientNetworkSystem::handleEntityDestroy(std::uint32_t entityId) {
    LOG_DEBUG_CAT(rtype::LogCategory::Network,
                  "[ClientNetworkSystem] Entity destroy received: entityId=" +
                      std::to_string(entityId));

    auto it = networkIdToEntity_.find(entityId);
    if (it == networkIdToEntity_.end()) {
        LOG_DEBUG_CAT(
            rtype::LogCategory::Network,
            "[ClientNetworkSystem] Entity not found in map, skipping");
        return;
    }

    ECS::Entity entity = it->second;

    if (registry_->isAlive(entity)) {
        using LaserAnim = games::rtype::client::LaserBeamAnimationComponent;
        if (registry_->hasComponent<LaserAnim>(entity)) {
            auto& anim = registry_->getComponent<LaserAnim>(entity);
            if (!anim.pendingDestroy) {
                anim.pendingDestroy = true;
                LOG_DEBUG_CAT(
                    rtype::LogCategory::Network,
                    "[ClientNetworkSystem] Laser beam end animation triggered");
            }
            this->networkIdToEntity_.erase(it);
            lastKnownHealth_.erase(entityId);
            return;
        }

        _playDeathSound(entity);
        registry_->killEntity(entity);
        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[ClientNetworkSystem] Entity killed");
    }

    this->networkIdToEntity_.erase(it);
    lastKnownHealth_.erase(entityId);

    if (localPlayerEntity_.has_value() && *localPlayerEntity_ == entity) {
        localPlayerEntity_.reset();
        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[ClientNetworkSystem] Local player entity reset!");
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

    if (registry_->hasComponent<Transform>(entity)) {
        auto& pos = registry_->getComponent<Transform>(entity);
        pos.x = x;
        pos.y = y;
    }
}

void ClientNetworkSystem::handleEntityHealth(const EntityHealthEvent& event) {
    LOG_DEBUG_CAT(rtype::LogCategory::Network,
                  "[ClientNetworkSystem] handleEntityHealth: entityId="
                      << event.entityId << " current=" << event.current
                      << " max=" << event.max);

    const auto prevIt = lastKnownHealth_.find(event.entityId);
    std::optional<int32_t> previousHealth = std::nullopt;
    if (prevIt != lastKnownHealth_.end()) {
        previousHealth = prevIt->second.current;
    }

    auto it = networkIdToEntity_.find(event.entityId);
    if (it == networkIdToEntity_.end()) {
        LOG_DEBUG_CAT(
            rtype::LogCategory::Network,
            "[ClientNetworkSystem] Entity "
                << event.entityId
                << " not found in networkIdToEntity_ map, ignoring health");
        lastKnownHealth_.erase(event.entityId);
        return;
    }

    ECS::Entity entity = it->second;

    if (!registry_->isAlive(entity)) {
        LOG_DEBUG_CAT(rtype::LogCategory::Network,
                      "[ClientNetworkSystem] Entity "
                          << event.entityId << " not alive, ignoring health");
        networkIdToEntity_.erase(it);
        lastKnownHealth_.erase(event.entityId);
        return;
    }

    if (registry_->hasComponent<rtype::games::rtype::shared::HealthComponent>(
            entity)) {
        auto& health =
            registry_
                ->getComponent<rtype::games::rtype::shared::HealthComponent>(
                    entity);
        previousHealth = health.current;
        health.current = event.current;
        health.max = event.max;
    } else {
        registry_
            ->emplaceComponent<rtype::games::rtype::shared::HealthComponent>(
                entity, event.current, event.max);
    }

    if (previousHealth.has_value() && previousHealth.value() > event.current &&
        registry_
            ->hasComponent<rtype::games::rtype::shared::TransformComponent>(
                entity)) {
        const auto& pos =
            registry_
                ->getComponent<rtype::games::rtype::shared::TransformComponent>(
                    entity);
        games::rtype::client::VisualCueFactory::createFlash(
            *registry_, {pos.x, pos.y}, rtype::display::Color(255, 80, 80, 255),
            70.f, 0.25f, 12);
        bool isLocalPlayer =
            localUserId_.has_value() && event.entityId == *localUserId_;
        bool hasEnemyTag =
            registry_->hasComponent<rtype::games::rtype::shared::EnemyTag>(
                entity);

        LOG_INFO("[ClientNetworkSystem] Health change: entityId="
                 << event.entityId << " isLocalPlayer=" << isLocalPlayer
                 << " hasEnemyTag=" << hasEnemyTag
                 << " previousHealth=" << previousHealth.value()
                 << " currentHealth=" << event.current);

        if (!isLocalPlayer && hasEnemyTag) {
            int32_t damageAmount = previousHealth.value() - event.current;
            LOG_INFO("[ClientNetworkSystem] Creating damage popup for enemy "
                     << event.entityId << " damage=" << damageAmount
                     << " at position (" << pos.x << ", " << pos.y << ")");
            games::rtype::client::VisualCueFactory::createDamagePopup(
                *registry_, {pos.x, pos.y}, damageAmount, "title_font",
                rtype::display::Color(255, 200, 0, 255));
        }
    }
    lastKnownHealth_[event.entityId] = {event.current, event.max};

    if (onHealthUpdateCallback_ && localUserId_.has_value() &&
        event.entityId == *localUserId_) {
        LOG_DEBUG_CAT(
            rtype::LogCategory::Network,
            "[ClientNetworkSystem] Calling onHealthUpdateCallback_ for "
            "local player");
        onHealthUpdateCallback_(event);
    }
}

void ClientNetworkSystem::handlePowerUpEvent(const PowerUpEvent& event) {
    LOG_INFO("[ClientNetworkSystem] *** RECEIVED POWERUP EVENT ***: playerId="
             << event.playerId
             << " type=" << static_cast<int>(event.powerUpType)
             << " duration=" << event.duration);

    auto it = networkIdToEntity_.find(event.playerId);
    if (it == networkIdToEntity_.end()) {
        LOG_WARNING("[ClientNetworkSystem] PowerUp event for unknown player: "
                    << event.playerId);
        return;
    }

    ECS::Entity entity = it->second;
    if (!registry_->isAlive(entity)) {
        LOG_WARNING("[ClientNetworkSystem] PowerUp event for dead entity");
        return;
    }

    const auto powerUpType =
        static_cast<rtype::games::rtype::shared::PowerUpType>(
            event.powerUpType);

    LOG_INFO("[ClientNetworkSystem] Applying powerup to entity " << entity.id);

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

    LOG_INFO("[ClientNetworkSystem] ActivePowerUpComponent set: type="
             << static_cast<int>(active.type)
             << " remainingTime=" << active.remainingTime);

    if (registry_
            ->hasComponent<rtype::games::rtype::shared::TransformComponent>(
                entity)) {
        const auto& pos =
            registry_
                ->getComponent<rtype::games::rtype::shared::TransformComponent>(
                    entity);
        rtype::display::Color cueColor =
            rtype::display::Color(180, 240, 255, 255);
        switch (powerUpType) {
            case rtype::games::rtype::shared::PowerUpType::Shield:
                cueColor = rtype::display::Color(255, 215, 0, 255);
                break;
            case rtype::games::rtype::shared::PowerUpType::SpeedBoost:
                cueColor = rtype::display::Color(120, 255, 200, 255);
                break;
            case rtype::games::rtype::shared::PowerUpType::RapidFire:
                cueColor = rtype::display::Color(120, 200, 255, 255);
                break;
            case rtype::games::rtype::shared::PowerUpType::DoubleDamage:
                cueColor = rtype::display::Color(255, 150, 150, 255);
                break;
            case rtype::games::rtype::shared::PowerUpType::HealthBoost:
                cueColor = rtype::display::Color(220, 180, 255, 255);
                break;
            default:
                break;
        }

        games::rtype::client::VisualCueFactory::createFlash(
            *registry_, {pos.x, pos.y}, cueColor, 80.f, 0.35f, 14);
    }
}

void ClientNetworkSystem::handleConnected(std::uint32_t userId) {
    LOG_INFO_CAT(rtype::LogCategory::Network,
                 "[ClientNetworkSystem] Connected with userId=" +
                     std::to_string(userId));
    localUserId_ = userId;
    disconnectedHandled_ = false;

    auto pendingIt = pendingPlayerSpawns_.find(userId);
    if (pendingIt != pendingPlayerSpawns_.end()) {
        localPlayerEntity_ = pendingIt->second;
        LOG_INFO_CAT(
            rtype::LogCategory::Network,
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
            LOG_INFO_CAT(
                rtype::LogCategory::Network,
                "[ClientNetworkSystem] Replaying cached health for newly "
                "assigned player: "
                    << event.current << "/" << event.max);
            onHealthUpdateCallback_(event);
        }
    }

    pendingPlayerSpawns_.clear();
}

void ClientNetworkSystem::handleDisconnected(network::DisconnectReason reason) {
    LOG_DEBUG("[ClientNetworkSystem] handleDisconnected called, reason="
              << static_cast<int>(reason));
    if (disconnectedHandled_) {
        LOG_DEBUG("[ClientNetworkSystem] Disconnect already handled, skipping");
        return;
    }
    disconnectedHandled_ = true;

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

    if (onDisconnectCallback_) {
        LOG_DEBUG("[ClientNetworkSystem] Calling onDisconnect callback");
        onDisconnectCallback_(reason);
    } else {
        LOG_WARNING(
            "[ClientNetworkSystem] No onDisconnect callback registered!");
    }
}

void ClientNetworkSystem::onDisconnect(
    std::function<void(network::DisconnectReason)> callback) {
    onDisconnectCallback_ = std::move(callback);
}

ECS::Entity ClientNetworkSystem::defaultEntityFactory(
    ECS::Registry& registry, const EntitySpawnEvent& event) {
    auto entity = registry.spawnEntity();

    registry.emplaceComponent<Transform>(entity, event.x, event.y);
    registry.emplaceComponent<Velocity>(entity, 0.f, 0.f);
    registry.emplaceComponent<rtype::games::rtype::shared::NetworkIdComponent>(
        entity, event.entityId);

    switch (event.type) {
        case network::EntityType::Pickup: {
            const rtype::display::Color color(140, 220, 255);
            registry.emplaceComponent<games::rtype::client::Rectangle>(
                entity, std::pair<float, float>{22.f, 22.f}, color, color);
            registry.emplaceComponent<games::rtype::client::BoxingComponent>(
                entity, rtype::display::Vector2f{0, 0},
                rtype::display::Vector2f{22.f, 22.f});
            registry.emplaceComponent<games::rtype::client::ZIndex>(entity, 0);
            break;
        }
        case network::EntityType::Obstacle: {
            const rtype::display::Color color(160, 160, 170);
            registry.emplaceComponent<games::rtype::client::Rectangle>(
                entity, std::pair<float, float>{48.f, 48.f}, color, color);
            registry.emplaceComponent<games::rtype::client::BoxingComponent>(
                entity, rtype::display::Vector2f{0, 0},
                rtype::display::Vector2f{48.f, 48.f});
            registry.emplaceComponent<games::rtype::client::ZIndex>(entity, 0);
            break;
        }
        default:
            break;
    }

    return entity;
}

}  // namespace rtype::client
