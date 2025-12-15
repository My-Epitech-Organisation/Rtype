/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SpawnerSystem - Server enemy spawning implementation
*/

#define NOMINMAX
#include "SpawnerSystem.hpp"

#include <algorithm>
#include <vector>

#include <rtype/network/Protocol.hpp>

#include "../../../shared/Components.hpp"

namespace rtype::games::rtype::server {

using ::rtype::network::EntityType;
using shared::AIBehavior;
using shared::AIComponent;
using shared::BoundingBoxComponent;
using shared::BydosSlaveTag;
using shared::DamageOnContactComponent;
using shared::EnemyTag;
using shared::HealthComponent;
using shared::NetworkIdComponent;
using shared::ObstacleTag;
using shared::PowerUpComponent;
using shared::TransformComponent;
using shared::VelocityComponent;
static constexpr float BYDOS_SLAVE_SIZE = 32.0F;
static constexpr int BYDOS_SLAVE_HEALTH = 10;

SpawnerSystem::SpawnerSystem(EventEmitter emitter, SpawnerConfig config)
    : ASystem("SpawnerSystem"),
      _emitEvent(std::move(emitter)),
      _config(config),
      _rng(getRandomSeed()),
      _spawnTimeDist(config.minSpawnInterval, config.maxSpawnInterval),
      _spawnYDist(config.minSpawnY, config.maxSpawnY),
      _obstacleSpawnTimeDist(config.obstacleMinInterval,
                             config.obstacleMaxInterval),
      _powerUpSpawnTimeDist(config.powerUpMinInterval,
                            config.powerUpMaxInterval),
      _powerUpTypeDist(1, static_cast<int>(shared::PowerUpType::HealthBoost)) {
    generateNextSpawnTime();
    generateNextObstacleSpawnTime();
    generateNextPowerUpSpawnTime();
}

void SpawnerSystem::update(ECS::Registry& registry, float deltaTime) {
    _spawnTimer += deltaTime;
    _obstacleSpawnTimer += deltaTime;
    _powerUpSpawnTimer += deltaTime;

    if (_spawnTimer >= _nextSpawnTime && _enemyCount < _config.maxEnemies) {
        spawnBydosSlave(registry);
        _spawnTimer = 0.0F;
        generateNextSpawnTime();
    }

    if (_obstacleSpawnTimer >= _nextObstacleSpawnTime) {
        spawnObstacle(registry);
        _obstacleSpawnTimer = 0.0F;
        generateNextObstacleSpawnTime();
    }

    if (_powerUpSpawnTimer >= _nextPowerUpSpawnTime) {
        spawnPowerUp(registry);
        _powerUpSpawnTimer = 0.0F;
        generateNextPowerUpSpawnTime();
    }
}

void SpawnerSystem::spawnBydosSlave(ECS::Registry& registry) {
    ECS::Entity enemy = registry.spawnEntity();
    float spawnY = _spawnYDist(_rng);
    const std::vector<std::pair<AIBehavior, float>> behaviors = {
        {AIBehavior::MoveLeft, _config.weightMoveLeft},
        {AIBehavior::SineWave, _config.weightSineWave},
        {AIBehavior::ZigZag, _config.weightZigZag},
        {AIBehavior::DiveBomb, _config.weightDiveBomb},
        {AIBehavior::Stationary, _config.weightStationary},
        {AIBehavior::Chase, _config.weightChase}};

    float totalWeight = 0.0F;
    for (const auto& b : behaviors) {
        totalWeight += b.second;
    }
    std::uniform_real_distribution<float> weightDist(0.0F, totalWeight);
    float pick = weightDist(_rng);
    AIBehavior chosenBehavior = AIBehavior::MoveLeft;
    float accumulator = 0.0F;
    for (const auto& [behavior, weight] : behaviors) {
        accumulator += weight;
        if (pick <= accumulator) {
            chosenBehavior = behavior;
            break;
        }
    }

    float spawnX = _config.spawnX;
    if (chosenBehavior == AIBehavior::Stationary) {
        spawnX = std::max(0.0F, _config.spawnX - _config.stationarySpawnInset);
    }

    registry.emplaceComponent<TransformComponent>(enemy, spawnX, spawnY, 0.0F);
    registry.emplaceComponent<VelocityComponent>(
        enemy, -_config.bydosSlaveSpeed, 0.0F);
    AIComponent ai{};
    ai.behavior = chosenBehavior;
    ai.speed = _config.bydosSlaveSpeed;

    switch (chosenBehavior) {
        case AIBehavior::Chase:
            ai.targetX = 0.0F;
            ai.targetY = _spawnYDist(_rng);
            break;
        case AIBehavior::DiveBomb:
            ai.targetY = _spawnYDist(_rng);
            break;
        case AIBehavior::ZigZag:
            ai.targetY = 1.0F;
            break;
        case AIBehavior::Stationary:
            ai.targetX = spawnX;
            ai.targetY = spawnY;
            break;
        default:
            ai.targetY = spawnY;
            break;
    }

    registry.emplaceComponent<AIComponent>(enemy, ai);
    registry.emplaceComponent<HealthComponent>(enemy, BYDOS_SLAVE_HEALTH,
                                               BYDOS_SLAVE_HEALTH);
    registry.emplaceComponent<BoundingBoxComponent>(enemy, BYDOS_SLAVE_SIZE,
                                                    BYDOS_SLAVE_SIZE);
    registry.emplaceComponent<shared::ShootCooldownComponent>(
        enemy, shared::WeaponPresets::EnemyBullet.cooldown);
    uint32_t networkId = _nextNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(enemy, networkId);
    registry.emplaceComponent<EnemyTag>(enemy);
    registry.emplaceComponent<BydosSlaveTag>(enemy);

    _enemyCount++;

    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = networkId;
    event.x = spawnX;
    event.y = spawnY;
    event.rotation = 0.0F;
    event.entityType = static_cast<uint8_t>(EntityType::Bydos);
    _emitEvent(event);
}

void SpawnerSystem::generateNextSpawnTime() {
    _nextSpawnTime = _spawnTimeDist(_rng);
}

void SpawnerSystem::generateNextObstacleSpawnTime() {
    _nextObstacleSpawnTime = _obstacleSpawnTimeDist(_rng);
}

void SpawnerSystem::generateNextPowerUpSpawnTime() {
    _nextPowerUpSpawnTime = _powerUpSpawnTimeDist(_rng);
}

void SpawnerSystem::spawnObstacle(ECS::Registry& registry) {
    ECS::Entity obstacle = registry.spawnEntity();
    float spawnY = _spawnYDist(_rng);

    registry.emplaceComponent<TransformComponent>(obstacle, _config.spawnX,
                                                  spawnY, 0.0F);
    registry.emplaceComponent<VelocityComponent>(obstacle,
                                                 -_config.obstacleSpeed, 0.0F);
    registry.emplaceComponent<BoundingBoxComponent>(
        obstacle, _config.obstacleWidth, _config.obstacleHeight);
    registry.emplaceComponent<DamageOnContactComponent>(
        obstacle, _config.obstacleDamage, true);
    registry.emplaceComponent<shared::ObstacleTag>(obstacle);

    uint32_t networkId = _nextNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(obstacle, networkId);
    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = networkId;
    event.x = _config.spawnX;
    event.y = spawnY;
    event.entityType = static_cast<uint8_t>(EntityType::Obstacle);
    _emitEvent(event);
}

void SpawnerSystem::spawnPowerUp(ECS::Registry& registry) {
    ECS::Entity pickup = registry.spawnEntity();
    float spawnY = _spawnYDist(_rng);

    registry.emplaceComponent<TransformComponent>(pickup, _config.spawnX,
                                                  spawnY, 0.0F);
    registry.emplaceComponent<VelocityComponent>(pickup, -_config.powerUpSpeed,
                                                 0.0F);
    registry.emplaceComponent<BoundingBoxComponent>(pickup, 24.0F, 24.0F);
    registry.emplaceComponent<shared::PickupTag>(pickup);

    shared::PowerUpComponent power{};
    power.type = static_cast<shared::PowerUpType>(_powerUpTypeDist(_rng));
    power.duration = 8.0F;
    power.magnitude = 0.5F;
    registry.emplaceComponent<shared::PowerUpComponent>(pickup, power);

    uint32_t networkId = _nextNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(pickup, networkId);
    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = networkId;
    event.x = _config.spawnX;
    event.y = spawnY;
    event.entityType = static_cast<uint8_t>(EntityType::Pickup);
    _emitEvent(event);
}

}  // namespace rtype::games::rtype::server
