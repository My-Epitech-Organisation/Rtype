/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CollisionSystem - Server-side collision handling using QuadTree + AABB
*/

#define NOMINMAX
#include "CollisionSystem.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>
#include <rtype/network/Protocol.hpp>

#include "../../../shared/Components.hpp"
#include "../../../shared/Systems/Collision/AABB.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

// Compile-time assertion: Entity IDs must fit in 32 bits for collision pair
// hashing
static_assert(sizeof(ECS::Entity::id) == sizeof(std::uint32_t),
              "Entity ID must be 32-bit for collision pair ID generation");

/**
 * @brief Generate a unique 64-bit collision pair ID from two entity IDs
 * @details Uses bit-shifting to combine two 32-bit entity IDs into a single
 * 64-bit value. The smaller ID is placed in the upper 32 bits to ensure
 * consistent ordering.
 * @param a First entity
 * @param b Second entity
 * @return Unique 64-bit collision pair identifier
 */
[[nodiscard]] inline constexpr std::uint64_t makeCollisionPairId(
    ECS::Entity a, ECS::Entity b) noexcept {
    const std::uint32_t id1 = std::min(a.id, b.id);
    const std::uint32_t id2 = std::max(a.id, b.id);
    return (static_cast<std::uint64_t>(id1) << 32) |
           static_cast<std::uint64_t>(id2);
}

using shared::ActivePowerUpComponent;
using shared::BoundingBoxComponent;
using shared::CollisionPair;
using shared::DamageOnContactComponent;
using shared::DestroyTag;
using shared::EnemyProjectileTag;
using shared::EnemyTag;
using shared::EntityType;
using shared::HealthComponent;
using shared::InvincibleTag;
using shared::LaserBeamTag;
using shared::NetworkIdComponent;
using shared::ObstacleTag;
using shared::PickupTag;
using shared::PlayerProjectileTag;
using shared::PlayerTag;
using shared::PowerUpComponent;
using shared::ProjectileComponent;
using shared::ProjectileOwner;
using shared::ProjectileTag;
using shared::QuadTreeSystem;
using shared::TransformComponent;
using shared::WeaponComponent;
using shared::collision::overlaps;
using shared::collision::Rect;

CollisionSystem::CollisionSystem(EventEmitter emitter, float worldWidth,
                                 float worldHeight)
    : ASystem("CollisionSystem"), _emitEvent(std::move(emitter)) {
    Rect worldBounds(0, 0, worldWidth, worldHeight);
    _quadTreeSystem = std::make_unique<QuadTreeSystem>(worldBounds, 10, 5);
}

