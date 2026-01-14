/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LaserBeamSystem - Implementation
*/

#include "LaserBeamSystem.hpp"

#include "../../../shared/Components/EntityType.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

using shared::BoundingBoxComponent;
using shared::DamageOnContactComponent;
using shared::EntityType;
using shared::LaserBeamComponent;
using shared::LaserBeamState;
using shared::LaserBeamTag;
using shared::NetworkIdComponent;
using shared::PlayerTag;
using shared::TransformComponent;

LaserBeamSystem::LaserBeamSystem(EventEmitter emitter,
                                 const game::config::LaserConfig& config)
    : ASystem("LaserBeamSystem"),
      _emitEvent(std::move(emitter)),
      _config(config) {}

void LaserBeamSystem::update(ECS::Registry& registry, float deltaTime) {
    rebuildPlayerCache(registry);
    updateActiveBeams(registry, deltaTime);
    updateBeamPositions(registry);
}

void LaserBeamSystem::handleLaserInput(ECS::Registry& registry,
                                       ECS::Entity playerEntity,
                                       uint32_t playerNetworkId,
                                       bool isFiring) {
    if (isFiring) {
        if (!hasActiveLaser(registry, playerNetworkId)) {
            startLaser(registry, playerEntity, playerNetworkId);
        }
    } else {
        stopLaser(registry, playerNetworkId);
    }
}

bool LaserBeamSystem::hasActiveLaser(ECS::Registry& registry,
                                     uint32_t playerNetworkId) const {
    auto view = registry.view<LaserBeamTag, LaserBeamComponent>();
    bool found = false;

    view.each([&found, playerNetworkId](ECS::Entity, const LaserBeamTag&,
                                        const LaserBeamComponent& beam) {
        if (beam.ownerNetworkId == playerNetworkId && beam.isActive()) {
            found = true;
        }
    });

    return found;
}

void LaserBeamSystem::startLaser(ECS::Registry& registry,
                                 ECS::Entity playerEntity,
                                 uint32_t playerNetworkId) {
    auto existingView = registry.view<LaserBeamTag, LaserBeamComponent>();
    bool hasExisting = false;
    ECS::Entity existingBeam;

    existingView.each([&hasExisting, &existingBeam, playerNetworkId](
                          ECS::Entity entity, const LaserBeamTag&,
                          const LaserBeamComponent& beam) {
        if (beam.ownerNetworkId == playerNetworkId) {
            hasExisting = true;
            existingBeam = entity;
        }
    });

    if (hasExisting) {
        auto& beam = registry.getComponent<LaserBeamComponent>(existingBeam);
        if (beam.canFire()) {
            startFiringBeam(beam);
            if (registry.hasComponent<TransformComponent>(playerEntity)) {
                const auto& pos =
                    registry.getComponent<TransformComponent>(playerEntity);
                if (registry.hasComponent<NetworkIdComponent>(existingBeam)) {
                    const auto& netId =
                        registry.getComponent<NetworkIdComponent>(existingBeam);
                    emitBeamSpawn(netId.networkId, pos.x + _config.offsetX,
                                  pos.y, playerNetworkId);
                }
            }
            LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                          "[LaserBeamSystem] Reactivated laser beam for player="
                              << playerNetworkId);
        }
        return;
    }

    if (!registry.hasComponent<TransformComponent>(playerEntity)) {
        return;
    }
    const auto& playerPos =
        registry.getComponent<TransformComponent>(playerEntity);

    ECS::Entity beamEntity = registry.spawnEntity();

    registry.emplaceComponent<TransformComponent>(
        beamEntity, playerPos.x + _config.offsetX, playerPos.y, 0.0F);
    registry.emplaceComponent<BoundingBoxComponent>(
        beamEntity, _config.hitboxWidth, _config.hitboxHeight);
    registry.emplaceComponent<LaserBeamTag>(beamEntity);

    LaserBeamComponent beamComp;
    beamComp.ownerNetworkId = playerNetworkId;
    beamComp.maxDuration = _config.maxDuration;
    beamComp.cooldownDuration = _config.cooldownDuration;
    startFiringBeam(beamComp);
    registry.emplaceComponent<LaserBeamComponent>(beamEntity, beamComp);

    shared::DamageOnContactComponent dmgComp;
    dmgComp.damagePerSecond = _config.damagePerSecond;
    dmgComp.isDPS = true;
    dmgComp.destroySelf = false;
    dmgComp.ownerNetworkId = playerNetworkId;
    dmgComp.startupDelay = _config.startupDelay;
    dmgComp.activeTime = 0.0F;
    registry.emplaceComponent<shared::DamageOnContactComponent>(beamEntity,
                                                                dmgComp);

    uint32_t beamNetworkId = _nextBeamNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(beamEntity, beamNetworkId);

    emitBeamSpawn(beamNetworkId, playerPos.x + _config.offsetX, playerPos.y,
                  playerNetworkId);

    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[LaserBeamSystem] Created laser beam entity="
                      << beamEntity.id << " networkId=" << beamNetworkId
                      << " for player=" << playerNetworkId);
}

