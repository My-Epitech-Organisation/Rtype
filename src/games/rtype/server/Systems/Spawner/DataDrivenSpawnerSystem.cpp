/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DataDrivenSpawnerSystem - Implementation
*/

#define NOMINMAX
#include "DataDrivenSpawnerSystem.hpp"

#include <algorithm>
#include <vector>

#include <rtype/network/Protocol.hpp>

#include "../../../shared/Components.hpp"
#include "../../../shared/Config/EntityConfig/EntityConfig.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

using ::rtype::network::EntityType;
using shared::AIBehavior;
using shared::AIComponent;
using shared::BoundingBoxComponent;
using shared::BydosSlaveTag;
using shared::DamageOnContactComponent;
using shared::EnemyTag;
using shared::EnemyTypeComponent;
using shared::HealthComponent;
using shared::NetworkIdComponent;
using shared::ObstacleTag;
using shared::PowerUpComponent;
using shared::PowerUpTypeComponent;
using shared::PowerUpVariant;
using shared::TransformComponent;
using shared::VelocityComponent;

DataDrivenSpawnerSystem::DataDrivenSpawnerSystem(EventEmitter emitter,
                                                 DataDrivenSpawnerConfig config)
    : ASystem("DataDrivenSpawnerSystem"),
      _emitEvent(std::move(emitter)),
      _config(config),
      _rng(getRandomSeed()),
      _spawnYDist(config.spawnMargin, config.screenHeight - config.spawnMargin),
      _obstacleSpawnTimeDist(config.obstacleMinInterval,
                             config.obstacleMaxInterval),
      _powerUpSpawnTimeDist(config.powerUpMinInterval,
                            config.powerUpMaxInterval),
      _powerUpTypeDist(1, static_cast<int>(shared::PowerUpType::HealthBoost)) {
    _waveManager.setWaitForClear(config.waitForClear);
    _waveManager.setWaveTransitionDelay(config.waveTransitionDelay);

    generateNextObstacleSpawnTime();
    generateNextPowerUpSpawnTime();
}

bool DataDrivenSpawnerSystem::loadLevel(const std::string& levelId) {
    bool result = _waveManager.loadLevel(levelId);
    if (result) {
        _levelStarted = false;
        _bossSpawned = false;
        _gameOverEmitted = false;
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[DataDrivenSpawner] Level '" << levelId << "' loaded");
    }
    return result;
}

bool DataDrivenSpawnerSystem::loadLevelFromFile(const std::string& filepath) {
    bool result = _waveManager.loadLevelFromFile(filepath);
    if (result) {
        _levelStarted = false;
        _bossSpawned = false;
        _gameOverEmitted = false;
        LOG_INFO_CAT(
            ::rtype::LogCategory::GameEngine,
            "[DataDrivenSpawner] Level loaded from '" << filepath << "'");
    }
    return result;
}

void DataDrivenSpawnerSystem::startLevel() {
    if (_waveManager.isLevelLoaded()) {
        _waveManager.start();
        _levelStarted = true;
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[DataDrivenSpawner] Level started");
    } else {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[DataDrivenSpawner] No level loaded, using fallback");
    }
}

void DataDrivenSpawnerSystem::reset() {
    _waveManager.reset();
    _enemyCount = 0;
    _gameOverEmitted = false;
    _bossSpawned = false;
    _levelStarted = false;
    _fallbackCurrentWave = 1;
    _fallbackEnemiesThisWave = 0;
    _fallbackSpawnTimer = 0.0F;
}

void DataDrivenSpawnerSystem::update(ECS::Registry& registry, float deltaTime) {
    if (_gameOverEmitted) {
        return;
    }
    std::size_t aliveEnemies = 0;
    auto enemyView = registry.view<shared::EnemyTag>();
    enemyView.each([&aliveEnemies](ECS::Entity, const shared::EnemyTag&) {
        aliveEnemies++;
    });
    
    // Only spawn obstacles and power-ups if a level is loaded or fallback is enabled
    if (_waveManager.isLevelLoaded() || _config.enableFallbackSpawning) {
        _obstacleSpawnTimer += deltaTime;
        _powerUpSpawnTimer += deltaTime;

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
    
    if (_waveManager.isLevelLoaded() && _levelStarted) {
        auto spawns = _waveManager.update(deltaTime, aliveEnemies);
        for (const auto& spawn : spawns) {
            if (_enemyCount < _config.maxEnemies) {
                spawnEnemy(registry, spawn);
            }
        }

        if (_waveManager.isAllWavesComplete()) {
            auto bossId = _waveManager.getBossId();
            if (bossId.has_value() && !_bossSpawned) {
                if (aliveEnemies == 0) {
                    spawnBoss(registry, *bossId);
                    _bossSpawned = true;
                }
            } else if (aliveEnemies == 0 &&
                       (!bossId.has_value() || _bossSpawned)) {
                LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                             "[DataDrivenSpawner] Level complete!");
                _gameOverEmitted = true;
                engine::GameEvent event{};
                event.type = engine::GameEventType::GameOver;
                _emitEvent(event);
            }
        }
    } else if (_config.enableFallbackSpawning) {
        updateFallbackSpawning(registry, deltaTime);
    }
}

