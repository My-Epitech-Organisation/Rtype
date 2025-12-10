/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameEngine - Server-side game engine implementation
*/

#include "GameEngine.hpp"

#include <memory>
#include <utility>
#include <vector>

#include "../shared/Components/EntityType.hpp"
#include "../shared/Components/NetworkIdComponent.hpp"
#include "../shared/Systems/AISystem/Behaviors/BehaviorRegistry.hpp"

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
    _spawnerSystem =
        std::make_unique<SpawnerSystem>(eventEmitter, spawnerConfig);
    ProjectileSpawnConfig projConfig{};
    _projectileSpawnerSystem =
        std::make_unique<ProjectileSpawnerSystem>(eventEmitter, projConfig);

    shared::registerDefaultBehaviors();
    _aiSystem = std::make_unique<shared::AISystem>();
    _movementSystem = std::make_unique<shared::MovementSystem>();
    _lifetimeSystem = std::make_unique<shared::LifetimeSystem>();
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
    _aiSystem->update(*_registry, deltaTime);
    _movementSystem->update(*_registry, deltaTime);
    _lifetimeSystem->update(*_registry, deltaTime);
    _collisionSystem->update(*_registry, deltaTime);
    _cleanupSystem->update(*_registry, deltaTime);
    _destroySystem->update(*_registry, deltaTime);
}

void GameEngine::shutdown() {
    if (!_running) {
        return;
    }
    _running = false;
    if (_systemScheduler) {
        _systemScheduler->clear();
    }
    _spawnerSystem.reset();
    _projectileSpawnerSystem.reset();
    _aiSystem.reset();
    _movementSystem.reset();
    _lifetimeSystem.reset();
    _cleanupSystem.reset();
    _destroySystem.reset();
    {
        std::lock_guard<std::mutex> lock(_eventMutex);
        _pendingEvents.clear();
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
            auto gameType = static_cast<shared::EntityType>(event.entityType);
            switch (gameType) {
                case shared::EntityType::Player:
                    result.networkEntityType = 0;  // Player
                    break;
                case shared::EntityType::Enemy:
                    result.networkEntityType = 1;  // Bydos
                    break;
                case shared::EntityType::Projectile:
                    result.networkEntityType = 2;  // Missile
                    break;
                default:
                    result.networkEntityType = 1;  // Default to Bydos
                    break;
            }
            result.valid = true;
            break;
        }
        case engine::GameEventType::EntityDestroyed:
        case engine::GameEventType::EntityUpdated:
            result.valid = true;
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

uint32_t GameEngine::spawnPlayerProjectile(uint32_t playerNetworkId,
                                           float playerX, float playerY) {
    if (!_running || !_projectileSpawnerSystem) {
        return 0;
    }
    return _projectileSpawnerSystem->spawnPlayerProjectile(
        *_registry, playerNetworkId, playerX, playerY);
}

void GameEngine::emitEvent(const engine::GameEvent& event) {
    std::lock_guard<std::mutex> lock(_eventMutex);
    _pendingEvents.push_back(event);
    if (_eventCallback) {
        _eventCallback(event);
    }
}

}  // namespace rtype::games::rtype::server

namespace rtype::engine {

std::unique_ptr<IGameEngine> createGameEngine(
    std::shared_ptr<ECS::Registry> registry) {
    return std::make_unique<games::rtype::server::GameEngine>(
        std::move(registry));
}

}  // namespace rtype::engine
