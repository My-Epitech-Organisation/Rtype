/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** CollisionSystem - Server-side collision handling using QuadTree + AABB
*/

#define NOMINMAX
#include "CollisionSystem.hpp"

#include <algorithm>
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
        bool aHasHealth = registry.hasComponent<HealthComponent>(entityA);
        bool bHasHealth = registry.hasComponent<HealthComponent>(entityB);

        if (aIsPickup && bIsPlayer) {
            handlePickupCollision(registry, cmdBuffer, entityB, entityA);
            continue;
        }
        if (bIsPickup && aIsPlayer) {
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

        if (isTargetPlayer && _emitEvent &&
            registry.hasComponent<NetworkIdComponent>(target)) {
            const auto& netId =
                registry.getComponent<NetworkIdComponent>(target);
            if (netId.isValid()) {
                LOG_DEBUG_CAT(
                    ::rtype::LogCategory::GameEngine,
                    "[CollisionSystem] Emitting EntityHealthChanged for player "
                    "networkId="
                        << netId.networkId << " health=" << health.current
                        << "/" << health.max);
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
    if (registry.hasComponent<DestroyTag>(pickup) ||
        registry.hasComponent<DestroyTag>(player)) {
        return;
    }

    if (!registry.hasComponent<PowerUpComponent>(pickup)) {
        return;
    }

    const auto& powerUp = registry.getComponent<PowerUpComponent>(pickup);

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
            if (!registry.hasComponent<shared::ForcePodTag>(player) &&
                registry.hasComponent<NetworkIdComponent>(player)) {
                const auto& playerNetId =
                    registry.getComponent<NetworkIdComponent>(player);
                
                ECS::Entity forcePod = registry.spawnEntity();
                registry.emplaceComponent<shared::ForcePodComponent>(
                    forcePod, shared::ForcePodState::Attached, 40.0F, 0.0F,
                    playerNetId.networkId);
                registry.emplaceComponent<shared::ForcePodTag>(forcePod);
                registry.emplaceComponent<TransformComponent>(forcePod, 0.0F,
                                                             0.0F, 0.0F);
                registry.emplaceComponent<BoundingBoxComponent>(forcePod, 32.0F,
                                                               32.0F);
                
                if (_emitEvent) {
                    uint32_t forcePodNetId = playerNetId.networkId + 10000;
                    registry.emplaceComponent<NetworkIdComponent>(forcePod,
                                                                 forcePodNetId);
                    
                    engine::GameEvent evt{};
                    evt.type = engine::GameEventType::EntitySpawned;
                    evt.entityNetworkId = forcePodNetId;
                    evt.entityType = 5;
                    evt.x = 0.0F;
                    evt.y = 0.0F;
                    _emitEvent(evt);
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

    int32_t damage = 15;
    bool destroyObstacle = false;
    if (registry.hasComponent<DamageOnContactComponent>(obstacle)) {
        const auto& dmgComp =
            registry.getComponent<DamageOnContactComponent>(obstacle);
        damage = dmgComp.damage;
        destroyObstacle = dmgComp.destroySelf;
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
    } else {
        cmdBuffer.emplaceComponentDeferred<DestroyTag>(other, DestroyTag{});
    }

    if (destroyObstacle) {
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

}  // namespace rtype::games::rtype::server
