/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SpawnerSystem - Server-side enemy spawning system
*/

#pragma once

#include <functional>
#include <random>

#include <rtype/engine.hpp>

namespace rtype::games::rtype::server {

/**
 * @struct SpawnerConfig
 * @brief Configuration for enemy spawning
 */
struct SpawnerConfig {
    float minSpawnInterval = 1.0F;   // Minimum time between spawns
    float maxSpawnInterval = 3.0F;   // Maximum time between spawns
    std::size_t maxEnemies = 50;     // Maximum enemies allowed
    float spawnX = 800.0F;           // X position for spawning (right edge)
    float minSpawnY = 50.0F;         // Minimum Y spawn position
    float maxSpawnY = 550.0F;        // Maximum Y spawn position
    float bydosSlaveSpeed = 100.0F;  // Speed of Bydos slave enemies
    float stationarySpawnInset =
        120.0F;  // How far inside the screen to spawn stationary enemies

    float weightMoveLeft = 0.2F;
    float weightSineWave = 0.1F;
    float weightZigZag = 0.3F;
    float weightDiveBomb = 1.0F;
    float weightStationary = 1.2F;
    float weightChase = 1.5F;

    float obstacleMinInterval = 3.0F;
    float obstacleMaxInterval = 6.0F;
    float obstacleSpeed = 80.0F;
    float obstacleWidth = 64.0F;
    float obstacleHeight = 64.0F;
    int32_t obstacleDamage = 20;

    float powerUpMinInterval = 8.0F;
    float powerUpMaxInterval = 14.0F;
    float powerUpSpeed = 70.0F;

    std::size_t maxWaves = 1;           // 0 = infinite waves
    std::size_t enemiesPerWave = 5;
};

/**
 * @class SpawnerSystem
 * @brief Server-only system that handles enemy spawning
 *
 * This is a server-specific system - clients receive spawn events
 * through the network, they don't spawn enemies themselves.
 */
class SpawnerSystem : public ::rtype::engine::ASystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    /**
     * @brief Construct SpawnerSystem with event emitter and configuration
     * @param emitter Function to emit game events
     * @param config Spawner configuration
     */
    SpawnerSystem(EventEmitter emitter, SpawnerConfig config);

    void update(ECS::Registry& registry, float deltaTime) override;

    /**
     * @brief Get the current enemy count (read-only)
     * @return Current number of tracked enemies
     */
    [[nodiscard]] std::size_t getEnemyCount() const noexcept {
        return _enemyCount;
    }

    /**
     * @brief Get the current wave number
     * @return Current wave (starts at 1)
     */
    [[nodiscard]] std::size_t getCurrentWave() const noexcept {
        return _currentWave;
    }

    /**
     * @brief Check if all waves are completed
     * @return true if maxWaves is set and reached
     */
    [[nodiscard]] bool isAllWavesCompleted() const noexcept {
        return _config.maxWaves > 0 && _currentWave > _config.maxWaves;
    }

   private:
    friend class GameEngine;

    void setEnemyCount(std::size_t count) noexcept { _enemyCount = count; }
    void incrementEnemyCount() noexcept { _enemyCount++; }
    void decrementEnemyCount() noexcept {
        if (_enemyCount > 0) --_enemyCount;
    }
    void spawnBydosSlave(ECS::Registry& registry);
    void generateNextSpawnTime();
    void generateNextObstacleSpawnTime();
    void generateNextPowerUpSpawnTime();

    void spawnObstacle(ECS::Registry& registry);
    void spawnPowerUp(ECS::Registry& registry);
    std::random_device::result_type getRandomSeed() {
        std::random_device rd;
        return rd();
    }

    EventEmitter _emitEvent;
    SpawnerConfig _config;
    float _spawnTimer = 0.0F;
    float _nextSpawnTime = 0.0F;
    float _obstacleSpawnTimer = 0.0F;
    float _nextObstacleSpawnTime = 0.0F;
    float _powerUpSpawnTimer = 0.0F;
    float _nextPowerUpSpawnTime = 0.0F;
    std::size_t _enemyCount = 0;
    uint32_t _nextNetworkId = 1000;
    std::size_t _currentWave = 1;
    std::size_t _enemiesSpawnedThisWave = 0;
    bool _gameOverEmitted = false;

    std::mt19937 _rng;
    std::uniform_real_distribution<float> _spawnTimeDist;
    std::uniform_real_distribution<float> _spawnYDist;
    std::uniform_real_distribution<float> _obstacleSpawnTimeDist;
    std::uniform_real_distribution<float> _powerUpSpawnTimeDist;
    std::uniform_int_distribution<int> _powerUpTypeDist;
};

}  // namespace rtype::games::rtype::server