void CollisionSystem::update(ECS::Registry& registry, float deltaTime) {
    _quadTreeSystem->update(registry, deltaTime);
    auto collisionPairs = _quadTreeSystem->queryCollisionPairs(registry);
    ECS::CommandBuffer cmdBuffer(std::ref(registry));

    _laserDamagedThisFrame.clear();
    _obstacleCollidedThisFrame.clear();

    for (const auto& pair : collisionPairs) {
        ECS::Entity entityA = pair.entityA;
        ECS::Entity entityB = pair.entityB;

        if (!registry.isAlive(entityA) || !registry.isAlive(entityB)) {
            continue;
        }
        if (registry.hasComponent<DestroyTag>(entityA) ||
            registry.hasComponent<DestroyTag>(entityB)) {
            continue;
        }

        if (!registry.hasComponent<TransformComponent>(entityA) ||
            !registry.hasComponent<TransformComponent>(entityB) ||
            !registry.hasComponent<BoundingBoxComponent>(entityA) ||
            !registry.hasComponent<BoundingBoxComponent>(entityB)) {
            continue;
        }

        const auto& transformA =
            registry.getComponent<TransformComponent>(entityA);
        const auto& transformB =
            registry.getComponent<TransformComponent>(entityB);
        const auto& boxA = registry.getComponent<BoundingBoxComponent>(entityA);
        const auto& boxB = registry.getComponent<BoundingBoxComponent>(entityB);

        if (!overlaps(transformA, boxA, transformB, boxB)) {
            continue;
        }

        bool aIsProjectile = registry.hasComponent<ProjectileTag>(entityA);
        bool bIsProjectile = registry.hasComponent<ProjectileTag>(entityB);
        bool aIsEnemy = registry.hasComponent<EnemyTag>(entityA);
        bool bIsEnemy = registry.hasComponent<EnemyTag>(entityB);
        bool aIsPlayer = registry.hasComponent<PlayerTag>(entityA);
        bool bIsPlayer = registry.hasComponent<PlayerTag>(entityB);
        bool aIsPickup = registry.hasComponent<PickupTag>(entityA);
        bool bIsPickup = registry.hasComponent<PickupTag>(entityB);
        bool aIsObstacle = registry.hasComponent<ObstacleTag>(entityA);
        bool bIsObstacle = registry.hasComponent<ObstacleTag>(entityB);
        bool aIsLaser = registry.hasComponent<LaserBeamTag>(entityA);
        bool bIsLaser = registry.hasComponent<LaserBeamTag>(entityB);
        bool aHasHealth = registry.hasComponent<HealthComponent>(entityA);
        bool bHasHealth = registry.hasComponent<HealthComponent>(entityB);

        if (aIsPickup && bIsPlayer) {
            LOG_INFO(
                "[CollisionSystem] Pickup-Player collision detected: pickup="
                << entityA.id << " player=" << entityB.id);
            handlePickupCollision(registry, cmdBuffer, entityB, entityA);
            continue;
        }
        if (bIsPickup && aIsPlayer) {
            LOG_INFO(
                "[CollisionSystem] Player-Pickup collision detected: player="
                << entityA.id << " pickup=" << entityB.id);
            handlePickupCollision(registry, cmdBuffer, entityA, entityB);
            continue;
        }

        if (aIsObstacle && (bIsPlayer || bIsProjectile)) {
            handleObstacleCollision(registry, cmdBuffer, entityA, entityB,
                                    bIsPlayer);
            continue;
        }
        if (bIsObstacle && (aIsPlayer || aIsProjectile)) {
            handleObstacleCollision(registry, cmdBuffer, entityB, entityA,
                                    aIsPlayer);
            continue;
        }

        if (aIsLaser && bIsEnemy) {
            handleLaserEnemyCollision(registry, cmdBuffer, entityA, entityB,
                                      deltaTime);
            continue;
        }
        if (bIsLaser && aIsEnemy) {
            handleLaserEnemyCollision(registry, cmdBuffer, entityB, entityA,
                                      deltaTime);
            continue;
        }

        if (aIsProjectile && (bIsEnemy || bIsPlayer || bHasHealth)) {
            handleProjectileCollision(registry, cmdBuffer, entityA, entityB,
                                      bIsPlayer);
        } else if (bIsProjectile && (aIsEnemy || aIsPlayer || aHasHealth)) {
            handleProjectileCollision(registry, cmdBuffer, entityB, entityA,
                                      aIsPlayer);
        }
        if (aIsEnemy && bIsPlayer) {
            handleEnemyPlayerCollision(registry, cmdBuffer, entityA, entityB);
        } else if (bIsEnemy && aIsPlayer) {
            handleEnemyPlayerCollision(registry, cmdBuffer, entityB, entityA);
        }
    }
    cmdBuffer.flush();
}

