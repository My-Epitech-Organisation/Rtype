/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** GameEngine - Server-side game engine implementation
*/

#include "GameEngine.hpp"

#include "../shared/Config/EntityConfig/EntityConfig.hpp"
#include "../shared/Systems/AISystem/Behaviors/BehaviorRegistry.hpp"

namespace rtype::games::rtype::server {

// Default scroll state for when no level is loaded
static shared::LevelScrollState s_defaultScrollState;

GameEngine::GameEngine() : _systemScheduler(_registry) {}

GameEngine::~GameEngine() {
    if (_running) {
        shutdown();
    }
}

bool GameEngine::initialize() {
    if (_running) {
        return false;
    }
    _registry.reserveEntities(GameConfig::MAX_ENEMIES + 100);
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
    shared::registerDefaultBehaviors();
    _aiSystem = std::make_unique<shared::AISystem>();
    _movementSystem = std::make_unique<shared::MovementSystem>();
    CleanupConfig cleanupConfig{};
    cleanupConfig.leftBoundary = GameConfig::CLEANUP_LEFT;
    cleanupConfig.rightBoundary = GameConfig::CLEANUP_RIGHT;
    cleanupConfig.topBoundary = GameConfig::CLEANUP_TOP;
    cleanupConfig.bottomBoundary = GameConfig::CLEANUP_BOTTOM;
    _cleanupSystem =
        std::make_unique<CleanupSystem>(eventEmitter, cleanupConfig);
    auto enemyDecrementer = [this]() { _spawnerSystem->decrementEnemyCount(); };
    _destroySystem =
        std::make_unique<DestroySystem>(eventEmitter, enemyDecrementer);

    // Initialize level/map systems
    _levelLoader = std::make_unique<shared::LevelLoader>();
    _mapScrollingSystem = std::make_unique<shared::MapScrollingSystem>();

    _running = true;
    return true;
}

void GameEngine::update(float deltaTime) {
    if (!_running) {
        return;
    }

    // Update level scrolling and map element spawning
    if (_levelLoader && _levelLoader->isLoaded()) {
        _levelLoader->update(_registry, deltaTime);
        _mapScrollingSystem->update(_registry, deltaTime);
    }

    _spawnerSystem->update(_registry, deltaTime);
    _aiSystem->update(_registry, deltaTime);
    _movementSystem->update(_registry, deltaTime);
    _cleanupSystem->update(_registry, deltaTime);
    _destroySystem->update(_registry, deltaTime);
}

void GameEngine::shutdown() {
    if (!_running) {
        return;
    }
    _running = false;
    _systemScheduler.clear();
    _spawnerSystem.reset();
    _aiSystem.reset();
    _movementSystem.reset();
    _cleanupSystem.reset();
    _destroySystem.reset();
    _levelLoader.reset();
    _mapScrollingSystem.reset();
    {
        std::lock_guard<std::mutex> lock(_eventMutex);
        _pendingEvents.clear();
    }
}

bool GameEngine::loadLevel(const std::string& levelId) {
    if (!_levelLoader) {
        return false;
    }

    // Reset existing level if any
    if (_levelLoader->isLoaded()) {
        _levelLoader->reset(_registry);
    }

    // Load the new level
    if (!_levelLoader->loadLevel(levelId)) {
        return false;
    }

    // Connect scroll state to the scrolling system
    _mapScrollingSystem->setScrollState(&_levelLoader->getScrollState());

    return true;
}

const shared::LevelScrollState& GameEngine::getScrollState() const {
    if (_levelLoader && _levelLoader->isLoaded()) {
        return _levelLoader->getScrollState();
    }
    return s_defaultScrollState;
}

void GameEngine::setLevelPaused(bool paused) {
    if (_levelLoader) {
        _levelLoader->setPaused(paused);
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

void GameEngine::emitEvent(const engine::GameEvent& event) {
    std::lock_guard<std::mutex> lock(_eventMutex);
    _pendingEvents.push_back(event);
    if (_eventCallback) {
        _eventCallback(event);
    }
}

}  // namespace rtype::games::rtype::server

namespace rtype::engine {

std::unique_ptr<IGameEngine> createGameEngine() {
    return std::make_unique<games::rtype::server::GameEngine>();
}

}  // namespace rtype::engine
