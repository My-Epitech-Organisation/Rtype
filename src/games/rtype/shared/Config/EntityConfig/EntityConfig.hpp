/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EntityConfig - Entity configuration definitions from config files
*/

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

#include "EntitiesStructs/EnemyConfig.hpp"
#include "EntitiesStructs/LevelConfig.hpp"
#include "EntitiesStructs/PlayerConfig.hpp"
#include "EntitiesStructs/PowerUpConfig.hpp"
#include "EntitiesStructs/ProjectileConfig.hpp"
#include "EntitiesStructs/WaveConfig.hpp"

namespace rtype::games::rtype::shared {

template <typename T>
using OptionalRef = std::optional<std::reference_wrapper<const T>>;

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

    [[nodiscard]] OptionalRef<EnemyConfig> getEnemy(
        const std::string& id) const;
    [[nodiscard]] OptionalRef<ProjectileConfig> getProjectile(
        const std::string& id) const;
    [[nodiscard]] OptionalRef<PlayerConfig> getPlayer(
        const std::string& id) const;
    [[nodiscard]] OptionalRef<PowerUpConfig> getPowerUp(
        const std::string& id) const;
    [[nodiscard]] OptionalRef<LevelConfig> getLevel(
        const std::string& id) const;

    [[nodiscard]] const std::unordered_map<std::string, EnemyConfig>&
    getAllEnemies() const {
        return m_enemies;
    }
    [[nodiscard]] const std::unordered_map<std::string, ProjectileConfig>&
    getAllProjectiles() const {
        return m_projectiles;
    }
    [[nodiscard]] const std::unordered_map<std::string, PlayerConfig>&
    getAllPlayers() const {
        return m_players;
    }
    [[nodiscard]] const std::unordered_map<std::string, PowerUpConfig>&
    getAllPowerUps() const {
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