void CollisionSystem::handleProjectileCollision(ECS::Registry& registry,
                                                ECS::CommandBuffer& cmdBuffer,
                                                ECS::Entity projectile,
                                                ECS::Entity target,
                                                bool isTargetPlayer) {
    if (registry.hasComponent<DestroyTag>(projectile) ||
        registry.hasComponent<DestroyTag>(target)) {
        return;
    }

    ProjectileOwner projOwner = ProjectileOwner::Neutral;
    int32_t damage = 25;
    bool piercing = false;

    if (registry.hasComponent<ProjectileComponent>(projectile)) {
        const auto& projComp =
            registry.getComponent<ProjectileComponent>(projectile);
        projOwner = projComp.owner;
        damage = projComp.damage;
        piercing = projComp.piercing;
    } else {
        if (registry.hasComponent<PlayerProjectileTag>(projectile)) {
            projOwner = ProjectileOwner::Player;
        } else if (registry.hasComponent<EnemyProjectileTag>(projectile)) {
            projOwner = ProjectileOwner::Enemy;
        }
    }

    bool canHit = false;
    if (projOwner == ProjectileOwner::Neutral) {
        canHit = true;
    } else if (projOwner == ProjectileOwner::Player && !isTargetPlayer) {
        canHit = true;
    } else if (projOwner == ProjectileOwner::Enemy && isTargetPlayer) {
        canHit = true;
    }

    if (!canHit) {
        return;
    }

    if (isTargetPlayer && registry.hasComponent<InvincibleTag>(target)) {
        return;
    }

    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[CollisionSystem] Collision detected! Projectile "
                      << projectile.id << " hit target " << target.id
                      << " (isPlayer=" << isTargetPlayer << ")");

    if (registry.hasComponent<HealthComponent>(target)) {
        auto& health = registry.getComponent<HealthComponent>(target);
        const int32_t prevHealth = health.current;
        health.takeDamage(damage);
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[CollisionSystem] Health after damage: "
                          << prevHealth << " -> " << health.current
                          << " (damage=" << damage << ")");

        if (_emitEvent && registry.hasComponent<NetworkIdComponent>(target)) {
            const auto& netId =
                registry.getComponent<NetworkIdComponent>(target);
            if (netId.isValid()) {
                ::rtype::network::EntityType entityType =
                    isTargetPlayer ? ::rtype::network::EntityType::Player
                                   : ::rtype::network::EntityType::Bydos;
                LOG_INFO("[CollisionSystem] Emitting EntityHealthChanged for "
                         << (isTargetPlayer ? "player" : "enemy")
                         << " networkId=" << netId.networkId
                         << " health=" << health.current << "/" << health.max
                         << " damage=" << damage);
                engine::GameEvent event{};
                event.type = engine::GameEventType::EntityHealthChanged;
                event.entityNetworkId = netId.networkId;
                event.entityType = static_cast<uint8_t>(entityType);
                event.healthCurrent = health.current;
                event.healthMax = health.max;
                event.damage = damage;
                _emitEvent(event);
            }
        }

        if (!health.isAlive()) {
            LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                          "[CollisionSystem] Target "
                              << target.id << " destroyed (no health)");
            cmdBuffer.emplaceComponentDeferred<DestroyTag>(target,
                                                           DestroyTag{});
        }
    } else {
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[CollisionSystem] Target "
                          << target.id << " destroyed (no HealthComponent)");
        cmdBuffer.emplaceComponentDeferred<DestroyTag>(target, DestroyTag{});
    }

    if (!piercing) {
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[CollisionSystem] Projectile "
                          << projectile.id << " destroyed (non-piercing)");
        cmdBuffer.emplaceComponentDeferred<DestroyTag>(projectile,
                                                       DestroyTag{});
    } else if (registry.hasComponent<ProjectileComponent>(projectile)) {
        auto& projComp = registry.getComponent<ProjectileComponent>(projectile);
        if (projComp.registerHit()) {
            LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                          "[CollisionSystem] Projectile "
                              << projectile.id << " destroyed (max hits)");
            cmdBuffer.emplaceComponentDeferred<DestroyTag>(projectile,
                                                           DestroyTag{});
        }
    }
}