void LaserBeamSystem::stopLaser(ECS::Registry& registry,
                                uint32_t playerNetworkId) {
    auto view =
        registry.view<LaserBeamTag, LaserBeamComponent, NetworkIdComponent>();

    view.each([this, playerNetworkId](
                  ECS::Entity /*entity*/, const LaserBeamTag&,
                  LaserBeamComponent& beam, const NetworkIdComponent& netId) {
        if (beam.ownerNetworkId == playerNetworkId && beam.isActive()) {
            stopFiringBeam(beam);
            emitBeamDestroy(netId.networkId);
            LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                          "[LaserBeamSystem] Stopped laser beam networkId="
                              << netId.networkId
                              << " for player=" << playerNetworkId);
        }
    });
}

void LaserBeamSystem::updateActiveBeams(ECS::Registry& registry,
                                        float deltaTime) {
    auto view =
        registry.view<LaserBeamTag, LaserBeamComponent, NetworkIdComponent>();

    view.each([this, &registry, deltaTime](
                  ECS::Entity entity, const LaserBeamTag&,
                  LaserBeamComponent& beam, const NetworkIdComponent& netId) {
        bool wasActive = beam.isActive();
        bool forceStop = updateBeamState(beam, deltaTime);

        if (registry.hasComponent<shared::DamageOnContactComponent>(entity)) {
            auto& dmgComp =
                registry.getComponent<shared::DamageOnContactComponent>(entity);
            dmgComp.activeTime = beam.activeTime;
        }

        if (wasActive && forceStop) {
            emitBeamDestroy(netId.networkId);
            LOG_DEBUG_CAT(
                ::rtype::LogCategory::GameEngine,
                "[LaserBeamSystem] Max duration reached for beam networkId="
                    << netId.networkId);
        }
    });
}

void LaserBeamSystem::updateBeamPositions(ECS::Registry& registry) {
    auto beamView =
        registry.view<LaserBeamTag, LaserBeamComponent, TransformComponent>();

    beamView.each([this, &registry](ECS::Entity, const LaserBeamTag&,
                                    const LaserBeamComponent& beam,
                                    TransformComponent& beamPos) {
        if (!beam.isActive()) {
            return;
        }

        auto it = _playerCache.find(beam.ownerNetworkId);
        if (it == _playerCache.end()) {
            return;
        }

        ECS::Entity playerEntity = it->second;
        if (!registry.hasComponent<TransformComponent>(playerEntity)) {
            return;
        }

        const auto& playerPos =
            registry.getComponent<TransformComponent>(playerEntity);
        beamPos.x = playerPos.x + _config.offsetX;
        beamPos.y = playerPos.y;
    });
}

void LaserBeamSystem::rebuildPlayerCache(ECS::Registry& registry) {
    _playerCache.clear();
    auto view = registry.view<PlayerTag, NetworkIdComponent>();
    view.each([this](ECS::Entity entity, const PlayerTag&,
                     const NetworkIdComponent& netId) {
        _playerCache[netId.networkId] = entity;
    });
}

void LaserBeamSystem::startFiringBeam(LaserBeamComponent& beam) {
    if (beam.canFire()) {
        beam.state = LaserBeamState::Active;
        beam.activeTime = 0.0F;
        beam.pulsePhase = 0.0F;
    }
}

void LaserBeamSystem::stopFiringBeam(LaserBeamComponent& beam) {
    if (beam.isActive()) {
        beam.state = LaserBeamState::Cooldown;
        beam.cooldownTime = beam.cooldownDuration;
    }
}

void LaserBeamSystem::forceStopBeam(LaserBeamComponent& beam) {
    beam.state = LaserBeamState::Cooldown;
    beam.cooldownTime = beam.cooldownDuration;
}

bool LaserBeamSystem::updateBeamState(LaserBeamComponent& beam,
                                      float deltaTime) {
    if (beam.state == LaserBeamState::Active) {
        beam.activeTime += deltaTime;
        beam.pulsePhase += beam.pulseSpeed * deltaTime;

        if (beam.activeTime >= beam.maxDuration) {
            forceStopBeam(beam);
            return true;
        }
    } else if (beam.state == LaserBeamState::Cooldown) {
        beam.cooldownTime -= deltaTime;
        if (beam.cooldownTime <= 0.0F) {
            beam.cooldownTime = 0.0F;
            beam.state = LaserBeamState::Inactive;
        }
    }
    return false;
}

void LaserBeamSystem::emitBeamSpawn(uint32_t beamNetworkId, float x, float y,
                                    uint32_t) {
    if (!_emitEvent) {
        return;
    }

    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = beamNetworkId;
    event.x = x;
    event.y = y;
    event.entityType = static_cast<uint8_t>(EntityType::LaserBeam);
    event.subType = 0;
    _emitEvent(event);
}

void LaserBeamSystem::emitBeamDestroy(uint32_t beamNetworkId) {
    if (!_emitEvent) {
        return;
    }

    engine::GameEvent event{};
    event.type = engine::GameEventType::EntityDestroyed;
    event.entityNetworkId = beamNetworkId;
    _emitEvent(event);
}

}  // namespace rtype::games::rtype::server
