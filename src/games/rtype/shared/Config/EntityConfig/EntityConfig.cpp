/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EntityConfig - Implementation of entity configuration loading
*/

#include "EntityConfig.hpp"

#include <filesystem>
#include <iostream>

#include <toml++/toml.hpp>

namespace rtype::games::rtype::shared {

namespace {

/**
 * @brief Convert string to AIBehavior enum
 */
AIBehavior stringToBehavior(const std::string& str) {
    if (str == "move_left" || str == "MoveLeft") return AIBehavior::MoveLeft;
    if (str == "sine_wave" || str == "SineWave") return AIBehavior::SineWave;
    if (str == "chase" || str == "Chase") return AIBehavior::Chase;
    if (str == "patrol" || str == "Patrol") return AIBehavior::Patrol;
    if (str == "stationary" || str == "Stationary")
        return AIBehavior::Stationary;
    return AIBehavior::MoveLeft;
}

/**
 * @brief Convert string to PowerUpConfig::EffectType
 */
PowerUpConfig::EffectType stringToEffect(const std::string& str) {
    if (str == "health" || str == "Health")
        return PowerUpConfig::EffectType::Health;
    if (str == "speed_boost" || str == "SpeedBoost")
        return PowerUpConfig::EffectType::SpeedBoost;
    if (str == "weapon_upgrade" || str == "WeaponUpgrade")
        return PowerUpConfig::EffectType::WeaponUpgrade;
    if (str == "shield" || str == "Shield")
        return PowerUpConfig::EffectType::Shield;
    if (str == "extra_life" || str == "ExtraLife")
        return PowerUpConfig::EffectType::ExtraLife;
    return PowerUpConfig::EffectType::Health;
}

}  // namespace

bool EntityConfigRegistry::loadFromDirectory(const std::string& configDir) {
    namespace fs = std::filesystem;

    bool success = true;
    const fs::path dir(configDir);

    if (!fs::exists(dir)) {
        std::cerr << "[EntityConfig] Directory not found: " << configDir
                  << "\n";
        return false;
    }
    if (fs::exists(dir / "enemies.toml")) {
        success &= loadEnemies((dir / "enemies.toml").string());
    }
    if (fs::exists(dir / "projectiles.toml")) {
        success &= loadProjectiles((dir / "projectiles.toml").string());
    }
    if (fs::exists(dir / "players.toml")) {
        success &= loadPlayers((dir / "players.toml").string());
    }
    if (fs::exists(dir / "powerups.toml")) {
        success &= loadPowerUps((dir / "powerups.toml").string());
    }
    const fs::path levelsDir = dir / "levels";
    if (fs::exists(levelsDir) && fs::is_directory(levelsDir)) {
        for (const auto& entry : fs::directory_iterator(levelsDir)) {
            if (entry.path().extension() == ".toml") {
                success &= loadLevel(entry.path().string());
            }
        }
    }

    return success;
}

bool EntityConfigRegistry::loadEnemies(const std::string& filepath) {
    try {
        auto tbl = toml::parse_file(filepath);

        if (auto* enemies = tbl["enemy"].as_array()) {
            for (const auto& elem : *enemies) {
                if (auto* enemyTbl = elem.as_table()) {
                    EnemyConfig config;
                    config.id = (*enemyTbl)["id"].value_or("");
                    config.name = (*enemyTbl)["name"].value_or(config.id);
                    config.spriteSheet =
                        (*enemyTbl)["sprite_sheet"].value_or("");

                    // Stats
                    config.health = (*enemyTbl)["health"].value_or(100);
                    config.damage = (*enemyTbl)["damage"].value_or(10);
                    config.scoreValue =
                        (*enemyTbl)["score_value"].value_or(100);

                    // Movement
                    config.behavior = stringToBehavior(
                        (*enemyTbl)["behavior"].value_or("move_left"));
                    config.speed = (*enemyTbl)["speed"].value_or(100.0F);

                    // Hitbox
                    config.hitboxWidth =
                        (*enemyTbl)["hitbox_width"].value_or(32.0F);
                    config.hitboxHeight =
                        (*enemyTbl)["hitbox_height"].value_or(32.0F);

                    // Shooting
                    config.canShoot = (*enemyTbl)["can_shoot"].value_or(false);
                    config.fireRate = (*enemyTbl)["fire_rate"].value_or(1.0F);
                    config.projectileType =
                        (*enemyTbl)["projectile_type"].value_or("");

                    if (config.isValid()) {
                        m_enemies[config.id] = std::move(config);
                    } else {
                        std::cerr << "[EntityConfig] Invalid enemy config: "
                                  << config.id << "\n";
                    }
                }
            }
        }

        std::cout << "[EntityConfig] Loaded " << m_enemies.size()
                  << " enemies from " << filepath << "\n";
        return true;

    } catch (const toml::parse_error& err) {
        std::cerr << "[EntityConfig] Failed to parse " << filepath << ": "
                  << err.what() << "\n";
        return false;
    }
}

bool EntityConfigRegistry::loadProjectiles(const std::string& filepath) {
    try {
        auto tbl = toml::parse_file(filepath);

        if (auto* projectiles = tbl["projectile"].as_array()) {
            for (const auto& elem : *projectiles) {
                if (auto* projTbl = elem.as_table()) {
                    ProjectileConfig config;
                    config.id = (*projTbl)["id"].value_or("");
                    config.spriteSheet =
                        (*projTbl)["sprite_sheet"].value_or("");

                    config.damage = (*projTbl)["damage"].value_or(10);
                    config.speed = (*projTbl)["speed"].value_or(300.0F);
                    config.lifetime = (*projTbl)["lifetime"].value_or(5.0F);

                    config.hitboxWidth =
                        (*projTbl)["hitbox_width"].value_or(8.0F);
                    config.hitboxHeight =
                        (*projTbl)["hitbox_height"].value_or(4.0F);

                    config.piercing = (*projTbl)["piercing"].value_or(false);
                    config.maxHits = (*projTbl)["max_hits"].value_or(1);

                    if (config.isValid()) {
                        m_projectiles[config.id] = std::move(config);
                    }
                }
            }
        }

        std::cout << "[EntityConfig] Loaded " << m_projectiles.size()
                  << " projectiles from " << filepath << "\n";
        return true;

    } catch (const toml::parse_error& err) {
        std::cerr << "[EntityConfig] Failed to parse " << filepath << ": "
                  << err.what() << "\n";
        return false;
    }
}

bool EntityConfigRegistry::loadPlayers(const std::string& filepath) {
    try {
        auto tbl = toml::parse_file(filepath);

        if (auto* players = tbl["player"].as_array()) {
            for (const auto& elem : *players) {
                if (auto* playerTbl = elem.as_table()) {
                    PlayerConfig config;
                    config.id = (*playerTbl)["id"].value_or("");
                    config.name = (*playerTbl)["name"].value_or(config.id);
                    config.spriteSheet =
                        (*playerTbl)["sprite_sheet"].value_or("");

                    config.health = (*playerTbl)["health"].value_or(100);
                    config.speed = (*playerTbl)["speed"].value_or(200.0F);
                    config.fireRate = (*playerTbl)["fire_rate"].value_or(5.0F);

                    config.hitboxWidth =
                        (*playerTbl)["hitbox_width"].value_or(32.0F);
                    config.hitboxHeight =
                        (*playerTbl)["hitbox_height"].value_or(16.0F);

                    config.defaultProjectile =
                        (*playerTbl)["default_projectile"].value_or(
                            "basic_bullet");

                    if (config.isValid()) {
                        m_players[config.id] = std::move(config);
                    }
                }
            }
        }

        std::cout << "[EntityConfig] Loaded " << m_players.size()
                  << " players from " << filepath << "\n";
        return true;

    } catch (const toml::parse_error& err) {
        std::cerr << "[EntityConfig] Failed to parse " << filepath << ": "
                  << err.what() << "\n";
        return false;
    }
}

bool EntityConfigRegistry::loadPowerUps(const std::string& filepath) {
    try {
        auto tbl = toml::parse_file(filepath);

        if (auto* powerups = tbl["powerup"].as_array()) {
            for (const auto& elem : *powerups) {
                if (auto* puTbl = elem.as_table()) {
                    PowerUpConfig config;
                    config.id = (*puTbl)["id"].value_or("");
                    config.name = (*puTbl)["name"].value_or(config.id);
                    config.spriteSheet = (*puTbl)["sprite_sheet"].value_or("");

                    config.effect =
                        stringToEffect((*puTbl)["effect"].value_or("health"));
                    config.duration = (*puTbl)["duration"].value_or(0.0F);
                    config.value = (*puTbl)["value"].value_or(25);

                    config.hitboxWidth =
                        (*puTbl)["hitbox_width"].value_or(16.0F);
                    config.hitboxHeight =
                        (*puTbl)["hitbox_height"].value_or(16.0F);

                    if (config.isValid()) {
                        m_powerUps[config.id] = std::move(config);
                    }
                }
            }
        }

        std::cout << "[EntityConfig] Loaded " << m_powerUps.size()
                  << " power-ups from " << filepath << "\n";
        return true;

    } catch (const toml::parse_error& err) {
        std::cerr << "[EntityConfig] Failed to parse " << filepath << ": "
                  << err.what() << "\n";
        return false;
    }
}

bool EntityConfigRegistry::loadLevel(const std::string& filepath) {
    try {
        auto tbl = toml::parse_file(filepath);

        LevelConfig config;
        config.id = tbl["level"]["id"].value_or("");
        config.name = tbl["level"]["name"].value_or(config.id);
        config.backgroundPath = tbl["level"]["background"].value_or("");
        config.scrollSpeed = tbl["level"]["scroll_speed"].value_or(50.0F);

        if (auto boss = tbl["level"]["boss"].value<std::string>()) {
            config.bossId = *boss;
        }

        if (auto* waves = tbl["wave"].as_array()) {
            for (const auto& waveElem : *waves) {
                if (auto* waveTbl = waveElem.as_table()) {
                    WaveConfig wave;
                    wave.waveNumber = (*waveTbl)["number"].value_or(1);
                    wave.spawnDelay = (*waveTbl)["spawn_delay"].value_or(0.5F);
                    if (auto* spawns = (*waveTbl)["spawn"].as_array()) {
                        for (const auto& spawnElem : *spawns) {
                            if (auto* spawnTbl = spawnElem.as_table()) {
                                WaveConfig::SpawnEntry spawn;
                                spawn.enemyId =
                                    (*spawnTbl)["enemy"].value_or("");
                                spawn.x = (*spawnTbl)["x"].value_or(800.0F);
                                spawn.y = (*spawnTbl)["y"].value_or(300.0F);
                                spawn.delay =
                                    (*spawnTbl)["delay"].value_or(0.0F);
                                spawn.count = (*spawnTbl)["count"].value_or(1);
                                wave.spawns.push_back(spawn);
                            }
                        }
                    }

                    if (wave.isValid()) {
                        config.waves.push_back(std::move(wave));
                    }
                }
            }
        }

        if (config.isValid()) {
            m_levels[config.id] = std::move(config);
            std::cout << "[EntityConfig] Loaded level: " << config.id << "\n";
            return true;
        }

        return false;

    } catch (const toml::parse_error& err) {
        std::cerr << "[EntityConfig] Failed to parse level " << filepath << ": "
                  << err.what() << "\n";
        return false;
    }
}

OptionalRef<EnemyConfig> EntityConfigRegistry::getEnemy(const std::string& id) const {
    auto it = m_enemies.find(id);
    if (it != m_enemies.end()) {
        return std::cref(it->second);
    }
    return std::nullopt;
}

OptionalRef<ProjectileConfig> EntityConfigRegistry::getProjectile(
    const std::string& id) const {
    auto it = m_projectiles.find(id);
    if (it != m_projectiles.end()) {
        return std::cref(it->second);
    }
    return std::nullopt;
}

OptionalRef<PlayerConfig> EntityConfigRegistry::getPlayer(
    const std::string& id) const {
    auto it = m_players.find(id);
    if (it != m_players.end()) {
        return std::cref(it->second);
    }
    return std::nullopt;
}

OptionalRef<PowerUpConfig> EntityConfigRegistry::getPowerUp(
    const std::string& id) const {
    auto it = m_powerUps.find(id);
    if (it != m_powerUps.end()) {
        return std::cref(it->second);
    }
    return std::nullopt;
}

OptionalRef<LevelConfig> EntityConfigRegistry::getLevel(const std::string& id) const {
    auto it = m_levels.find(id);
    if (it != m_levels.end()) {
        return std::cref(it->second);
    }
    return std::nullopt;
}

void EntityConfigRegistry::clear() {
    m_enemies.clear();
    m_projectiles.clear();
    m_players.clear();
    m_powerUps.clear();
    m_levels.clear();
}

}  // namespace rtype::games::rtype::shared
