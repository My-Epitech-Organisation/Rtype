/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** LaserBeamSystem - Implementation
*/

#include "LaserBeamSystem.hpp"

#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

using shared::BoundingBoxComponent;
using shared::DamageOnContactComponent;
using shared::LaserBeamComponent;
using shared::LaserBeamState;
using shared::LaserBeamTag;
using shared::NetworkIdComponent;
using shared::PlayerTag;
using shared::TransformComponent;

// Offset from player center to laser center position
// Laser sprite is 614px wide (3072 * 0.2 scale), rendered centered
// Half-width = 307px, plus ~32px for player gun position = 340px
// This places the laser's left visual edge at the player's front
constexpr float kLaserOffsetX = 340.0F;

LaserBeamSystem::LaserBeamSystem(EventEmitter emitter)
    : ASystem("LaserBeamSystem"), _emitEvent(std::move(emitter)) {}

void LaserBeamSystem::update(ECS::Registry& registry, float deltaTime) {
    updateActiveBeams(registry, deltaTime);
    updateBeamPositions(registry);
    // Collision detection is now handled by CollisionSystem
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

    view.each([&found, playerNetworkId](ECS::Entity /*entity*/,
                                        const LaserBeamTag&,
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
    // Check if player already has a beam (active or cooling down)
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
        // Check if beam can be fired again
        auto& beam = registry.getComponent<LaserBeamComponent>(existingBeam);
        if (beam.canFire()) {
            beam.startFiring();
            // Emit spawn event
            if (registry.hasComponent<TransformComponent>(playerEntity)) {
                const auto& pos =
                    registry.getComponent<TransformComponent>(playerEntity);
                if (registry.hasComponent<NetworkIdComponent>(existingBeam)) {
                    const auto& netId =
                        registry.getComponent<NetworkIdComponent>(existingBeam);
                    emitBeamSpawn(netId.networkId, pos.x + kLaserOffsetX, pos.y,
                                  playerNetworkId);
                }
            }
            LOG_DEBUG_CAT(
                ::rtype::LogCategory::GameEngine,
                "[LaserBeamSystem] Reactivated laser beam for player="
                    << playerNetworkId);
        }
        return;
    }

    // Get player position
    if (!registry.hasComponent<TransformComponent>(playerEntity)) {
        return;
    }
    const auto& playerPos =
        registry.getComponent<TransformComponent>(playerEntity);

    // Create new beam entity
    ECS::Entity beamEntity = registry.spawnEntity();

    // Laser beam dimensions (matching visual: 614px wide, 50px tall)
    constexpr float kBeamWidth = 614.0F;
    constexpr float kBeamHeight = 50.0F;
    constexpr float kDamagePerSecond = 50.0F;
    constexpr float kStartupDelay = 0.56F;  // 7 frames * 0.08s

    // Add components (spawn in front of player)
    registry.emplaceComponent<TransformComponent>(
        beamEntity, playerPos.x + kLaserOffsetX, playerPos.y, 0.0F);
    registry.emplaceComponent<BoundingBoxComponent>(beamEntity, kBeamWidth,
                                                    kBeamHeight);
    registry.emplaceComponent<LaserBeamTag>(beamEntity);

    LaserBeamComponent beamComp;
    beamComp.ownerNetworkId = playerNetworkId;
    beamComp.startFiring();
    registry.emplaceComponent<LaserBeamComponent>(beamEntity, beamComp);

    // Add DamageOnContactComponent for collision-based damage (DPS mode)
    shared::DamageOnContactComponent dmgComp;
    dmgComp.damagePerSecond = kDamagePerSecond;
    dmgComp.isDPS = true;
    dmgComp.destroySelf = false;
    dmgComp.ownerNetworkId = playerNetworkId;
    dmgComp.startupDelay = kStartupDelay;
    dmgComp.activeTime = 0.0F;
    registry.emplaceComponent<shared::DamageOnContactComponent>(beamEntity,
                                                                 dmgComp);

    // Assign network ID
    uint32_t beamNetworkId = _nextBeamNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(beamEntity, beamNetworkId);

    // Emit spawn event (with offset applied)
    emitBeamSpawn(beamNetworkId, playerPos.x + kLaserOffsetX, playerPos.y,
                  playerNetworkId);

    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[LaserBeamSystem] Created laser beam entity="
                      << beamEntity.id << " networkId=" << beamNetworkId
                      << " for player=" << playerNetworkId);
}

void LaserBeamSystem::stopLaser(ECS::Registry& registry,
                                uint32_t playerNetworkId) {
    auto view = registry.view<LaserBeamTag, LaserBeamComponent,
                              NetworkIdComponent>();

    view.each([this, playerNetworkId](ECS::Entity /*entity*/,
                                      const LaserBeamTag&,
                                      LaserBeamComponent& beam,
                                      const NetworkIdComponent& netId) {
        if (beam.ownerNetworkId == playerNetworkId && beam.isActive()) {
            beam.stopFiring();
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
    auto view = registry.view<LaserBeamTag, LaserBeamComponent,
                              NetworkIdComponent>();

    view.each([this, &registry, deltaTime](ECS::Entity entity,
                                           const LaserBeamTag&,
                                           LaserBeamComponent& beam,
                                           const NetworkIdComponent& netId) {
        bool wasActive = beam.isActive();
        bool forceStop = beam.update(deltaTime);

        // Synchronize activeTime with DamageOnContactComponent for startup delay
        if (registry.hasComponent<shared::DamageOnContactComponent>(entity)) {
            auto& dmgComp =
                registry.getComponent<shared::DamageOnContactComponent>(entity);
            dmgComp.activeTime = beam.activeTime;
        }

        // If max duration was reached, emit destroy event
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
    auto beamView = registry.view<LaserBeamTag, LaserBeamComponent,
                                  TransformComponent>();

    beamView.each([&registry](ECS::Entity /*beamEntity*/, const LaserBeamTag&,
                              const LaserBeamComponent& beam,
                              TransformComponent& beamPos) {
        if (!beam.isActive()) {
            return;
        }

        // Find owner player and update beam position
        auto playerView =
            registry.view<PlayerTag, NetworkIdComponent, TransformComponent>();

        playerView.each([&beam, &beamPos](ECS::Entity /*playerEntity*/,
                                          const PlayerTag&,
                                          const NetworkIdComponent& netId,
                                          const TransformComponent& playerPos) {
            if (netId.networkId == beam.ownerNetworkId) {
                // Beam origin is in front of player, extends to the right
                beamPos.x = playerPos.x + kLaserOffsetX;
                beamPos.y = playerPos.y;
            }
        });
    });
}

void LaserBeamSystem::emitBeamSpawn(uint32_t beamNetworkId, float x, float y,
                                    uint32_t /*ownerNetworkId*/) {
    if (!_emitEvent) {
        return;
    }

    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = beamNetworkId;
    event.x = x;
    event.y = y;
    event.entityType = 6;  // EntityType::LaserBeam
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
