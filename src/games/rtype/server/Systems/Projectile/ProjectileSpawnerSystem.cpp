/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ProjectileSpawnerSystem - Server projectile spawning implementation
*/

#include "ProjectileSpawnerSystem.hpp"

#include <cmath>
#include <numbers>
#include <utility>

#include <rtype/network/Protocol.hpp>

#include "../../../shared/Components.hpp"

namespace rtype::games::rtype::server {
using ::rtype::network::EntityType;
using shared::ActivePowerUpComponent;
using shared::BoundingBoxComponent;
using shared::EnemyProjectileTag;
using shared::LifetimeComponent;
using shared::NetworkIdComponent;
using shared::PlayerProjectileTag;
using shared::PlayerTag;
using shared::ProjectileComponent;
using shared::ProjectileOwner;
using shared::ProjectileTag;
using shared::ShootCooldownComponent;
using shared::TransformComponent;
using shared::VelocityComponent;
using shared::WeaponComponent;
using shared::WeaponConfig;
using shared::WeaponPresets::BasicBullet;
using shared::WeaponPresets::EnemyBullet;

ProjectileSpawnerSystem::ProjectileSpawnerSystem(EventEmitter emitter,
                                                 ProjectileSpawnConfig config)
    : ASystem("ProjectileSpawnerSystem"),
      _emitEvent(std::move(emitter)),
      _config(config) {
    std::random_device rd;
    _rng.seed(rd());
}

void ProjectileSpawnerSystem::update(ECS::Registry& registry, float deltaTime) {
    auto cooldownView = registry.view<ShootCooldownComponent>();
    cooldownView.each(
        [deltaTime](ECS::Entity /*entity*/, ShootCooldownComponent& cooldown) {
            cooldown.update(deltaTime);
        });
}

// LCOV_EXCL_START - lambda-based callback not easily testable
uint32_t ProjectileSpawnerSystem::spawnPlayerProjectile(
    ECS::Registry& registry, uint32_t playerNetworkId, float playerX,
    float playerY) {
    WeaponConfig weaponConfig = BasicBullet;

    float damageMultiplier = 1.0F;
    auto powerView = registry.view<NetworkIdComponent, shared::PlayerTag,
                                   shared::ActivePowerUpComponent>();
    powerView.each([playerNetworkId, &damageMultiplier](
                       ECS::Entity /*entity*/, const NetworkIdComponent& net,
                       const shared::PlayerTag& /*player*/,
                       const shared::ActivePowerUpComponent& active) {
        if (net.networkId == playerNetworkId) {
            damageMultiplier = active.damageMultiplier;
        }
    });

    weaponConfig.damage = static_cast<int32_t>(
        static_cast<float>(weaponConfig.damage) * damageMultiplier);

    float spawnX = playerX + _config.playerProjectileOffsetX;
    float spawnY = playerY + _config.playerProjectileOffsetY;

    if (weaponConfig.projectileCount > 1) {
        float totalSpread = weaponConfig.spreadAngle;
        float angleStep =
            totalSpread / static_cast<float>(weaponConfig.projectileCount - 1);
        float startAngle = -totalSpread / 2.0F;

        uint32_t firstId = 0;
        for (uint8_t i = 0; i < weaponConfig.projectileCount; ++i) {
            float angle = startAngle + angleStep * static_cast<float>(i);
            float radians = angle * std::numbers::pi_v<float> / 180.0F;

            float vx = weaponConfig.speed * std::cos(radians);
            float vy = weaponConfig.speed * std::sin(radians);

            uint32_t id = spawnProjectileWithConfig(
                registry, spawnX, spawnY, vx, vy, weaponConfig,
                ProjectileOwner::Player, playerNetworkId);
            if (i == 0) {
                firstId = id;
            }
        }
        return firstId;
    }

    return spawnProjectileWithConfig(registry, spawnX, spawnY,
                                     weaponConfig.speed, 0.0F, weaponConfig,
                                     ProjectileOwner::Player, playerNetworkId);
}
// LCOV_EXCL_STOP - lambda-based callback not easily testable

uint32_t ProjectileSpawnerSystem::spawnEnemyProjectile(
    ECS::Registry& registry, ECS::Entity enemyEntity, uint32_t enemyNetworkId,
    float enemyX, float enemyY, float targetX, float targetY) {
    float spawnX = enemyX + _config.enemyProjectileOffsetX;
    float spawnY = enemyY + _config.enemyProjectileOffsetY;
    float dx = targetX - spawnX;
    float dy = targetY - spawnY;
    float length = std::sqrt(dx * dx + dy * dy);
    WeaponConfig weaponConfig = EnemyBullet;
    float vx = (length > 0.0F) ? (dx / length) * weaponConfig.speed
                               : -weaponConfig.speed;
    float vy = (length > 0.0F) ? (dy / length) * weaponConfig.speed : 0.0F;

    return spawnProjectileWithConfig(registry, spawnX, spawnY, vx, vy,
                                     weaponConfig, ProjectileOwner::Enemy,
                                     enemyNetworkId);
}

uint32_t ProjectileSpawnerSystem::spawnProjectileWithConfig(
    ECS::Registry& registry, float x, float y, float vx, float vy,
    const WeaponConfig& config, ProjectileOwner owner,
    uint32_t ownerNetworkId) {
    ECS::Entity projectile = registry.spawnEntity();
    registry.emplaceComponent<TransformComponent>(projectile, x, y, 0.0F);
    registry.emplaceComponent<VelocityComponent>(projectile, vx, vy);
    registry.emplaceComponent<BoundingBoxComponent>(
        projectile, config.hitboxWidth, config.hitboxHeight);
    registry.emplaceComponent<LifetimeComponent>(projectile, config.lifetime);
    ProjectileComponent projComp;
    projComp.damage = config.damage;
    projComp.ownerNetworkId = ownerNetworkId;
    projComp.owner = owner;
    projComp.type = config.projectileType;
    projComp.piercing = config.piercing;
    projComp.maxHits = config.maxHits;
    projComp.currentHits = 0;
    registry.emplaceComponent<ProjectileComponent>(projectile, projComp);
    registry.emplaceComponent<ProjectileTag>(projectile);
    if (owner == ProjectileOwner::Player) {
        registry.emplaceComponent<PlayerProjectileTag>(projectile);
    } else {
        registry.emplaceComponent<EnemyProjectileTag>(projectile);
    }
    uint32_t networkId = _nextNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(projectile, networkId);
    _projectileCount++;
    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = networkId;
    event.x = x;
    event.y = y;
    event.rotation = 0.0F;
    event.entityType = static_cast<uint8_t>(EntityType::Missile);
    event.subType = static_cast<uint8_t>(config.projectileType);
    _emitEvent(event);

    return networkId;
}

}  // namespace rtype::games::rtype::server
