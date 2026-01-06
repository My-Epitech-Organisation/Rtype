/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DataDrivenSpawnerSystem - Data-driven enemy spawning using WaveManager
*/

#pragma once

#include <functional>
#include <memory>
#include <random>

#include <rtype/engine.hpp>

#include "../WaveManager/WaveManager.hpp"

namespace rtype::games::rtype::server {

/**
 * @struct DataDrivenSpawnerConfig
 * @brief Configuration for the data-driven spawner
 */
struct DataDrivenSpawnerConfig {
    float screenWidth = 1920.0F;
    float screenHeight = 1080.0F;
    float spawnMargin = 50.0F;
    std::size_t maxEnemies = 100;
    float waveTransitionDelay = 2.0F;
    bool waitForClear = true;

    // Fallback random spawning when no level is loaded
    bool enableFallbackSpawning = true;
    float fallbackMinInterval = 2.0F;
    float fallbackMaxInterval = 4.0F;
    std::size_t fallbackEnemiesPerWave = 10;

    // Obstacle spawning
    float obstacleMinInterval = 3.0F;
    float obstacleMaxInterval = 6.0F;
    float obstacleSpeed = 80.0F;
    float obstacleWidth = 64.0F;
    float obstacleHeight = 64.0F;
    int32_t obstacleDamage = 20;

    // Power-up spawning
    float powerUpMinInterval = 8.0F;
    float powerUpMaxInterval = 14.0F;
    float powerUpSpeed = 70.0F;
};

/**
 * @class DataDrivenSpawnerSystem
 * @brief Server-only system that handles data-driven enemy spawning
 *
 * This system replaces random enemy spawning with configuration-based
 * wave spawning. It uses the WaveManager to read level configurations
 * and spawn enemies according to the defined waves.
 *
 * Features:
 * - Data-driven wave spawning from TOML config files
 * - Fallback to random spawning when no level is loaded
 * - Boss spawning support
 * - Obstacle and power-up spawning
 * - Thread-safe event emission
 */
class DataDrivenSpawnerSystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    /**
     * @brief Construct with event emitter and configuration
     * @param emitter Function to emit game events
     * @param config Spawner configuration
     */
    DataDrivenSpawnerSystem(EventEmitter emitter,
                            DataDrivenSpawnerConfig config);

    void update(ECS::Registry& registry, float deltaTime) override;

    /**
     * @brief Load a level for spawning
     * @param levelId Level identifier
     * @return true if level loaded successfully
     */
    bool loadLevel(const std::string& levelId);

    /**
     * @brief Load a level from file path
     * @param filepath Path to level TOML file
     * @return true if level loaded successfully
     */
    bool loadLevelFromFile(const std::string& filepath);

    /**
     * @brief Start the wave sequence
     */
    void startLevel();

    /**
     * @brief Reset to initial state
     */
    void reset();

    /**
     * @brief Get current enemy count
     */
    [[nodiscard]] std::size_t getEnemyCount() const noexcept {
        return _enemyCount;
    }

    /**
     * @brief Get current wave number
     */
    [[nodiscard]] std::size_t getCurrentWave() const noexcept {
        return _waveManager.getCurrentWave();
    }

    /**
     * @brief Get total waves in current level
     */
    [[nodiscard]] std::size_t getTotalWaves() const noexcept {
        return _waveManager.getTotalWaves();
    }

    /**
     * @brief Check if all waves completed
     */
    [[nodiscard]] bool isAllWavesComplete() const noexcept {
        return _waveManager.isAllWavesComplete();
    }

    /**
     * @brief Get wave manager state
     */
    [[nodiscard]] WaveState getWaveState() const noexcept {
        return _waveManager.getState();
    }

    /**
     * @brief Set enemy count (used by GameEngine)
     */
    void setEnemyCount(std::size_t count) noexcept { _enemyCount = count; }
    void incrementEnemyCount() noexcept { _enemyCount++; }
    void decrementEnemyCount() noexcept {
        if (_enemyCount > 0) --_enemyCount;
    }

   private:
    friend class GameEngine;

    void spawnEnemy(ECS::Registry& registry, const SpawnRequest& request);
    void spawnBoss(ECS::Registry& registry, const std::string& bossId);
    void spawnObstacle(ECS::Registry& registry);
    void spawnPowerUp(ECS::Registry& registry);

    void updateFallbackSpawning(ECS::Registry& registry, float deltaTime);
    void generateNextObstacleSpawnTime();
    void generateNextPowerUpSpawnTime();

    /**
     * @brief Check if an enemy ID represents a boss enemy
     * @param enemyId Enemy identifier to check
     * @param config Enemy configuration
     * @return true if the enemy is identified as a boss
     */
    bool isBossEnemy(const std::string& enemyId, const shared::EnemyConfig& config) const;

    std::random_device::result_type getRandomSeed() {
        std::random_device rd;
        return rd();
    }

    EventEmitter _emitEvent;
    DataDrivenSpawnerConfig _config;
    WaveManager _waveManager;

    std::size_t _enemyCount = 0;
    uint32_t _nextNetworkId = 1000;
    bool _gameOverEmitted = false;
    bool _bossSpawned = false;
    bool _levelStarted = false;

    float _fallbackSpawnTimer = 0.0F;
    float _nextFallbackSpawnTime = 0.0F;
    std::size_t _fallbackCurrentWave = 1;
    std::size_t _fallbackEnemiesThisWave = 0;

    float _obstacleSpawnTimer = 0.0F;
    float _nextObstacleSpawnTime = 0.0F;
    float _powerUpSpawnTimer = 0.0F;
    float _nextPowerUpSpawnTime = 0.0F;

    std::mt19937 _rng;
    std::uniform_real_distribution<float> _spawnYDist;
    std::uniform_real_distribution<float> _obstacleSpawnTimeDist;
    std::uniform_real_distribution<float> _powerUpSpawnTimeDist;
    std::uniform_int_distribution<int> _powerUpTypeDist;
};

}  // namespace rtype::games::rtype::server