void CollisionSystem::handlePickupCollision(ECS::Registry& registry,
                                            ECS::CommandBuffer& cmdBuffer,
                                            ECS::Entity player,
                                            ECS::Entity pickup) {
    LOG_INFO("[CollisionSystem] handlePickupCollision called: player="
             << player.id << " pickup=" << pickup.id);

    if (registry.hasComponent<DestroyTag>(pickup) ||
        registry.hasComponent<DestroyTag>(player)) {
        LOG_INFO("[CollisionSystem] Entity already has DestroyTag, skipping");
        return;
    }

    if (!registry.hasComponent<PowerUpComponent>(pickup)) {
        LOG_WARNING("[CollisionSystem] Pickup entity "
                    << pickup.id << " missing PowerUpComponent!");
        return;
    }

    const auto& powerUp = registry.getComponent<PowerUpComponent>(pickup);

    LOG_INFO("[CollisionSystem] PowerUp type="
             << static_cast<int>(powerUp.type) << " duration="
             << powerUp.duration << " magnitude=" << powerUp.magnitude);

    if (powerUp.type == shared::PowerUpType::None) {
        LOG_DEBUG_CAT(
            ::rtype::LogCategory::GameEngine,
            "[CollisionSystem] Ignoring pickup with PowerUpType::None");
        cmdBuffer.emplaceComponentDeferred<DestroyTag>(pickup, DestroyTag{});
        return;
    }

    ActivePowerUpComponent* activePtr = nullptr;
    if (registry.hasComponent<ActivePowerUpComponent>(player)) {
        auto& existing = registry.getComponent<ActivePowerUpComponent>(player);
        if (existing.shieldActive &&
            registry.hasComponent<shared::InvincibleTag>(player)) {
            registry.removeComponent<shared::InvincibleTag>(player);
        }
        if (existing.hasOriginalCooldown &&
            registry.hasComponent<shared::ShootCooldownComponent>(player)) {
            auto& cd =
                registry.getComponent<shared::ShootCooldownComponent>(player);
            cd.setCooldownTime(existing.originalCooldown);
        }
        existing = ActivePowerUpComponent{};
        activePtr = &existing;
    } else {
        activePtr = &registry.emplaceComponent<ActivePowerUpComponent>(
            player, ActivePowerUpComponent{});
    }

    auto& active = *activePtr;

    active.type = powerUp.type;
    active.remainingTime = powerUp.duration;
    active.speedMultiplier = 1.0F;
    active.fireRateMultiplier = 1.0F;
    active.damageMultiplier = 1.0F;
    active.shieldActive = false;
    active.hasOriginalCooldown = false;

    switch (powerUp.type) {
        case shared::PowerUpType::SpeedBoost:
            active.speedMultiplier = 1.0F + powerUp.magnitude;
            break;
        case shared::PowerUpType::Shield:
            active.shieldActive = true;
            if (!registry.hasComponent<shared::InvincibleTag>(player)) {
                registry.emplaceComponent<shared::InvincibleTag>(player);
            }
            break;
        case shared::PowerUpType::RapidFire:
            active.fireRateMultiplier = 1.0F + powerUp.magnitude;
            if (registry.hasComponent<shared::ShootCooldownComponent>(player)) {
                auto& cd =
                    registry.getComponent<shared::ShootCooldownComponent>(
                        player);
                active.originalCooldown = cd.cooldownTime;
                active.hasOriginalCooldown = true;
                float factor = 1.0F / active.fireRateMultiplier;
                cd.setCooldownTime(std::max(0.05F, cd.cooldownTime * factor));
            }
            break;
        case shared::PowerUpType::DoubleDamage:
            active.damageMultiplier = 1.0F + powerUp.magnitude;
            break;
        case shared::PowerUpType::HealthBoost:
            if (registry.hasComponent<HealthComponent>(player)) {
                auto& health = registry.getComponent<HealthComponent>(player);
                int32_t healthBoost =
                    static_cast<int32_t>(powerUp.magnitude * 100.0F);
                health.current =
                    std::min(health.current + healthBoost, health.max);
            }
            break;
        case shared::PowerUpType::ForcePod:
            LOG_INFO("[CollisionSystem] Spawning Force Pod for player="
                     << player.id);
            if (registry.hasComponent<NetworkIdComponent>(player)) {
                const auto& playerNetId =
                    registry.getComponent<NetworkIdComponent>(player);
                int existingPodCount = 0;
                auto view =
                    registry
                        .view<shared::ForcePodTag, shared::ForcePodComponent>();
                view.each([&existingPodCount, &playerNetId](
                              ECS::Entity /*entity*/,
                              const shared::ForcePodTag&,
                              const shared::ForcePodComponent& podComp) {
                    if (podComp.ownerNetworkId == playerNetId.networkId) {
                        existingPodCount++;
                    }
                });

                const float distance = 60.0F;
                const std::vector<std::pair<float, float>> positions = {
                    {0.0F, -distance},
                    {0.0F, distance},
                    {distance, 0.0F},
                    {-distance, 0.0F},
                    {distance * 0.7F, -distance * 0.7F},
                    {distance * 0.7F, distance * 0.7F},
                    {-distance * 0.7F, -distance * 0.7F},
                    {-distance * 0.7F, distance * 0.7F}};

                float offsetX = 0.0F;
                float offsetY = 0.0F;
                if (existingPodCount < static_cast<int>(positions.size())) {
                    offsetX = positions[existingPodCount].first;
                    offsetY = positions[existingPodCount].second;
                } else {
                    const float angle =
                        2.0F * 3.14159265359F * existingPodCount / 8.0F;
                    offsetX = distance * std::cos(angle);
                    offsetY = distance * std::sin(angle);
                }

                LOG_INFO(
                    "[CollisionSystem] Creating Force Pod entity with "
                    "parentNetId="
                    << playerNetId.networkId << " at position "
                    << existingPodCount << " (offset: " << offsetX << ", "
                    << offsetY << ")");

                ECS::Entity forcePod = registry.spawnEntity();
                registry.emplaceComponent<shared::ForcePodComponent>(
                    forcePod, shared::ForcePodState::Attached, offsetX, offsetY,
                    playerNetId.networkId);
                registry.emplaceComponent<shared::PlayerTag>(forcePod);
                registry.emplaceComponent<shared::ForcePodTag>(forcePod);
                registry.emplaceComponent<TransformComponent>(forcePod, 0.0F,
                                                              0.0F, 0.0F);
                registry.emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F,
                                                                32.0F);

                if (_emitEvent) {
                    uint32_t forcePodNetId =
                        playerNetId.networkId + 10000 + existingPodCount;
                    registry.emplaceComponent<NetworkIdComponent>(
                        forcePod, forcePodNetId);

                    LOG_INFO(
                        "[CollisionSystem] Emitting ForcePod spawn event: "
                        "networkId="
                        << forcePodNetId);

                    engine::GameEvent evt{};
                    evt.type = engine::GameEventType::EntitySpawned;
                    evt.entityNetworkId = forcePodNetId;
                    evt.entityType = 5;
                    evt.x = 0.0F;
                    evt.y = 0.0F;
                    _emitEvent(evt);
                }
            } else {
                LOG_INFO("[CollisionSystem] Player missing NetworkIdComponent");
            }
            break;
        case shared::PowerUpType::LaserUpgrade:
            LOG_INFO("[CollisionSystem] Applying LaserUpgrade for player="
                     << player.id);
            if (registry.hasComponent<WeaponComponent>(player)) {
                auto& weapon = registry.getComponent<WeaponComponent>(player);
                weapon.unlockSlot();
                uint8_t newSlot = weapon.unlockedSlots - 1;
                if (newSlot < shared::MAX_WEAPON_SLOTS) {
                    weapon.weapons[newSlot] =
                        shared::WeaponPresets::ContinuousLaser;
                    LOG_INFO("[CollisionSystem] Laser weapon added to slot "
                             << static_cast<int>(newSlot));
                }
            }
            break;
        case shared::PowerUpType::None:
        default:
            break;
    }

    if (_emitEvent && registry.hasComponent<NetworkIdComponent>(player) &&
        registry.getComponent<NetworkIdComponent>(player).isValid()) {
        const auto& netId = registry.getComponent<NetworkIdComponent>(player);
        engine::GameEvent evt{};
        evt.type = engine::GameEventType::PowerUpApplied;
        evt.entityNetworkId = netId.networkId;
        evt.subType = static_cast<uint8_t>(powerUp.type);
        evt.duration = powerUp.duration;
        _emitEvent(evt);

        LOG_INFO("[CollisionSystem] Emitted PowerUpApplied event: playerId="
                 << netId.networkId
                 << " type=" << static_cast<int>(powerUp.type)
                 << " duration=" << powerUp.duration);
    }

    cmdBuffer.emplaceComponentDeferred<DestroyTag>(pickup, DestroyTag{});
}

