/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SpawnerSystem - Server-side enemy spawning system
*/

#pragma once

#include <functional>
#include <random>

#include "../../../../../engine/ISystem.hpp"
#include "IGameEngine.hpp"

namespace rtype::games::rtype::server {

/**
 * @struct SpawnerConfig
 * @brief Configuration for enemy spawning
 */
struct SpawnerConfig {
    float minSpawnInterval = 1.0F;   ///< Minimum time between spawns
    float maxSpawnInterval = 3.0F;   ///< Maximum time between spawns
    std::size_t maxEnemies = 50;     ///< Maximum enemies allowed
    float spawnX = 800.0F;           ///< X position for spawning (right edge)
    float minSpawnY = 50.0F;         ///< Minimum Y spawn position
    float maxSpawnY = 550.0F;        ///< Maximum Y spawn position
    float bydosSlaveSpeed = 100.0F;  ///< Speed of Bydos slave enemies
};

/**
 * @class SpawnerSystem
 * @brief Server-only system that handles enemy spawning
 *
 * This is a server-specific system - clients receive spawn events
 * through the network, they don't spawn enemies themselves.
 */
class SpawnerSystem : public shared::ISystem {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    /**
     * @brief Construct SpawnerSystem with event emitter and configuration
     * @param emitter Function to emit game events
     * @param config Spawner configuration
     */
    SpawnerSystem(EventEmitter emitter, SpawnerConfig config);

    void update(ECS::Registry& registry, float deltaTime) override;

    [[nodiscard]] const std::string getName() const noexcept override {
        return "SpawnerSystem";
    }

    [[nodiscard]] std::size_t getEnemyCount() const noexcept {
        return _enemyCount;
    }
    void setEnemyCount(std::size_t count) noexcept { _enemyCount = count; }
    void incrementEnemyCount() noexcept { ++_enemyCount; }
    void decrementEnemyCount() noexcept {
        if (_enemyCount > 0) --_enemyCount;
    }

   private:
    void spawnBydosSlave(ECS::Registry& registry);
    void generateNextSpawnTime();

    EventEmitter _emitEvent;
    SpawnerConfig _config;
    float _spawnTimer = 0.0F;
    float _nextSpawnTime = 0.0F;
    std::size_t _enemyCount = 0;
    uint32_t _nextNetworkId = 1000;

    std::mt19937 _rng;
    std::uniform_real_distribution<float> _spawnTimeDist;
    std::uniform_real_distribution<float> _spawnYDist;
};

}  // namespace rtype::games::rtype::server
