/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** WaveManager - Implementation
*/

#include "WaveManager.hpp"

#include <algorithm>
#include <filesystem>

#include "../../../shared/Config/EntityConfig/EntityConfig.hpp"
#include "Logger/Macros.hpp"

namespace rtype::games::rtype::server {

WaveManager::WaveManager() = default;

bool WaveManager::loadLevel(const std::string& levelId) {
    auto& configRegistry = shared::EntityConfigRegistry::getInstance();
    auto levelOpt = configRegistry.getLevel(levelId);

    if (!levelOpt.has_value()) {
        _lastError = "Level not found in registry: " + levelId;
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[WaveManager] " << _lastError);
        _state = WaveState::Failed;
        return false;
    }

    _levelConfig = levelOpt.value().get();

    if (!_levelConfig->isValid()) {
        _lastError = "Invalid level configuration: " + levelId;
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[WaveManager] " << _lastError);
        _levelConfig.reset();
        _state = WaveState::Failed;
        return false;
    }

    _state = WaveState::NotStarted;
    _currentWaveIndex = 0;
    _pendingSpawns.clear();

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[WaveManager] Loaded level '"
                     << _levelConfig->name << "' with "
                     << _levelConfig->waves.size() << " waves");

    return true;
}

bool WaveManager::loadLevelFromFile(const std::string& filepath) {
    auto& configRegistry = shared::EntityConfigRegistry::getInstance();

    namespace fs = std::filesystem;
    const std::vector<std::string> searchPaths = {
        filepath, "../" + filepath, "../../" + filepath, "../../../" + filepath,
        "config/game/levels/" + filepath};

    std::string foundPath;
    for (const auto& path : searchPaths) {
        if (fs::exists(path)) {
            foundPath = path;
            break;
        }
    }

    if (foundPath.empty()) {
        _lastError = "Level file not found: " + filepath;
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[WaveManager] " << _lastError);
        _state = WaveState::Failed;
        return false;
    }

    if (!configRegistry.loadLevel(foundPath)) {
        _lastError = "Failed to parse level file: " + foundPath;
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[WaveManager] " << _lastError);
        _state = WaveState::Failed;
        return false;
    }

    fs::path p(foundPath);
    std::string levelId = p.stem().string();

    return loadLevel(levelId);
}

void WaveManager::start() {
    if (!_levelConfig) {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[WaveManager] Cannot start: no level loaded");
        return;
    }

    if (_levelConfig->waves.empty()) {
        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                        "[WaveManager] Cannot start: level has no waves");
        _state = WaveState::AllComplete;
        return;
    }

    _currentWaveIndex = 0;
    _waveTimer = 0.0F;
    _transitionTimer = 0.0F;
    _state = WaveState::InProgress;

    prepareCurrentWave();

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[WaveManager] Starting level '"
                     << _levelConfig->name << "' - Wave 1/"
                     << _levelConfig->waves.size());
}

void WaveManager::reset() {
    _currentWaveIndex = 0;
    _waveTimer = 0.0F;
    _transitionTimer = 0.0F;
    _pendingSpawns.clear();
    _pendingPowerUps.clear();

    if (_levelConfig) {
        _state = WaveState::NotStarted;
    } else {
        _state = WaveState::Failed;
    }
}

std::vector<SpawnRequest> WaveManager::update(float deltaTime,
                                              std::size_t aliveEnemyCount) {
    std::vector<SpawnRequest> spawns;

    if (_state == WaveState::NotStarted || _state == WaveState::AllComplete ||
        _state == WaveState::Failed) {
        return spawns;
    }

    if (_state == WaveState::WaveComplete) {
        if (_waitForClear && aliveEnemyCount > 0) {
            return spawns;
        }

        _transitionTimer += deltaTime;
        if (_transitionTimer >= _waveTransitionDelay) {
            advanceToNextWave();
        }
        return spawns;
    }

    _waveTimer += deltaTime;

    bool allSpawnsComplete = true;
    for (auto& pending : _pendingSpawns) {
        if (pending.remainingCount <= 0) {
            continue;
        }

        allSpawnsComplete = false;

        if (!pending.started) {
            if (_waveTimer >= pending.entry.delay) {
                pending.started = true;
                pending.remainingDelay = 0.0F;
            }
        }

        if (pending.started) {
            pending.remainingDelay -= deltaTime;
            if (pending.remainingDelay <= 0.0F) {
                SpawnRequest request;
                request.enemyId = pending.entry.enemyId;
                request.x = pending.entry.x;
                request.y = pending.entry.y;
                request.count = 1;
                spawns.push_back(request);
                pending.remainingCount--;
                if (pending.remainingCount > 0 && _levelConfig) {
                    const auto& wave = _levelConfig->waves[_currentWaveIndex];
                    pending.remainingDelay = wave.spawnDelay;
                }
            }
        }
    }

    if (allSpawnsComplete) {
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[WaveManager] Wave " << (_currentWaveIndex + 1) << "/"
                                           << _levelConfig->waves.size()
                                           << " spawn complete");
        _state = WaveState::WaveComplete;
        _transitionTimer = 0.0F;
    }

    return spawns;
}