void CollisionSystem::handleObstacleCollision(ECS::Registry& registry,
                                              ECS::CommandBuffer& cmdBuffer,
                                              ECS::Entity obstacle,
                                              ECS::Entity other,
                                              bool otherIsPlayer) {
    if (registry.hasComponent<DestroyTag>(obstacle) ||
        registry.hasComponent<DestroyTag>(other)) {
        return;
    }

    const std::uint64_t collisionPairId = makeCollisionPairId(obstacle, other);

    if (_obstacleCollidedThisFrame.find(collisionPairId) !=
        _obstacleCollidedThisFrame.end()) {
        return;
    }
    _obstacleCollidedThisFrame.insert(collisionPairId);

    int32_t damage = 15;
    if (registry.hasComponent<DamageOnContactComponent>(obstacle)) {
        const auto& dmgComp =
            registry.getComponent<DamageOnContactComponent>(obstacle);
        damage = dmgComp.damage;
    }

    if (otherIsPlayer) {
        if (registry.hasComponent<shared::InvincibleTag>(other)) {
            return;
        }
        if (registry.hasComponent<HealthComponent>(other)) {
            auto& health = registry.getComponent<HealthComponent>(other);
            health.takeDamage(damage);
            if (!health.isAlive()) {
                cmdBuffer.emplaceComponentDeferred<DestroyTag>(other,
                                                               DestroyTag{});
            }
            if (_emitEvent &&
                registry.hasComponent<NetworkIdComponent>(other) &&
                registry.getComponent<NetworkIdComponent>(other).isValid()) {
                const auto& netId =
                    registry.getComponent<NetworkIdComponent>(other);
                engine::GameEvent evt{};
                evt.type = engine::GameEventType::EntityHealthChanged;
                evt.entityNetworkId = netId.networkId;
                evt.entityType =
                    static_cast<uint8_t>(::rtype::network::EntityType::Player);
                evt.healthCurrent = health.current;
                evt.healthMax = health.max;
                _emitEvent(evt);
            }
        } else {
            cmdBuffer.emplaceComponentDeferred<DestroyTag>(other, DestroyTag{});
        }
        cmdBuffer.emplaceComponentDeferred<DestroyTag>(obstacle, DestroyTag{});
    } else {
        cmdBuffer.emplaceComponentDeferred<DestroyTag>(other, DestroyTag{});
        cmdBuffer.emplaceComponentDeferred<DestroyTag>(obstacle, DestroyTag{});
    }
}

