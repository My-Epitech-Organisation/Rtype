/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameConfig - Game configuration data structures
*/

#ifndef GAME_CONFIG_HPP
    #define GAME_CONFIG_HPP

#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>

namespace PoC {

    /**
     * @brief Player configuration
     */
    struct PlayerConfig {
        std::string name;
        int maxHealth;
        float speed;
        int score;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerConfig, name, maxHealth, speed, score)
    };

    /**
     * @brief Enemy configuration
     */
    struct EnemyConfig {
        std::string type;
        int health;
        int damage;
        float speed;
        int scoreValue;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(EnemyConfig, type, health, damage, speed, scoreValue)
    };

    /**
     * @brief Weapon configuration
     */
    struct WeaponConfig {
        std::string name;
        int damage;
        float fireRate;
        int ammoCapacity;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(WeaponConfig, name, damage, fireRate, ammoCapacity)
    };

    /**
     * @brief Level configuration
     */
    struct LevelConfig {
        int levelNumber;
        std::string name;
        std::string background;
        int enemyCount;
        float difficulty;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(LevelConfig, levelNumber, name, background, enemyCount, difficulty)
    };

    /**
     * @brief Main game configuration
     */
    struct GameConfig {
        std::string version;
        int windowWidth;
        int windowHeight;
        bool fullscreen;
        PlayerConfig player;
        std::vector<EnemyConfig> enemies;
        std::vector<WeaponConfig> weapons;
        std::vector<LevelConfig> levels;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(GameConfig, version, windowWidth, windowHeight, 
                                       fullscreen, player, enemies, weapons, levels)
    };

    /**
     * @brief Load game configuration from JSON file
     * @param filename Path to JSON file
     * @return GameConfig structure
     * @throws std::runtime_error if file cannot be opened or parsed
     */
    inline GameConfig loadGameConfig(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filename);
        }

        nlohmann::json jsonData;
        file >> jsonData;
        
        return jsonData.get<GameConfig>();
    }

    /**
     * @brief Save game configuration to JSON file
     * @param config GameConfig structure
     * @param filename Path to JSON file
     * @throws std::runtime_error if file cannot be written
     */
    inline void saveGameConfig(const GameConfig& config, const std::string& filename) {
        nlohmann::json jsonData = config;
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to create file: " + filename);
        }

        file << jsonData.dump(4); // Pretty print with 4 spaces indentation
    }

} // namespace PoC

#endif // GAME_CONFIG_HPP
