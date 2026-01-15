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
constexpr int32_t KBOSS_HEALTH_THRESHOLD = 500;

namespace {
shared::AttackPatternConfig createPatternFromType(
    shared::BossAttackPattern pattern) {
    switch (pattern) {
        case shared::BossAttackPattern::CircularShot:
            return shared::AttackPatternConfig::createCircularShot();
        case shared::BossAttackPattern::SpreadFan:
            return shared::AttackPatternConfig::createSpreadFan();
        case shared::BossAttackPattern::LaserSweep:
            return shared::AttackPatternConfig::createLaserSweep();
        case shared::BossAttackPattern::MinionSpawn:
            return shared::AttackPatternConfig::createMinionSpawn();
        case shared::BossAttackPattern::TailSweep:
            return shared::AttackPatternConfig::createTailSweep();
        default:
            return shared::AttackPatternConfig{};
    }
}
}  // namespace

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
    _waveManager.setStartDelay(config.startDelay);

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

        auto powerupSpawns = _waveManager.getPowerUpSpawns(deltaTime);
        for (const auto& powerupSpawn : powerupSpawns) {
            spawnPowerUpFromConfig(registry, powerupSpawn);
        }

        if (_waveManager.isAllWavesComplete()) {
            auto bossId = _waveManager.getBossId();
            if (bossId.has_value() && !_bossSpawned) {
                if (aliveEnemies == 0) {
                    spawnBoss(registry, *bossId);
                    _bossSpawned = true;
                }
                return;
            }
            bool noBossRequired = !bossId.has_value();
            bool bossDefeated = _bossSpawned;
            bool allEnemiesDefeated = aliveEnemies == 0;

            if (allEnemiesDefeated && (noBossRequired || bossDefeated)) {
                LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                             "[DataDrivenSpawner] Level complete!");
                _gameOverEmitted = true;
                engine::GameEvent event{};
                event.type = engine::GameEventType::LevelComplete;
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
    DamageOnContactComponent enemyDmg{};
    enemyDmg.damage = enemyConfig.damage;
    enemyDmg.destroySelf = true;
    registry.emplaceComponent<DamageOnContactComponent>(enemy, enemyDmg);

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

    auto& configRegistry = shared::EntityConfigRegistry::getInstance();
    auto enemyConfigOpt = configRegistry.getEnemy(bossId);
    if (!enemyConfigOpt.has_value()) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[DataDrivenSpawner] Boss config not found: " << bossId);
        return;
    }

    const auto& bossConfig = enemyConfigOpt.value().get();
    ECS::Entity boss = registry.spawnEntity();

    float spawnX = _config.screenWidth - 200.0F;
    float spawnY = _config.screenHeight / 2.0F;

    registry.emplaceComponent<TransformComponent>(boss, spawnX, spawnY, 0.0F);
    registry.emplaceComponent<VelocityComponent>(boss, 0.0F, 0.0F);
    registry.emplaceComponent<HealthComponent>(boss, bossConfig.health,
                                               bossConfig.health);
    registry.emplaceComponent<BoundingBoxComponent>(
        boss, bossConfig.hitboxWidth, bossConfig.hitboxHeight);
    registry.emplaceComponent<DamageOnContactComponent>(boss, bossConfig.damage,
                                                        true);

    if (bossConfig.canShoot) {
        float shootCooldown =
            (bossConfig.fireRate > 0) ? (1.0F / bossConfig.fireRate) : 0.3F;
        registry.emplaceComponent<shared::ShootCooldownComponent>(
            boss, shootCooldown);
    }

    uint32_t networkId = _nextNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(boss, networkId);
    registry.emplaceComponent<EnemyTag>(boss);
    registry.emplaceComponent<BydosSlaveTag>(boss);

    auto variant = EnemyTypeComponent::stringToVariant(bossId);
    registry.emplaceComponent<EnemyTypeComponent>(boss, variant, bossId);

    shared::BossComponent bossComp;
    bossComp.bossId = bossId;
    bossComp.bossType = shared::stringToBossType(bossConfig.bossType);
    bossComp.phaseTransitionDuration = bossConfig.phaseTransitionDuration;
    bossComp.scoreValue = bossConfig.scoreValue;
    bossComp.levelCompleteTrigger = bossConfig.levelCompleteTrigger;
    bossComp.baseX = spawnX;
    bossComp.baseY = spawnY;

    const auto& movementConfig = bossConfig.animationConfig.movement;
    bossComp.amplitude = movementConfig.amplitude;
    bossComp.frequency = movementConfig.frequency;

    for (const auto& phaseConfig : bossConfig.phases) {
        shared::BossPhase phase;
        phase.healthThreshold = phaseConfig.healthThreshold;
        phase.phaseName = phaseConfig.name;
        phase.primaryPattern =
            shared::stringToBossAttackPattern(phaseConfig.primaryPattern);
        phase.secondaryPattern =
            shared::stringToBossAttackPattern(phaseConfig.secondaryPattern);
        phase.speedMultiplier = phaseConfig.speedMultiplier;
        phase.attackSpeedMultiplier = phaseConfig.attackSpeedMultiplier;
        phase.damageMultiplier = phaseConfig.damageMultiplier;
        phase.colorR = phaseConfig.colorR;
        phase.colorG = phaseConfig.colorG;
        phase.colorB = phaseConfig.colorB;
        bossComp.phases.push_back(std::move(phase));
    }

    registry.emplaceComponent<shared::BossComponent>(boss, std::move(bossComp));
    registry.emplaceComponent<shared::BossTag>(boss);

    shared::BossPatternComponent patternComp;
    patternComp.enabled = true;
    patternComp.cyclical = true;
    if (!bossConfig.phases.empty()) {
        const auto& firstPhase = bossConfig.phases[0];
        auto primaryPattern =
            shared::stringToBossAttackPattern(firstPhase.primaryPattern);
        if (primaryPattern != shared::BossAttackPattern::None) {
            patternComp.patternQueue.push_back(
                createPatternFromType(primaryPattern));
        }
        auto secondaryPattern =
            shared::stringToBossAttackPattern(firstPhase.secondaryPattern);
        if (secondaryPattern != shared::BossAttackPattern::None) {
            patternComp.patternQueue.push_back(
                createPatternFromType(secondaryPattern));
        }
    }
    registry.emplaceComponent<shared::BossPatternComponent>(
        boss, std::move(patternComp));

    for (const auto& wpConfig : bossConfig.weakPoints) {
        ECS::Entity weakPoint = registry.spawnEntity();
        float wpX = spawnX + wpConfig.offsetX;
        float wpY = spawnY + wpConfig.offsetY;

        registry.emplaceComponent<TransformComponent>(weakPoint, wpX, wpY,
                                                      0.0F);
        registry.emplaceComponent<VelocityComponent>(weakPoint, 0.0F, 0.0F);
        registry.emplaceComponent<HealthComponent>(weakPoint, wpConfig.health,
                                                   wpConfig.health);
        registry.emplaceComponent<BoundingBoxComponent>(
            weakPoint, wpConfig.hitboxWidth, wpConfig.hitboxHeight);

        shared::WeakPointComponent wpComp;
        wpComp.parentBossEntity = boss;
        wpComp.parentBossNetworkId = networkId;
        wpComp.weakPointId = wpConfig.id;
        wpComp.type = shared::stringToWeakPointType(wpConfig.type);
        wpComp.localOffsetX = wpConfig.offsetX;
        wpComp.localOffsetY = wpConfig.offsetY;
        wpComp.bonusScore = wpConfig.bonusScore;
        wpComp.damageToParent = wpConfig.damageToParent;
        wpComp.critical = wpConfig.critical;
        wpComp.segmentIndex = wpConfig.segmentIndex;
        if (!wpConfig.disablesAttack.empty()) {
            wpComp.disablesBossAttack = true;
            wpComp.disabledAttackPattern = wpConfig.disablesAttack;
        }
        registry.emplaceComponent<shared::WeakPointComponent>(
            weakPoint, std::move(wpComp));
        registry.emplaceComponent<shared::WeakPointTag>(weakPoint);
        registry.emplaceComponent<shared::EnemyTag>(
            weakPoint);  // For collision detection

        uint32_t wpNetworkId = _nextNetworkId++;
        registry.emplaceComponent<NetworkIdComponent>(weakPoint, wpNetworkId);

        engine::GameEvent wpEvent{};
        wpEvent.type = engine::GameEventType::EntitySpawned;
        wpEvent.entityNetworkId = wpNetworkId;
        wpEvent.x = wpX;
        wpEvent.y = wpY;
        wpEvent.rotation = 0.0F;
        wpEvent.entityType = static_cast<uint8_t>(EntityType::BossPart);
        if (wpComp.segmentIndex >= 0) {
            wpEvent.subType = static_cast<uint8_t>(wpComp.segmentIndex);
        } else {
            wpEvent.subType =
                static_cast<uint8_t>(100 + std::abs(wpComp.segmentIndex));
        }
        wpEvent.parentNetworkId = networkId;
        _emitEvent(wpEvent);

        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[DataDrivenSpawner] Spawned weak point '"
                          << wpConfig.id << "' for boss " << bossId);
    }

    _enemyCount++;

    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = networkId;
    event.x = spawnX;
    event.y = spawnY;
    event.rotation = 0.0F;
    event.entityType = static_cast<uint8_t>(EntityType::Boss);
    event.subType = static_cast<uint8_t>(bossComp.bossType);
    _emitEvent(event);

    engine::GameEvent bossEvent{};
    bossEvent.type = engine::GameEventType::BossPhaseChanged;
    bossEvent.entityNetworkId = networkId;
    bossEvent.bossPhase = 0;
    bossEvent.bossPhaseCount = static_cast<uint8_t>(bossConfig.phases.size());
    _emitEvent(bossEvent);

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[DataDrivenSpawner] Boss '"
                     << bossId << "' spawned with " << bossConfig.phases.size()
                     << " phases and " << bossConfig.weakPoints.size()
                     << " weak points");
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
                    if (!isBossEnemy(id, config)) {
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

bool DataDrivenSpawnerSystem::isBossEnemy(
    const std::string& enemyId, const shared::EnemyConfig& config) const {
    auto& configRegistry = shared::EntityConfigRegistry::getInstance();
    const auto& allLevels = configRegistry.getAllLevels();
    for (const auto& [levelId, levelConfig] : allLevels) {
        if (levelConfig.bossId.has_value() && *levelConfig.bossId == enemyId) {
            return true;
        }
    }
    if (config.health >= KBOSS_HEALTH_THRESHOLD) {
        return true;
    }
    return false;
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
    DamageOnContactComponent obstacleDmg{};
    obstacleDmg.damage = _config.obstacleDamage;
    obstacleDmg.destroySelf = true;
    registry.emplaceComponent<DamageOnContactComponent>(obstacle, obstacleDmg);
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
        case shared::PowerUpType::ForcePod:
            duration = 0.0F;
            magnitude = 1.0F;
            variant = shared::PowerUpVariant::ForcePod;
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
    registry.emplaceComponent<shared::PickupTag>(pickup);

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

void DataDrivenSpawnerSystem::spawnPowerUpFromConfig(
    ECS::Registry& registry, const PowerUpSpawnRequest& request) {
    auto& configRegistry = shared::EntityConfigRegistry::getInstance();
    auto powerupConfigOpt = configRegistry.getPowerUp(request.powerUpId);

    if (!powerupConfigOpt.has_value()) {
        LOG_WARNING_CAT(
            ::rtype::LogCategory::GameEngine,
            "[DataDrivenSpawner] Unknown powerup type: " << request.powerUpId);
        return;
    }

    const auto& powerupConfig = powerupConfigOpt.value().get();

    float spawnX =
        request.hasFixedX() ? *request.x : _config.screenWidth + 30.0F;
    float spawnY = request.hasFixedY() ? *request.y : _spawnYDist(_rng);

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[DataDrivenSpawner] Spawning powerup '"
                     << request.powerUpId << "' at (" << spawnX << ", "
                     << spawnY << ")");

    ECS::Entity pickup = registry.spawnEntity();

    registry.emplaceComponent<TransformComponent>(pickup, spawnX, spawnY, 0.0F);
    registry.emplaceComponent<VelocityComponent>(pickup, -_config.powerUpSpeed,
                                                 0.0F);
    registry.emplaceComponent<BoundingBoxComponent>(
        pickup, powerupConfig.hitboxWidth, powerupConfig.hitboxHeight);

    shared::PowerUpVariant variant =
        PowerUpTypeComponent::stringToVariant(powerupConfig.id);

    shared::PowerUpType type = shared::PowerUpType::HealthBoost;
    float duration = powerupConfig.duration;
    float magnitude = static_cast<float>(powerupConfig.value) / 100.0F;

    switch (powerupConfig.effect) {
        case shared::PowerUpConfig::EffectType::Health:
            type = shared::PowerUpType::HealthBoost;
            magnitude = static_cast<float>(powerupConfig.value) / 100.0F;
            break;
        case shared::PowerUpConfig::EffectType::SpeedBoost:
            type = shared::PowerUpType::SpeedBoost;
            break;
        case shared::PowerUpConfig::EffectType::WeaponUpgrade:
            if (powerupConfig.id == "force_pod") {
                type = shared::PowerUpType::ForcePod;
                duration = 0.0F;
                magnitude = 1.0F;
            } else if (powerupConfig.id == "laser_upgrade") {
                type = shared::PowerUpType::LaserUpgrade;
                duration = 0.0F;
                magnitude = 1.0F;
            } else {
                type = shared::PowerUpType::RapidFire;
            }
            break;
        case shared::PowerUpConfig::EffectType::Shield:
            type = shared::PowerUpType::Shield;
            break;
        case shared::PowerUpConfig::EffectType::HealthBoost:
            type = shared::PowerUpType::HealthBoost;
            magnitude = static_cast<float>(powerupConfig.value) / 100.0F;
            break;
    }

    shared::PowerUpComponent power{};
    power.type = type;
    power.duration = duration;
    power.magnitude = magnitude;
    registry.emplaceComponent<shared::PowerUpComponent>(pickup, power);
    registry.emplaceComponent<PowerUpTypeComponent>(pickup, variant);
    registry.emplaceComponent<shared::PickupTag>(pickup);

    uint32_t networkId = _nextNetworkId++;
    registry.emplaceComponent<NetworkIdComponent>(pickup, networkId);

    engine::GameEvent event{};
    event.type = engine::GameEventType::EntitySpawned;
    event.entityNetworkId = networkId;
    event.x = spawnX;
    event.y = spawnY;
    event.entityType = static_cast<uint8_t>(EntityType::Pickup);
    event.subType = static_cast<uint8_t>(variant);
    _emitEvent(event);
}

}  // namespace rtype::games::rtype::server