void DataDrivenSpawnerSystem::spawnEnemy(ECS::Registry& registry,
                                         const SpawnRequest& request) {
    auto& configRegistry = shared::EntityConfigRegistry::getInstance();
    auto enemyConfigOpt = configRegistry.getEnemy(request.enemyId);

    if (!enemyConfigOpt.has_value()) {
        LOG_WARNING_CAT(
            ::rtype::LogCategory::GameEngine,
            "[DataDrivenSpawner] Unknown enemy type: " << request.enemyId);
        return;
    }

    const auto& enemyConfig = enemyConfigOpt.value().get();

    float spawnX = request.hasFixedX() ? *request.x : _config.screenWidth;
    float spawnY = request.hasFixedY() ? *request.y : _spawnYDist(_rng);

    ECS::Entity enemy = registry.spawnEntity();

    registry.emplaceComponent<TransformComponent>(enemy, spawnX, spawnY, 0.0F);

    float speedX = 0.0F;
    if (enemyConfig.behavior == AIBehavior::MoveLeft ||
        enemyConfig.behavior == AIBehavior::Stationary) {
        speedX = -enemyConfig.speed;
    }
    registry.emplaceComponent<VelocityComponent>(enemy, speedX, 0.0F);

    AIComponent ai{};
    ai.behavior = enemyConfig.behavior;
    ai.speed = enemyConfig.speed;

    switch (enemyConfig.behavior) {
        case AIBehavior::Chase:
            ai.targetX = 0.0F;
            ai.targetY = 0.0F;
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
    registry.emplaceComponent<HealthComponent>(enemy, enemyConfig.health,
                                               enemyConfig.health);
    registry.emplaceComponent<BoundingBoxComponent>(
        enemy, enemyConfig.hitboxWidth, enemyConfig.hitboxHeight);
    registry.emplaceComponent<DamageOnContactComponent>(
        enemy, enemyConfig.damage, true);

    if (enemyConfig.canShoot) {
        float shootCooldown =
            (enemyConfig.fireRate > 0) ? (1.0F / enemyConfig.fireRate) : 0.3F;
        registry.emplaceComponent<shared::ShootCooldownComponent>(
            enemy, shootCooldown);
    }

    uint32_t networkId = _nextNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(enemy, networkId);
    registry.emplaceComponent<EnemyTag>(enemy);
    registry.emplaceComponent<BydosSlaveTag>(enemy);

    auto variant = EnemyTypeComponent::stringToVariant(request.enemyId);
    registry.emplaceComponent<EnemyTypeComponent>(enemy, variant,
                                                  request.enemyId);

    _enemyCount++;

    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = networkId;
    event.x = spawnX;
    event.y = spawnY;
    event.rotation = 0.0F;
    event.entityType = static_cast<uint8_t>(EntityType::Bydos);
    event.subType = static_cast<uint8_t>(variant);
    _emitEvent(event);

    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[DataDrivenSpawner] Spawned enemy '"
                      << request.enemyId << "' at (" << spawnX << ", " << spawnY
                      << ")");
}

void DataDrivenSpawnerSystem::spawnBoss(ECS::Registry& registry,
                                        const std::string& bossId) {
    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[DataDrivenSpawner] Spawning boss: " << bossId);

    SpawnRequest bossRequest;
    bossRequest.enemyId = bossId;
    bossRequest.x = _config.screenWidth - 200.0F;
    bossRequest.y = _config.screenHeight / 2.0F;
    bossRequest.count = 1;

    spawnEnemy(registry, bossRequest);
}

