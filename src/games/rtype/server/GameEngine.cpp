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
#include "../shared/Components/PositionComponent.hpp"
#include "../shared/Components/Tags.hpp"
#include "../shared/Components/TransformComponent.hpp"
#include "../shared/Components/VelocityComponent.hpp"
#include "../shared/Systems/AISystem/Behaviors/BehaviorRegistry.hpp"
#include "Logger/Macros.hpp"
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
    auto eventEmitter = [this](const engine::GameEvent& event) {
        emitEvent(event);
    };
    SpawnerConfig spawnerConfig{};
    spawnerConfig.minSpawnInterval = GameConfig::MIN_SPAWN_INTERVAL;
    spawnerConfig.maxSpawnInterval = GameConfig::MAX_SPAWN_INTERVAL;
    spawnerConfig.maxEnemies = GameConfig::MAX_ENEMIES;
    spawnerConfig.spawnX = GameConfig::SCREEN_WIDTH + GameConfig::SPAWN_MARGIN;
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
        _spawnerSystem->decrementEnemyCount();
        _projectileSpawnerSystem->decrementProjectileCount();
    };
    _destroySystem =
        std::make_unique<DestroySystem>(eventEmitter, entityDecrementer);
    _running = true;
    return true;
}

void GameEngine::update(float deltaTime) {
    if (!_running) {
        return;
    }

    _spawnerSystem->update(*_registry, deltaTime);
    _projectileSpawnerSystem->update(*_registry, deltaTime);
    _enemyShootingSystem->update(*_registry, deltaTime);
    _aiSystem->update(*_registry, deltaTime);
    _movementSystem->update(*_registry, deltaTime);
    _lifetimeSystem->update(*_registry, deltaTime);
    _powerUpSystem->update(*_registry, deltaTime);
    _collisionSystem->update(*_registry, deltaTime);
    _cleanupSystem->update(*_registry, deltaTime);
    _destroySystem->update(*_registry, deltaTime);
}

void GameEngine::shutdown() {
    LOG_DEBUG("[GameEngine] Shutdown: Checking running state");
    if (!_running) {
        LOG_DEBUG("[GameEngine] Already shut down, returning");
        return;
    }
    _running = false;
    LOG_DEBUG("[GameEngine] Shutdown: Clearing system scheduler");
    if (_systemScheduler) {
        _systemScheduler->clear();
    }
    LOG_DEBUG("[GameEngine] Shutdown: Resetting spawner system");
    _spawnerSystem.reset();
    LOG_DEBUG("[GameEngine] Shutdown: Resetting projectile spawner system");
    _projectileSpawnerSystem.reset();
    LOG_DEBUG("[GameEngine] Shutdown: Resetting enemy shooting system");
    _enemyShootingSystem.reset();
    LOG_DEBUG("[GameEngine] Shutdown: Resetting AI system");
    _aiSystem.reset();
    LOG_DEBUG("[GameEngine] Shutdown: Resetting movement system");
    _movementSystem.reset();
    LOG_DEBUG("[GameEngine] Shutdown: Resetting lifetime system");
    _lifetimeSystem.reset();
    LOG_DEBUG("[GameEngine] Shutdown: Resetting power up system");
    _powerUpSystem.reset();
    LOG_DEBUG("[GameEngine] Shutdown: Resetting cleanup system");
    _cleanupSystem.reset();
    LOG_DEBUG("[GameEngine] Shutdown: Resetting destroy system");
    _destroySystem.reset();
    LOG_DEBUG("[GameEngine] Shutdown: Clearing pending events");
    {
        std::lock_guard<std::mutex> lock(_eventMutex);
        _pendingEvents.clear();
    }
    LOG_DEBUG("[GameEngine] Shutdown: Complete");
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
