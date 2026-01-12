/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameEngine - Server-side game engine implementation
*/

#include "GameEngine.hpp"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include <rtype/network/Protocol.hpp>

#include "../shared/Components/EntityType.hpp"
#include "../shared/Components/NetworkIdComponent.hpp"
#include "../shared/Components/Tags.hpp"
#include "../shared/Components/TransformComponent.hpp"
#include "../shared/Components/VelocityComponent.hpp"
#include "../shared/Config/EntityConfig/EntityConfig.hpp"
#include "../shared/Config/PrefabLoader.hpp"
#include "../shared/Systems/AISystem/Behaviors/BehaviorRegistry.hpp"
#include "Logger/Macros.hpp"
#include "Systems/Spawner/DataDrivenSpawnerSystem.hpp"
namespace rtype::games::rtype::server {

GameEngine::GameEngine(std::shared_ptr<ECS::Registry> registry)
    : _registry(std::move(registry)),
      _systemScheduler(std::make_unique<ECS::SystemScheduler>(*_registry)) {}

GameEngine::~GameEngine() {
    if (_running) {
        shutdown();
    }
}

bool GameEngine::initialize() {
    if (_running) {
        return false;
    }
    _registry->reserveEntities(GameConfig::MAX_ENEMIES + 100);

    LOG_INFO("[GameEngine] Loading entity configurations");
    auto& entityConfigRegistry = shared::EntityConfigRegistry::getInstance();
    try {
        entityConfigRegistry.loadEnemiesWithSearch("config/game/enemies.toml");
        entityConfigRegistry.loadPlayersWithSearch("config/game/players.toml");
        entityConfigRegistry.loadProjectilesWithSearch(
            "config/game/projectiles.toml");
        entityConfigRegistry.loadPowerUpsWithSearch(
            "config/game/powerups.toml");
        if (entityConfigRegistry.loadFromDirectory("config/game")) {
            LOG_INFO(
                "[GameEngine] Level configurations loaded from config/game");
        } else {
            LOG_WARNING(
                "[GameEngine] Failed to load level configurations - "
                "data-driven spawning may not work correctly");
        }
        LOG_INFO("[GameEngine] Entity configurations loaded");
    } catch (const std::exception& e) {
        LOG_WARNING("[GameEngine] Failed to load some entity configurations: "
                    << e.what() << " - Continuing with available configs");
    }

    _prefabManager = std::make_unique<ECS::PrefabManager>(std::ref(*_registry));
    shared::PrefabLoader::registerAllPrefabs(*_prefabManager);
    LOG_INFO("[GameEngine] Registered "
             << _prefabManager->getPrefabNames().size()
             << " prefabs from entity configs");

    setupECSSignals();
    auto eventEmitter = [this](const engine::GameEvent& event) {
        emitEvent(event);
    };

    DataDrivenSpawnerConfig ddConfig{};
    ddConfig.screenWidth = GameConfig::SCREEN_WIDTH;
    ddConfig.screenHeight = GameConfig::SCREEN_HEIGHT;
    ddConfig.spawnMargin = GameConfig::SPAWN_MARGIN;
    ddConfig.maxEnemies = GameConfig::MAX_ENEMIES;
    ddConfig.waveTransitionDelay = 2.0F;
    ddConfig.waitForClear = true;
    ddConfig.enableFallbackSpawning = true;
    ddConfig.fallbackMinInterval = GameConfig::MIN_SPAWN_INTERVAL;
    ddConfig.fallbackMaxInterval = GameConfig::MAX_SPAWN_INTERVAL;
    ddConfig.fallbackEnemiesPerWave = 10;
    ddConfig.powerUpMinInterval = 9999.0F;
    ddConfig.powerUpMaxInterval = 9999.0F;
    _dataDrivenSpawnerSystem =
        std::make_unique<DataDrivenSpawnerSystem>(eventEmitter, ddConfig);

    if (_dataDrivenSpawnerSystem->loadLevel("level_1")) {
        LOG_INFO(
            "[GameEngine] Level 'level_1' loaded for data-driven spawning");
        _dataDrivenSpawnerSystem->startLevel();
    } else {
        LOG_WARNING(
            "[GameEngine] Could not load level_1 - using fallback spawning");
    }

    SpawnerConfig spawnerConfig{};
    spawnerConfig.minSpawnInterval = GameConfig::MIN_SPAWN_INTERVAL;
    spawnerConfig.maxSpawnInterval = GameConfig::MAX_SPAWN_INTERVAL;
    spawnerConfig.maxEnemies = GameConfig::MAX_ENEMIES;
    spawnerConfig.spawnX = GameConfig::SCREEN_WIDTH + GameConfig::SPAWN_OFFSET;
    spawnerConfig.minSpawnY = GameConfig::SPAWN_MARGIN;
    spawnerConfig.maxSpawnY =
        GameConfig::SCREEN_HEIGHT - GameConfig::SPAWN_MARGIN;
    spawnerConfig.bydosSlaveSpeed = GameConfig::BYDOS_SLAVE_SPEED;
    spawnerConfig.weightMoveLeft = 0.2F;
    spawnerConfig.weightSineWave = 0.1F;
    spawnerConfig.weightZigZag = 0.3F;
    spawnerConfig.weightDiveBomb = 1.0F;
    spawnerConfig.weightStationary = 1.2F;
    spawnerConfig.weightChase = 1.5F;
    spawnerConfig.stationarySpawnInset = GameConfig::STATIONARY_SPAWN_INSET;
    spawnerConfig.maxWaves = 1;
    spawnerConfig.enemiesPerWave = 5;
    _spawnerSystem =
        std::make_unique<SpawnerSystem>(eventEmitter, spawnerConfig);
    ProjectileSpawnConfig projConfig{};
    _projectileSpawnerSystem =
        std::make_unique<ProjectileSpawnerSystem>(eventEmitter, projConfig);

    auto enemyShootCb = [this](ECS::Registry& reg, ECS::Entity enemy,
                               uint32_t enemyNetId, float ex, float ey,
                               float tx, float ty) {
        if (_projectileSpawnerSystem) {
            return _projectileSpawnerSystem->spawnEnemyProjectile(
                reg, enemy, enemyNetId, ex, ey, tx, ty);
        }
        return 0U;
    };
    _enemyShootingSystem =
        std::make_unique<EnemyShootingSystem>(std::move(enemyShootCb));

    shared::registerDefaultBehaviors();
    _aiSystem = std::make_unique<shared::AISystem>();
    _movementSystem = std::make_unique<shared::MovementSystem>();
    _lifetimeSystem = std::make_unique<shared::LifetimeSystem>();
    _powerUpSystem = std::make_unique<shared::PowerUpSystem>();
    _collisionSystem = std::make_unique<CollisionSystem>(
        eventEmitter, GameConfig::SCREEN_WIDTH, GameConfig::SCREEN_HEIGHT);
    CleanupConfig cleanupConfig{};
    cleanupConfig.leftBoundary = GameConfig::CLEANUP_LEFT;
    cleanupConfig.rightBoundary = GameConfig::CLEANUP_RIGHT;
    cleanupConfig.topBoundary = GameConfig::CLEANUP_TOP;
    cleanupConfig.bottomBoundary = GameConfig::CLEANUP_BOTTOM;
    _cleanupSystem =
        std::make_unique<CleanupSystem>(eventEmitter, cleanupConfig);
    auto entityDecrementer = [this]() {
        if (_useDataDrivenSpawner && _dataDrivenSpawnerSystem) {
            _dataDrivenSpawnerSystem->decrementEnemyCount();
        } else if (_spawnerSystem) {
            _spawnerSystem->decrementEnemyCount();
        }
        _projectileSpawnerSystem->decrementProjectileCount();
    };
    _destroySystem =
        std::make_unique<DestroySystem>(eventEmitter, entityDecrementer);
    _forcePodAttachmentSystem = std::make_unique<ForcePodAttachmentSystem>();
    _forcePodLaunchSystem = std::make_unique<ForcePodLaunchSystem>();
    _forcePodShootingSystem = std::make_unique<ForcePodShootingSystem>(
        _projectileSpawnerSystem.get());

    _forcePodAttachmentSystem->setLaunchSystem(_forcePodLaunchSystem.get());

    _systemScheduler->addSystem("Spawner", [this](ECS::Registry& reg) {
        if (_useDataDrivenSpawner && _dataDrivenSpawnerSystem) {
            _dataDrivenSpawnerSystem->update(reg, _lastDeltaTime);
        } else if (_spawnerSystem) {
            _spawnerSystem->update(reg, _lastDeltaTime);
        }
    });
    _systemScheduler->addSystem(
        "ProjectileSpawner", [this](ECS::Registry& reg) {
            _projectileSpawnerSystem->update(reg, _lastDeltaTime);
        });
    _systemScheduler->addSystem("EnemyShooting",
                                [this](ECS::Registry& reg) {
                                    _enemyShootingSystem->update(
                                        reg, _lastDeltaTime);
                                },
                                {"Spawner"});
    _systemScheduler->addSystem(
        "AI",
        [this](ECS::Registry& reg) { _aiSystem->update(reg, _lastDeltaTime); },
        {"EnemyShooting"});
    _systemScheduler->addSystem("Movement",
                                [this](ECS::Registry& reg) {
                                    _movementSystem->update(reg,
                                                            _lastDeltaTime);
                                },
                                {"AI"});
    _systemScheduler->addSystem("Lifetime", [this](ECS::Registry& reg) {
        _lifetimeSystem->update(reg, _lastDeltaTime);
    });
    _systemScheduler->addSystem("PowerUp", [this](ECS::Registry& reg) {
        _powerUpSystem->update(reg, _lastDeltaTime);
    });
    _systemScheduler->addSystem("ForcePodAttachment",
                                [this](ECS::Registry& reg) {
                                    _forcePodAttachmentSystem->update(
                                        reg, _lastDeltaTime);
                                },
                                {"Movement"});
    _systemScheduler->addSystem("ForcePodLaunch",
                                [this](ECS::Registry& reg) {
                                    _forcePodLaunchSystem->update(
                                        reg, _lastDeltaTime);
                                },
                                {"ForcePodAttachment"});
    _systemScheduler->addSystem("ForcePodShooting",
                                [this](ECS::Registry& reg) {
                                    _forcePodShootingSystem->update(
                                        reg, _lastDeltaTime);
                                },
                                {"ForcePodLaunch"});
    _systemScheduler->addSystem("Collision",
                                [this](ECS::Registry& reg) {
                                    _collisionSystem->update(reg,
                                                             _lastDeltaTime);
                                },
                                {"Movement"});
    _systemScheduler->addSystem("Cleanup",
                                [this](ECS::Registry& reg) {
                                    _cleanupSystem->update(reg, _lastDeltaTime);
                                },
                                {"Collision"});
    _systemScheduler->addSystem(
        "Destroy",
        [this](ECS::Registry& reg) {
            _destroySystem->update(reg, _lastDeltaTime);
        },
        {"Cleanup", "Collision", "Lifetime", "PowerUp"});

    _running = true;
    return true;
}

void GameEngine::update(float deltaTime) {
    if (!_running) {
        return;
    }
    _lastDeltaTime = deltaTime;
    _systemScheduler->run();
}

void GameEngine::shutdown() {
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[GameEngine] Shutdown: Checking running state");
    if (!_running) {
        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[GameEngine] Already shut down, returning");
        return;
    }
    _running = false;
    if (_systemScheduler) {
        _systemScheduler->clear();
    }
    _spawnerSystem.reset();
    _dataDrivenSpawnerSystem.reset();
    _projectileSpawnerSystem.reset();
    _enemyShootingSystem.reset();
    _aiSystem.reset();
    _movementSystem.reset();
    _lifetimeSystem.reset();
    _powerUpSystem.reset();
    _cleanupSystem.reset();
    _destroySystem.reset();
    {
        std::lock_guard<std::mutex> lock(_eventMutex);
        _pendingEvents.clear();
    }
    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[GameEngine] Shutdown: Complete");
}

bool GameEngine::loadLevel(const std::string& levelId) {
    if (_dataDrivenSpawnerSystem) {
        return _dataDrivenSpawnerSystem->loadLevel(levelId);
    }
    LOG_ERROR(
        "[GameEngine] Cannot load level: DataDrivenSpawnerSystem not "
        "initialized");
    return false;
}

bool GameEngine::loadLevelFromFile(const std::string& filepath) {
    if (_dataDrivenSpawnerSystem) {
        return _dataDrivenSpawnerSystem->loadLevelFromFile(filepath);
    }
    LOG_ERROR(
        "[GameEngine] Cannot load level: DataDrivenSpawnerSystem not "
        "initialized");
    return false;
}

void GameEngine::startLevel() {
    if (_dataDrivenSpawnerSystem) {
        _dataDrivenSpawnerSystem->startLevel();
    }
}

void GameEngine::setEventCallback(EventCallback callback) {
    std::lock_guard<std::mutex> lock(_eventMutex);
    _eventCallback = std::move(callback);
}

std::vector<engine::GameEvent> GameEngine::getPendingEvents() {
    std::lock_guard<std::mutex> lock(_eventMutex);
    return _pendingEvents;
}

void GameEngine::clearPendingEvents() {
    std::lock_guard<std::mutex> lock(_eventMutex);
    _pendingEvents.clear();
}

std::size_t GameEngine::getEntityCount() const {
    if (_useDataDrivenSpawner && _dataDrivenSpawnerSystem) {
        return _dataDrivenSpawnerSystem->getEnemyCount();
    }
    if (_spawnerSystem) {
        return _spawnerSystem->getEnemyCount();
    }
    return 0;
}

bool GameEngine::isRunning() const { return _running; }

engine::ProcessedEvent GameEngine::processEvent(
    const engine::GameEvent& event) {
    engine::ProcessedEvent result{};
    result.type = event.type;
    result.networkId = event.entityNetworkId;
    result.subType = event.subType;
    result.x = event.x;
    result.y = event.y;
    result.vx = event.velocityX;
    result.vy = event.velocityY;
    result.duration = event.duration;
    result.valid = false;

    switch (event.type) {
        case engine::GameEventType::EntitySpawned: {
            ECS::Entity foundEntity{0};
            bool found = false;

            auto view = _registry->view<shared::NetworkIdComponent>();
            view.each([&](ECS::Entity entity,
                          const shared::NetworkIdComponent& netId) {
                if (netId.networkId == event.entityNetworkId) {
                    foundEntity = entity;
                    found = true;
                }
            });

            if (!found) {
                return result;
            }
            auto protoType =
                static_cast<::rtype::network::EntityType>(event.entityType);

            switch (protoType) {
                case ::rtype::network::EntityType::Player:
                case ::rtype::network::EntityType::Bydos:
                case ::rtype::network::EntityType::Missile:
                case ::rtype::network::EntityType::Pickup:
                case ::rtype::network::EntityType::Obstacle:
                case ::rtype::network::EntityType::ForcePod:
                    result.networkEntityType = event.entityType;
                    break;
                default:
                    result.networkEntityType = static_cast<uint8_t>(
                        ::rtype::network::EntityType::Bydos);
                    break;
            }
            result.valid = true;
            break;
        }
        case engine::GameEventType::EntityDestroyed:
        case engine::GameEventType::EntityUpdated:
        case engine::GameEventType::EntityHealthChanged:
        case engine::GameEventType::PowerUpApplied:
            result.valid = true;
            break;
        case engine::GameEventType::GameOver:
            result.valid = false;
            break;
    }

    return result;
}

// LCOV_EXCL_START - lambda-based callback not easily testable
void GameEngine::syncEntityPositions(
    std::function<void(uint32_t, float, float, float, float)> callback) {
    if (!callback) {
        return;
    }

    auto view =
        _registry
            ->view<shared::TransformComponent, shared::NetworkIdComponent>();
    view.each([this, &callback](ECS::Entity entity,
                                const shared::TransformComponent& transform,
                                const shared::NetworkIdComponent& netId) {
        float vx = 0.0F;
        float vy = 0.0F;
        if (_registry->hasComponent<shared::VelocityComponent>(entity)) {
            const auto& vel =
                _registry->getComponent<shared::VelocityComponent>(entity);
            vx = vel.vx;
            vy = vel.vy;
        }
        callback(netId.networkId, transform.x, transform.y, vx, vy);
    });
}
// LCOV_EXCL_STOP

uint32_t GameEngine::spawnProjectile(uint32_t playerNetworkId, float playerX,
                                     float playerY) {
    if (!_running || !_projectileSpawnerSystem) {
        return 0;
    }
    return _projectileSpawnerSystem->spawnPlayerProjectile(
        *_registry, playerNetworkId, playerX, playerY);
}

void GameEngine::emitEvent(const engine::GameEvent& event) {
    EventCallback callbackCopy;
    {
        std::lock_guard<std::mutex> lock(_eventMutex);
        _pendingEvents.push_back(event);
        callbackCopy = _eventCallback;
    }
    if (callbackCopy) {
        callbackCopy(event);
    }
}

void GameEngine::setupECSSignals() {
    using shared::EnemyTag;
    using shared::NetworkIdComponent;
    using shared::PlayerTag;
    using shared::ProjectileTag;

    _registry->onConstruct<NetworkIdComponent>([this](ECS::Entity entity) {
        _totalEntitiesCreated.fetch_add(1, std::memory_order_relaxed);
        if (_registry->hasComponent<NetworkIdComponent>(entity)) {
            const auto& netId =
                _registry->getComponent<NetworkIdComponent>(entity);
            LOG_DEBUG("[GameEngine] Entity created: ID="
                      << entity.id << ", NetworkID=" << netId.networkId);
        }
    });

    _registry->onDestroy<NetworkIdComponent>([this](ECS::Entity entity) {
        _totalEntitiesDestroyed.fetch_add(1, std::memory_order_relaxed);
        LOG_DEBUG("[GameEngine] Entity with NetworkIdComponent destroyed: ID="
                  << entity.id);
    });
    _registry->onConstruct<EnemyTag>([](ECS::Entity entity) {
        LOG_DEBUG("[GameEngine] Enemy spawned: EntityID=" << entity.id);
    });
    _registry->onDestroy<EnemyTag>([](ECS::Entity entity) {
        LOG_DEBUG("[GameEngine] Enemy destroyed: EntityID=" << entity.id);
    });
    _registry->onConstruct<ProjectileTag>([](ECS::Entity entity) {
        LOG_DEBUG("[GameEngine] Projectile spawned: EntityID=" << entity.id);
    });
    _registry->onDestroy<ProjectileTag>([](ECS::Entity entity) {
        LOG_DEBUG("[GameEngine] Projectile destroyed: EntityID=" << entity.id);
    });

    LOG_INFO(
        "[GameEngine] ECS signals configured for entity lifecycle tracking");
}

void registerRTypeGameEngine() {
    static bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    engine::GameEngineFactory::registerGame(
        "rtype", [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<GameEngine>(std::move(registry));
        });
    engine::GameEngineFactory::setDefaultGame("rtype");
}

namespace {
struct RTypeAutoRegistrar {
    RTypeAutoRegistrar() { registerRTypeGameEngine(); }
} rtypeAutoRegistrar;
}  // namespace

}  // namespace rtype::games::rtype::server
