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
using shared::EnemyTag;
using shared::HealthComponent;
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
    _damagedThisFrame.clear();
    updateActiveBeams(registry, deltaTime);
    updateBeamPositions(registry);
    performBeamCollisions(registry, deltaTime);
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

    // Add components (spawn in front of player)
    registry.emplaceComponent<TransformComponent>(
        beamEntity, playerPos.x + kLaserOffsetX, playerPos.y, 0.0F);
    registry.emplaceComponent<BoundingBoxComponent>(beamEntity, 0.0F,
                                                    16.0F);  // Width grows
    registry.emplaceComponent<LaserBeamTag>(beamEntity);

    LaserBeamComponent beamComp;
    beamComp.ownerNetworkId = playerNetworkId;
    beamComp.startFiring();
    registry.emplaceComponent<LaserBeamComponent>(beamEntity, beamComp);

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
                              BoundingBoxComponent, NetworkIdComponent>();

    view.each([this, deltaTime](ECS::Entity /*entity*/, const LaserBeamTag&,
                                LaserBeamComponent& beam,
                                BoundingBoxComponent& bbox,
                                const NetworkIdComponent& netId) {
        bool wasActive = beam.isActive();
        bool forceStop = beam.update(deltaTime);

        // Update bounding box width to match beam length
        if (beam.isActive()) {
            bbox.width = beam.beamLength;
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

void LaserBeamSystem::performBeamCollisions(ECS::Registry& registry,
                                            float deltaTime) {
    auto beamView = registry.view<LaserBeamTag, LaserBeamComponent,
                                  TransformComponent>();

    beamView.each([this, &registry, deltaTime](
                      ECS::Entity /*beamEntity*/, const LaserBeamTag&,
                      const LaserBeamComponent& beam,
                      const TransformComponent& beamPos) {
        if (!beam.isActive() || beam.beamLength <= 0.0F) {
            return;
        }

        // Calculate beam rectangle (origin to origin + length)
        float beamLeft = beamPos.x;
        float beamRight = beamPos.x + beam.beamLength;
        float beamTop = beamPos.y - beam.beamWidth / 2.0F;
        float beamBottom = beamPos.y + beam.beamWidth / 2.0F;

        // Check against all enemies
        auto enemyView =
            registry.view<EnemyTag, TransformComponent, BoundingBoxComponent,
                          HealthComponent, NetworkIdComponent>();

        enemyView.each([this, &registry, deltaTime, &beam, beamLeft, beamRight,
                        beamTop, beamBottom](
                           ECS::Entity enemyEntity, const EnemyTag&,
                           const TransformComponent& enemyPos,
                           const BoundingBoxComponent& enemyBox,
                           HealthComponent& /*health*/,
                           const NetworkIdComponent& enemyNetId) {
            // Calculate enemy bounds
            float enemyHalfW = enemyBox.width / 2.0F;
            float enemyHalfH = enemyBox.height / 2.0F;
            float enemyLeft = enemyPos.x - enemyHalfW;
            float enemyRight = enemyPos.x + enemyHalfW;
            float enemyTop = enemyPos.y - enemyHalfH;
            float enemyBottom = enemyPos.y + enemyHalfH;

            // AABB overlap check
            bool overlaps = !(beamRight < enemyLeft || enemyRight < beamLeft ||
                              beamBottom < enemyTop || enemyBottom < beamTop);

            if (overlaps) {
                // Create unique key for this beam-enemy pair this frame
                uint64_t pairKey =
                    (static_cast<uint64_t>(beam.ownerNetworkId) << 32) |
                    enemyNetId.networkId;

                // Only damage once per frame
                if (_damagedThisFrame.find(pairKey) ==
                    _damagedThisFrame.end()) {
                    _damagedThisFrame.insert(pairKey);

                    // Apply DPS damage
                    float damage = beam.damagePerSecond * deltaTime;
                    applyDamage(registry, enemyEntity, damage,
                                beam.ownerNetworkId);
                }
            }
        });
    });
}

void LaserBeamSystem::applyDamage(ECS::Registry& registry, ECS::Entity target,
                                  float damage, uint32_t attackerNetworkId) {
    if (!registry.hasComponent<HealthComponent>(target)) {
        return;
    }

    auto& health = registry.getComponent<HealthComponent>(target);
    int32_t damageInt = static_cast<int32_t>(damage);
    if (damageInt < 1) {
        damageInt = 1;  // Minimum 1 damage per frame
    }

    health.takeDamage(damageInt);

    // Emit health changed event
    if (_emitEvent && registry.hasComponent<NetworkIdComponent>(target)) {
        const auto& netId = registry.getComponent<NetworkIdComponent>(target);

        engine::GameEvent event{};
        event.type = engine::GameEventType::EntityHealthChanged;
        event.entityNetworkId = netId.networkId;
        event.entityType = 1;  // Bydos
        event.healthCurrent = health.current;
        event.healthMax = health.max;
        event.damage = damageInt;
        _emitEvent(event);
    }

    // Mark for destruction if dead
    if (!health.isAlive()) {
        if (!registry.hasComponent<shared::DestroyTag>(target)) {
            registry.emplaceComponent<shared::DestroyTag>(target);
        }
    }
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
