/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** WaveManager - Data-driven enemy wave system
*/

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <rtype/engine.hpp>

#include "../../../shared/Config/EntityConfig/EntitiesStructs/LevelConfig.hpp"
#include "../../../shared/Config/EntityConfig/EntitiesStructs/WaveConfig.hpp"

namespace rtype::games::rtype::server {

/**
 * @enum WaveState
 * @brief Current state of the wave manager
 */
enum class WaveState {
    NotStarted,    ///< Level not yet started
    InProgress,    ///< Currently spawning enemies in a wave
    WaveComplete,  ///< Current wave finished, waiting for transition
    AllComplete,   ///< All waves completed
    Failed         ///< Failed to load level config
};

/**
 * @struct SpawnRequest
 * @brief Request to spawn an enemy
 */
struct SpawnRequest {
    std::string enemyId;
    std::optional<float> x;  ///< X position (nullopt = random on right edge)
    std::optional<float> y;  ///< Y position (nullopt = random)
    int32_t count = 1;

    [[nodiscard]] bool hasFixedX() const noexcept { return x.has_value(); }
    [[nodiscard]] bool hasFixedY() const noexcept { return y.has_value(); }
};

/**
 * @class WaveManager
 * @brief Data-driven enemy wave manager
 *
 * This class manages the spawning of enemies based on level configuration
 * files. It reads wave definitions from TOML files and controls the timing
 * and sequence of enemy spawns.
 *
 * Features:
 * - Load wave definitions from configuration files
 * - Track wave progression and timing
 * - Emit spawn requests based on configured delays
 * - Support for multiple enemy types per wave
 * - Graceful handling of missing/malformed configs
 *
 * Example usage:
 * @code
 * WaveManager waveManager;
 * if (waveManager.loadLevel("level_1")) {
 *     waveManager.start();
 *     while (!waveManager.isAllWavesComplete()) {
 *         auto spawns = waveManager.update(deltaTime);
 *         for (const auto& spawn : spawns) {
 *             spawnEnemy(spawn.enemyId, spawn.x, spawn.y, spawn.count);
 *         }
 *     }
 * }
 * @endcode
 */
class WaveManager {
   public:
    using EventEmitter = std::function<void(const engine::GameEvent&)>;

    WaveManager();
    ~WaveManager() = default;

    WaveManager(const WaveManager&) = delete;
    WaveManager& operator=(const WaveManager&) = delete;
    WaveManager(WaveManager&&) = default;
    WaveManager& operator=(WaveManager&&) = default;

    /**
     * @brief Load a level configuration by ID
     * @param levelId The level identifier (e.g., "level_1")
     * @return true if level was loaded successfully
     */
    [[nodiscard]] bool loadLevel(const std::string& levelId);

    /**
     * @brief Load a level configuration from file path
     * @param filepath Path to the level TOML file
     * @return true if level was loaded successfully
     */
    [[nodiscard]] bool loadLevelFromFile(const std::string& filepath);

    /**
     * @brief Start the wave sequence
     */
    void start();

    /**
     * @brief Reset the wave manager to initial state
     */
    void reset();

    /**
     * @brief Update the wave manager and get spawn requests
     * @param deltaTime Time elapsed since last update
     * @param aliveEnemyCount Current number of alive enemies
     * @return Vector of enemies to spawn this frame
     */
    [[nodiscard]] std::vector<SpawnRequest> update(float deltaTime,
                                                   std::size_t aliveEnemyCount);

    /**
     * @brief Get current wave state
     * @return Current state of the wave manager
     */
    [[nodiscard]] WaveState getState() const noexcept { return _state; }

    /**
     * @brief Get current wave number (1-indexed)
     * @return Current wave number
     */
    [[nodiscard]] std::size_t getCurrentWave() const noexcept {
        return _currentWaveIndex + 1;
    }

    /**
     * @brief Get total number of waves in the level
     * @return Total wave count
     */
    [[nodiscard]] std::size_t getTotalWaves() const noexcept {
        return _levelConfig ? _levelConfig->waves.size() : 0;
    }

    /**
     * @brief Check if all waves are completed
     * @return true if all waves finished
     */
    [[nodiscard]] bool isAllWavesComplete() const noexcept {
        return _state == WaveState::AllComplete;
    }

    /**
     * @brief Check if a level is loaded
     * @return true if level config is available
     */
    [[nodiscard]] bool isLevelLoaded() const noexcept {
        return _levelConfig.has_value();
    }

    /**
     * @brief Get the loaded level ID
     * @return Level ID or empty string if not loaded
     */
    [[nodiscard]] const std::string& getLevelId() const noexcept {
        static const std::string empty;
        return _levelConfig ? _levelConfig->id : empty;
    }

    /**
     * @brief Get the level name
     * @return Level name or empty string if not loaded
     */
    [[nodiscard]] const std::string& getLevelName() const noexcept {
        static const std::string empty;
        return _levelConfig ? _levelConfig->name : empty;
    }

    /**
     * @brief Get boss ID for the level (if any)
     * @return Optional boss ID
     */
    [[nodiscard]] std::optional<std::string> getBossId() const noexcept {
        return _levelConfig ? _levelConfig->bossId : std::nullopt;
    }

    /**
     * @brief Get the next level ID (if any)
     * @return Optional next level ID
     */
    [[nodiscard]] std::optional<std::string> getNextLevel() const noexcept {
        return _levelConfig ? _levelConfig->nextLevel : std::nullopt;
    }

    /**
     * @brief Get last error message
     * @return Error message from last failed operation
     */
    [[nodiscard]] const std::string& getLastError() const noexcept {
        return _lastError;
    }

    /**
     * @brief Set whether to wait for all enemies to be killed before next wave
     * @param wait If true, waits for enemies to be cleared
     */
    void setWaitForClear(bool wait) noexcept { _waitForClear = wait; }

    /**
     * @brief Set transition delay between waves
     * @param delay Delay in seconds
     */
    void setWaveTransitionDelay(float delay) noexcept {
        _waveTransitionDelay = delay;
    }

    void setStartDelay(float delay) noexcept { _startDelay = delay; }

   private:
    /**
     * @struct PendingSpawn
     * @brief Internal tracking for spawn timing
     */
    struct PendingSpawn {
        shared::WaveConfig::SpawnEntry entry;
        float remainingDelay = 0.0F;
        int32_t remainingCount = 0;
        bool started = false;
    };

    void advanceToNextWave();
    void prepareCurrentWave();

    std::optional<shared::LevelConfig> _levelConfig;
    std::vector<PendingSpawn> _pendingSpawns;

    WaveState _state = WaveState::NotStarted;
    std::size_t _currentWaveIndex = 0;
    float _waveTimer = 0.0F;
    float _transitionTimer = 0.0F;
    float _waveTransitionDelay = 2.0F;
    bool _waitForClear = true;
    std::string _lastError;
    float _startDelay = 0.0f;
};

}  // namespace rtype::games::rtype::server