void WaveManager::advanceToNextWave() {
    _currentWaveIndex++;

    if (_currentWaveIndex >= _levelConfig->waves.size()) {
        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[WaveManager] All waves completed!");
        _state = WaveState::AllComplete;
        return;
    }

    _state = WaveState::InProgress;
    _waveTimer = 0.0F;
    prepareCurrentWave();

    LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                 "[WaveManager] Starting wave " << (_currentWaveIndex + 1)
                                                << "/"
                                                << _levelConfig->waves.size());
}

void WaveManager::prepareCurrentWave() {
    _pendingSpawns.clear();

    if (!_levelConfig) {
        return;
    }

    if (_currentWaveIndex >= _levelConfig->waves.size()) {
        return;
    }

    const auto& wave = _levelConfig->waves[_currentWaveIndex];

    for (const auto& spawnEntry : wave.spawns) {
        auto& configRegistry = shared::EntityConfigRegistry::getInstance();
        if (!configRegistry.getEnemy(spawnEntry.enemyId).has_value()) {
            LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                            "[WaveManager] Unknown enemy type: "
                                << spawnEntry.enemyId << " - skipping");
            continue;
        }

        PendingSpawn pending;
        pending.entry = spawnEntry;
        pending.remainingCount = spawnEntry.count;
        pending.remainingDelay = 0.0F;
        pending.started = false;
        _pendingSpawns.push_back(pending);
    }

    for (const auto& powerupEntry : wave.powerups) {
        auto& configRegistry = shared::EntityConfigRegistry::getInstance();
        if (!configRegistry.getPowerUp(powerupEntry.powerUpId).has_value()) {
            LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                            "[WaveManager] Unknown powerup type: "
                                << powerupEntry.powerUpId << " - skipping");
            continue;
        }

        PendingPowerUp pending;
        pending.entry = powerupEntry;
        pending.remainingDelay = powerupEntry.delay;
        pending.spawned = false;
        _pendingPowerUps.push_back(pending);
    }

    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[WaveManager] Prepared wave "
                      << (_currentWaveIndex + 1) << " with "
                      << _pendingSpawns.size() << " spawn entries and "
                      << _pendingPowerUps.size() << " powerups");
}

std::vector<PowerUpSpawnRequest> WaveManager::getPowerUpSpawns(
    float deltaTime) {
    std::vector<PowerUpSpawnRequest> spawns;
    if (_state != WaveState::InProgress && _state != WaveState::WaveComplete) {
        return spawns;
    }

    for (auto& pending : _pendingPowerUps) {
        if (pending.spawned) {
            continue;
        }

        pending.remainingDelay -= deltaTime;

        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[WaveManager] PowerUp '"
                          << pending.entry.powerUpId
                          << "' delay: " << pending.remainingDelay);

        if (pending.remainingDelay <= 0.0F) {
            LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                         "[WaveManager] PowerUp '" << pending.entry.powerUpId
                                                   << "' ready to spawn!");
            PowerUpSpawnRequest request;
            request.powerUpId = pending.entry.powerUpId;
            request.x = pending.entry.x;
            request.y = pending.entry.y;
            spawns.push_back(request);
            pending.spawned = true;
        }
    }

    return spawns;
}

}  // namespace rtype::games::rtype::server
