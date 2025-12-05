/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EntityConfig - Entity configuration definitions from config files
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

#include "Components/AIComponent.hpp"
#include "Components/EntityType.hpp"

namespace rtype::games::rtype::shared {

/**
 * @struct EnemyConfig
 * @brief Configuration for an enemy type loaded from config files
 */
struct EnemyConfig {
    std::string id;           // Unique identifier (e.g., "basic_enemy", "boss_1")
    std::string name;         // Display name
    std::string spriteSheet;  // Path to sprite sheet

    // Stats
    int32_t health = 100;
    int32_t damage = 10;
    int32_t scoreValue = 100;

    // Movement
    AIBehavior behavior = AIBehavior::MoveLeft;
    float speed = 100.0F;

    // Hitbox
    float hitboxWidth = 32.0F;
    float hitboxHeight = 32.0F;

    // Shooting (optional)
    bool canShoot = false;
    float fireRate = 1.0F;         // Shots per second
    std::string projectileType;    // Reference to projectile config

    /**
     * @brief Validate the enemy configuration
     * @return true if configuration is valid
     */
    [[nodiscard]] bool isValid() const noexcept {
        // Stationary enemies don't need speed > 0
        return !id.empty() && health > 0 && (speed >= 0 || behavior == AIBehavior::Stationary);
    }
};

/**
 * @struct ProjectileConfig
 * @brief Configuration for a projectile type
 */
struct ProjectileConfig {
    std::string id;            // Unique identifier
    std::string spriteSheet;   // Path to sprite sheet

    int32_t damage = 10;
    float speed = 300.0F;
    float lifetime = 5.0F;     // Seconds before auto-destroy

    // Hitbox
    float hitboxWidth = 8.0F;
    float hitboxHeight = 4.0F;

    // Effects
    bool piercing = false;     // Can hit multiple enemies
    int32_t maxHits = 1;       // Max enemies hit (if piercing)

    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && damage > 0 && speed > 0;
    }
};

/**
 * @struct PlayerConfig
 * @brief Configuration for player ships
 */
struct PlayerConfig {
    std::string id;
    std::string name;
    std::string spriteSheet;

    int32_t health = 100;
    float speed = 200.0F;
    float fireRate = 5.0F;

    // Hitbox
    float hitboxWidth = 32.0F;
    float hitboxHeight = 16.0F;

    // Starting projectile
    std::string defaultProjectile = "basic_bullet";

    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && health > 0 && speed > 0;
    }
};

/**
 * @struct PowerUpConfig
 * @brief Configuration for power-up items
 */
struct PowerUpConfig {
    std::string id;
    std::string name;
    std::string spriteSheet;

    // Effect type
    enum class EffectType : uint8_t {
        Health,
        SpeedBoost,
        WeaponUpgrade,
        Shield,
        ExtraLife
    };
    EffectType effect = EffectType::Health;

    float duration = 0.0F;    // 0 = permanent (like health)
    int32_t value = 25;       // Health amount, speed %, etc.

    // Hitbox
    float hitboxWidth = 16.0F;
    float hitboxHeight = 16.0F;

    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty();
    }
};

/**
 * @struct WaveConfig
 * @brief Configuration for an enemy wave
 */
struct WaveConfig {
    int32_t waveNumber = 1;
    float spawnDelay = 0.5F;  // Delay between spawns

    struct SpawnEntry {
        std::string enemyId;  // Reference to EnemyConfig
        float x = 800.0F;     // Spawn X position
        float y = 300.0F;     // Spawn Y position
        float delay = 0.0F;   // Delay from wave start
        int32_t count = 1;    // Number to spawn
    };

    std::vector<SpawnEntry> spawns;

    [[nodiscard]] bool isValid() const noexcept {
        return waveNumber > 0 && !spawns.empty();
    }
};

/**
 * @struct LevelConfig
 * @brief Configuration for a complete level
 */
struct LevelConfig {
    std::string id;
    std::string name;
    std::string backgroundPath;

    float scrollSpeed = 50.0F;  // Background scroll speed
    std::vector<WaveConfig> waves;

    // Boss (optional)
    std::optional<std::string> bossId;

    [[nodiscard]] bool isValid() const noexcept {
        return !id.empty() && !waves.empty();
    }
};

/**
 * @class EntityConfigRegistry
 * @brief Central registry for all entity configurations
 *
 * Loads and provides access to all entity configurations.
 * Used by factories to create entities with correct stats.
 */
class EntityConfigRegistry {
 public:
    /**
     * @brief Get singleton instance
     */
    static EntityConfigRegistry& getInstance() {
        static EntityConfigRegistry instance;
        return instance;
    }

    // Prevent copying
    EntityConfigRegistry(const EntityConfigRegistry&) = delete;
    EntityConfigRegistry& operator=(const EntityConfigRegistry&) = delete;

    /**
     * @brief Load all configurations from directory
     * @param configDir Path to configuration directory
     * @return true if loaded successfully
     */
    bool loadFromDirectory(const std::string& configDir);

    /**
     * @brief Load enemy configurations from file
     * @param filepath Path to enemies.toml
     */
    bool loadEnemies(const std::string& filepath);

    /**
     * @brief Load projectile configurations from file
     * @param filepath Path to projectiles.toml
     */
    bool loadProjectiles(const std::string& filepath);

    /**
     * @brief Load player configurations from file
     * @param filepath Path to players.toml
     */
    bool loadPlayers(const std::string& filepath);

    /**
     * @brief Load power-up configurations from file
     * @param filepath Path to powerups.toml
     */
    bool loadPowerUps(const std::string& filepath);

    /**
     * @brief Load level configurations from file
     * @param filepath Path to level file
     */
    bool loadLevel(const std::string& filepath);

    // Getters
    [[nodiscard]] const EnemyConfig* getEnemy(const std::string& id) const;
    [[nodiscard]] const ProjectileConfig* getProjectile(const std::string& id) const;
    [[nodiscard]] const PlayerConfig* getPlayer(const std::string& id) const;
    [[nodiscard]] const PowerUpConfig* getPowerUp(const std::string& id) const;
    [[nodiscard]] const LevelConfig* getLevel(const std::string& id) const;

    // Get all configs
    [[nodiscard]] const std::unordered_map<std::string, EnemyConfig>& getAllEnemies() const {
        return m_enemies;
    }
    [[nodiscard]] const std::unordered_map<std::string, ProjectileConfig>& getAllProjectiles() const {
        return m_projectiles;
    }
    [[nodiscard]] const std::unordered_map<std::string, PlayerConfig>& getAllPlayers() const {
        return m_players;
    }
    [[nodiscard]] const std::unordered_map<std::string, PowerUpConfig>& getAllPowerUps() const {
        return m_powerUps;
    }

    /**
     * @brief Clear all loaded configurations
     */
    void clear();

 private:
    EntityConfigRegistry() = default;

    std::unordered_map<std::string, EnemyConfig> m_enemies;
    std::unordered_map<std::string, ProjectileConfig> m_projectiles;
    std::unordered_map<std::string, PlayerConfig> m_players;
    std::unordered_map<std::string, PowerUpConfig> m_powerUps;
    std::unordered_map<std::string, LevelConfig> m_levels;
};

}  // namespace rtype::games::rtype::shared