void DataDrivenSpawnerSystem::updateFallbackSpawning(ECS::Registry& registry,
                                                     float deltaTime) {
    std::size_t aliveEnemies = 0;
    auto enemyView = registry.view<shared::EnemyTag>();
    enemyView.each([&aliveEnemies](ECS::Entity, const shared::EnemyTag&) {
        aliveEnemies++;
    });

    if (_fallbackEnemiesThisWave >= _config.fallbackEnemiesPerWave &&
        aliveEnemies == 0) {
        _fallbackCurrentWave++;
        _fallbackEnemiesThisWave = 0;
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[DataDrivenSpawner] Fallback wave "
                         << _fallbackCurrentWave << " starting");
    }

    _fallbackSpawnTimer += deltaTime;
    if (_fallbackSpawnTimer >= _nextFallbackSpawnTime) {
        if (_enemyCount < _config.maxEnemies &&
            _fallbackEnemiesThisWave < _config.fallbackEnemiesPerWave) {
            auto& configRegistry = shared::EntityConfigRegistry::getInstance();
            const auto& allEnemies = configRegistry.getAllEnemies();

            if (!allEnemies.empty()) {
                std::vector<std::string> enemyPool;
                for (const auto& [id, config] : allEnemies) {
                    if (id.find("boss") == std::string::npos) {
                        enemyPool.push_back(id);
                    }
                }

                if (!enemyPool.empty()) {
                    std::uniform_int_distribution<size_t> dist(
                        0, enemyPool.size() - 1);
                    const std::string& selectedId = enemyPool[dist(_rng)];

                    SpawnRequest request;
                    request.enemyId = selectedId;
                    request.x = _config.screenWidth + 30.0F;
                    request.y = _spawnYDist(_rng);
                    request.count = 1;

                    spawnEnemy(registry, request);
                    _fallbackEnemiesThisWave++;
                }
            }
        }

        _fallbackSpawnTimer = 0.0F;
        std::uniform_real_distribution<float> spawnTimeDist(
            _config.fallbackMinInterval, _config.fallbackMaxInterval);
        _nextFallbackSpawnTime = spawnTimeDist(_rng);
    }
}

void DataDrivenSpawnerSystem::generateNextObstacleSpawnTime() {
    _nextObstacleSpawnTime = _obstacleSpawnTimeDist(_rng);
}

void DataDrivenSpawnerSystem::generateNextPowerUpSpawnTime() {
    _nextPowerUpSpawnTime = _powerUpSpawnTimeDist(_rng);
}

void DataDrivenSpawnerSystem::spawnObstacle(ECS::Registry& registry) {
    ECS::Entity obstacle = registry.spawnEntity();
    float spawnY = _spawnYDist(_rng);

    registry.emplaceComponent<TransformComponent>(
        obstacle, _config.screenWidth + 30.0F, spawnY, 0.0F);
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
    event.x = _config.screenWidth + 30.0F;
    event.y = spawnY;
    event.entityType = static_cast<uint8_t>(EntityType::Obstacle);
    _emitEvent(event);
}

void DataDrivenSpawnerSystem::spawnPowerUp(ECS::Registry& registry) {
    ECS::Entity pickup = registry.spawnEntity();
    float spawnY = _spawnYDist(_rng);

    registry.emplaceComponent<TransformComponent>(
        pickup, _config.screenWidth + 30.0F, spawnY, 0.0F);
    registry.emplaceComponent<VelocityComponent>(pickup, -_config.powerUpSpeed,
                                                 0.0F);
    registry.emplaceComponent<BoundingBoxComponent>(pickup, 24.0F, 24.0F);

    int powerUpTypeInt = _powerUpTypeDist(_rng);
    auto powerUpType = static_cast<shared::PowerUpType>(powerUpTypeInt);

    float duration = 8.0F;
    float magnitude = 0.5F;
    shared::PowerUpVariant variant = shared::PowerUpVariant::Unknown;

    switch (powerUpType) {
        case shared::PowerUpType::SpeedBoost:
            duration = 5.0F;
            variant = shared::PowerUpVariant::SpeedBoost;
            break;
        case shared::PowerUpType::Shield:
            duration = 8.0F;
            variant = shared::PowerUpVariant::Shield;
            break;
        case shared::PowerUpType::RapidFire:
            duration = 10.0F;
            variant = shared::PowerUpVariant::RapidFire;
            break;
        case shared::PowerUpType::DoubleDamage:
            duration = 10.0F;
            variant = shared::PowerUpVariant::DoubleDamage;
            break;
        case shared::PowerUpType::HealthBoost:
            magnitude = 50.0F;
            variant = shared::PowerUpVariant::HealthBoost;
            break;
        default:
            variant = shared::PowerUpVariant::HealthBoost;
            break;
    }

    shared::PowerUpComponent power{};
    power.type = powerUpType;
    power.duration = duration;
    power.magnitude = magnitude;
    registry.emplaceComponent<shared::PowerUpComponent>(pickup, power);
    registry.emplaceComponent<PowerUpTypeComponent>(pickup, variant);

    uint32_t networkId = _nextNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(pickup, networkId);

    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = networkId;
    event.x = _config.screenWidth + 30.0F;
    event.y = spawnY;
    event.entityType = static_cast<uint8_t>(EntityType::Pickup);
    event.subType = static_cast<uint8_t>(variant);
    _emitEvent(event);
}

}  // namespace rtype::games::rtype::server