void CollisionSystem::handleEnemyPlayerCollision(ECS::Registry& registry,
                                                 ECS::CommandBuffer& cmdBuffer,
                                                 ECS::Entity enemy,
                                                 ECS::Entity player) {
    if (registry.hasComponent<DestroyTag>(enemy) ||
        registry.hasComponent<DestroyTag>(player)) {
        return;
    }
    if (registry.hasComponent<InvincibleTag>(player)) {
        return;
    }
    if (!registry.hasComponent<DamageOnContactComponent>(enemy)) {
        return;
    }
    const auto& damageComp =
        registry.getComponent<DamageOnContactComponent>(enemy);
    int32_t damage = damageComp.damage;

    LOG_DEBUG("[CollisionSystem] Enemy " << enemy.id << " collided with player "
                                         << player.id << " (damage=" << damage
                                         << ")");
    if (registry.hasComponent<HealthComponent>(player)) {
        auto& health = registry.getComponent<HealthComponent>(player);
        const int32_t prevHealth = health.current;
        health.takeDamage(damage);
        LOG_DEBUG("[CollisionSystem] Player health: " << prevHealth << " -> "
                                                      << health.current);
        if (_emitEvent && registry.hasComponent<NetworkIdComponent>(player)) {
            const auto& netId =
                registry.getComponent<NetworkIdComponent>(player);
            if (netId.isValid()) {
                engine::GameEvent event{};
                event.type = engine::GameEventType::EntityHealthChanged;
                event.entityNetworkId = netId.networkId;
                event.entityType =
                    static_cast<uint8_t>(::rtype::network::EntityType::Player);
                event.healthCurrent = health.current;
                event.healthMax = health.max;
                _emitEvent(event);
            }
        }
        if (!health.isAlive()) {
            LOG_DEBUG("[CollisionSystem] Player " << player.id << " destroyed");
            cmdBuffer.emplaceComponentDeferred<DestroyTag>(player,
                                                           DestroyTag{});
        }
    }
    if (damageComp.destroySelf) {
        LOG_DEBUG("[CollisionSystem] Enemy " << enemy.id
                                             << " destroyed on contact");
        cmdBuffer.emplaceComponentDeferred<DestroyTag>(enemy, DestroyTag{});
    }
}

void CollisionSystem::handleLaserEnemyCollision(ECS::Registry& registry,
                                                ECS::CommandBuffer& cmdBuffer,
                                                ECS::Entity laser,
                                                ECS::Entity enemy,
                                                float deltaTime) {
    if (registry.hasComponent<DestroyTag>(laser) ||
        registry.hasComponent<DestroyTag>(enemy)) {
        return;
    }

    if (!registry.hasComponent<DamageOnContactComponent>(laser)) {
        return;
    }

    auto& dmgComp = registry.getComponent<DamageOnContactComponent>(laser);

    if (!dmgComp.isActive()) {
        return;
    }

    if (!registry.hasComponent<HealthComponent>(enemy)) {
        return;
    }

    uint32_t laserNetId = 0;
    uint32_t enemyNetId = 0;

    if (registry.hasComponent<NetworkIdComponent>(laser)) {
        laserNetId = registry.getComponent<NetworkIdComponent>(laser).networkId;
    }
    if (registry.hasComponent<NetworkIdComponent>(enemy)) {
        enemyNetId = registry.getComponent<NetworkIdComponent>(enemy).networkId;
    }

    uint64_t pairKey = (static_cast<uint64_t>(laserNetId) << 32) | enemyNetId;
    if (_laserDamagedThisFrame.count(pairKey) > 0) {
        return;
    }
    _laserDamagedThisFrame.insert(pairKey);

    int32_t damage = dmgComp.calculateDamage(deltaTime);
    auto& health = registry.getComponent<HealthComponent>(enemy);
    int32_t prevHealth = health.current;
    health.takeDamage(damage);

    LOG_DEBUG("[CollisionSystem] Laser DPS hit enemy "
              << enemy.id << ": " << prevHealth << " -> " << health.current
              << " (damage=" << damage << ")");

    if (_emitEvent && registry.hasComponent<NetworkIdComponent>(enemy)) {
        const auto& netId = registry.getComponent<NetworkIdComponent>(enemy);
        if (netId.isValid()) {
            engine::GameEvent event{};
            event.type = engine::GameEventType::EntityHealthChanged;
            event.entityNetworkId = netId.networkId;
            event.entityType =
                static_cast<uint8_t>(::rtype::network::EntityType::Bydos);
            event.healthCurrent = health.current;
            event.healthMax = health.max;
            event.damage = damage;
            _emitEvent(event);
        }
    }

    if (!health.isAlive()) {
        LOG_DEBUG("[CollisionSystem] Enemy " << enemy.id
                                             << " destroyed by laser");
        cmdBuffer.emplaceComponentDeferred<DestroyTag>(enemy, DestroyTag{});
    }
}

}  // namespace rtype::games::rtype::server
