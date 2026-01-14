/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** EntityConfig - Implementation of entity configuration loading
*/

#include "EntityConfig.hpp"

#include <filesystem>

#include <toml++/toml.hpp>

#include "Logger/Macros.hpp"

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
    if (str == "zigzag" || str == "ZigZag") return AIBehavior::ZigZag;
    if (str == "divebomb" || str == "DiveBomb") return AIBehavior::DiveBomb;
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
    if (str == "extra_life" || str == "ExtraLife" || str == "health_boost" ||
        str == "HealthBoost")
        return PowerUpConfig::EffectType::HealthBoost;
    return PowerUpConfig::EffectType::Health;
}

/**
 * @brief Try to find a file in multiple locations
 * @param filepath Relative path to search for
 * @return First existing path, or original if none found
 */
std::string findConfigPath(const std::string& filepath) {
    namespace fs = std::filesystem;

    const std::vector<std::string> searchPaths = {filepath, "../" + filepath,
                                                  "../../" + filepath,
                                                  "../../../" + filepath};

    for (const auto& path : searchPaths) {
        if (fs::exists(path)) {
            return path;
        }
    }

    return filepath;
}

}  // namespace

bool EntityConfigRegistry::loadFromDirectory(const std::string& configDir) {
    namespace fs = std::filesystem;

    bool success = true;
    const fs::path dir(configDir);

    if (!fs::exists(dir)) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[EntityConfig] Directory not found: " << configDir);
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

                    // Visual - Color filter
                    if (auto* colorArray = (*enemyTbl)["color"].as_array()) {
                        if (colorArray->size() >= 4) {
                            config.colorR = static_cast<uint8_t>(
                                colorArray->get(0)->value<int64_t>().value_or(
                                    255));
                            config.colorG = static_cast<uint8_t>(
                                colorArray->get(1)->value<int64_t>().value_or(
                                    255));
                            config.colorB = static_cast<uint8_t>(
                                colorArray->get(2)->value<int64_t>().value_or(
                                    255));
                            config.colorA = static_cast<uint8_t>(
                                colorArray->get(3)->value<int64_t>().value_or(
                                    255));
                        }
                    }

                    config.isBoss = (*enemyTbl)["is_boss"].value_or(false);
                    config.bossType = (*enemyTbl)["boss_type"].value_or("");
                    config.levelCompleteTrigger =
                        (*enemyTbl)["level_complete_trigger"].value_or(true);
                    config.phaseTransitionDuration =
                        (*enemyTbl)["phase_transition_duration"].value_or(1.0F);
                    config.invulnerabilityDuration =
                        (*enemyTbl)["invulnerability_duration"].value_or(1.0F);

                    if (auto* phases = (*enemyTbl)["phase"].as_array()) {
                        for (const auto& phaseElem : *phases) {
                            if (auto* phaseTbl = phaseElem.as_table()) {
                                BossPhaseConfig phase;
                                phase.healthThreshold =
                                    (*phaseTbl)["health_threshold"].value_or(
                                        1.0F);
                                phase.name =
                                    (*phaseTbl)["name"].value_or("Phase");
                                phase.primaryPattern =
                                    (*phaseTbl)["primary_pattern"].value_or("");
                                phase.secondaryPattern =
                                    (*phaseTbl)["secondary_pattern"].value_or(
                                        "");
                                phase.speedMultiplier =
                                    (*phaseTbl)["speed_multiplier"].value_or(
                                        1.0F);
                                phase.attackSpeedMultiplier =
                                    (*phaseTbl)["attack_speed_multiplier"]
                                        .value_or(1.0F);
                                phase.damageMultiplier =
                                    (*phaseTbl)["damage_multiplier"].value_or(
                                        1.0F);
                                if (auto* col = (*phaseTbl)["color"].as_array();
                                    col && col->size() >= 3) {
                                    phase.colorR = static_cast<uint8_t>(
                                        col->get(0)->value<int64_t>().value_or(
                                            255));
                                    phase.colorG = static_cast<uint8_t>(
                                        col->get(1)->value<int64_t>().value_or(
                                            255));
                                    phase.colorB = static_cast<uint8_t>(
                                        col->get(2)->value<int64_t>().value_or(
                                            255));
                                }
                                config.phases.push_back(std::move(phase));
                            }
                        }
                    }

                    if (auto* weakPoints =
                            (*enemyTbl)["weak_point"].as_array()) {
                        for (const auto& wpElem : *weakPoints) {
                            if (auto* wpTbl = wpElem.as_table()) {
                                WeakPointConfig wp;
                                wp.id = (*wpTbl)["id"].value_or("");
                                wp.type = (*wpTbl)["type"].value_or("generic");
                                wp.offsetX =
                                    (*wpTbl)["offset_x"].value_or(0.0F);
                                wp.offsetY =
                                    (*wpTbl)["offset_y"].value_or(0.0F);
                                wp.health = (*wpTbl)["health"].value_or(100);
                                wp.hitboxWidth =
                                    (*wpTbl)["hitbox_width"].value_or(32.0F);
                                wp.hitboxHeight =
                                    (*wpTbl)["hitbox_height"].value_or(32.0F);
                                wp.bonusScore =
                                    (*wpTbl)["bonus_score"].value_or(500);
                                wp.damageToParent =
                                    (*wpTbl)["damage_to_parent"].value_or(0);
                                wp.critical =
                                    (*wpTbl)["critical"].value_or(false);
                                wp.disablesAttack =
                                    (*wpTbl)["disables_attack"].value_or("");
                                wp.segmentIndex =
                                    (*wpTbl)["segment_index"].value_or(-1);
                                config.weakPoints.push_back(std::move(wp));
                            }
                        }
                    }

                    if (config.isValid()) {
                        m_enemies[config.id] = std::move(config);
                    } else {
                        LOG_WARNING_CAT(::rtype::LogCategory::GameEngine,
                                        "[EntityConfig] Invalid enemy config: "
                                            << config.id);
                    }
                }
            }
        }

        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[EntityConfig] Loaded " << m_enemies.size()
                                              << " enemies from " << filepath);
        return true;
    } catch (const toml::parse_error& err) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[EntityConfig] Failed to parse " << filepath << ": "
                                                        << err.what());
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

        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[EntityConfig] Loaded " << m_projectiles.size()
                                              << " projectiles from "
                                              << filepath);
        return true;
    } catch (const toml::parse_error& err) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[EntityConfig] Failed to parse " << filepath << ": "
                                                        << err.what());
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

        LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                     "[EntityConfig] Loaded " << m_players.size()
                                              << " players from " << filepath);
        return true;
    } catch (const toml::parse_error& err) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[EntityConfig] Failed to parse " << filepath << ": "
                                                        << err.what());
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

                    if (auto* colorArray = (*puTbl)["color"].as_array()) {
                        if (colorArray->size() >= 4) {
                            config.colorR = static_cast<uint8_t>(
                                colorArray->get(0)->value<int64_t>().value_or(
                                    255));
                            config.colorG = static_cast<uint8_t>(
                                colorArray->get(1)->value<int64_t>().value_or(
                                    255));
                            config.colorB = static_cast<uint8_t>(
                                colorArray->get(2)->value<int64_t>().value_or(
                                    255));
                            config.colorA = static_cast<uint8_t>(
                                colorArray->get(3)->value<int64_t>().value_or(
                                    255));
                        }
                    }

                    if (config.isValid()) {
                        m_powerUps[config.id] = std::move(config);
                    }
                }
            }
        }

        LOG_INFO_CAT(::rtype::LogCategory::GameEngine, "[EntityConfig] Loaded "
                                                           << m_powerUps.size()
                                                           << " power-ups from "
                                                           << filepath);
        return true;
    } catch (const toml::parse_error& err) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[EntityConfig] Failed to parse " << filepath << ": "
                                                        << err.what());
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
                                if (auto xVal =
                                        (*spawnTbl)["x"].value<double>()) {
                                    spawn.x = static_cast<float>(*xVal);
                                }
                                if (auto yVal =
                                        (*spawnTbl)["y"].value<double>()) {
                                    spawn.y = static_cast<float>(*yVal);
                                }
                                spawn.delay =
                                    (*spawnTbl)["delay"].value_or(0.0F);
                                spawn.count = (*spawnTbl)["count"].value_or(1);
                                wave.spawns.push_back(spawn);
                            }
                        }
                    }

                    if (auto* powerups = (*waveTbl)["powerup"].as_array()) {
                        for (const auto& powerupElem : *powerups) {
                            if (auto* powerupTbl = powerupElem.as_table()) {
                                WaveConfig::PowerUpEntry powerup;
                                powerup.powerUpId =
                                    (*powerupTbl)["id"].value_or("");
                                if (auto xVal =
                                        (*powerupTbl)["x"].value<double>()) {
                                    powerup.x = static_cast<float>(*xVal);
                                }
                                if (auto yVal =
                                        (*powerupTbl)["y"].value<double>()) {
                                    powerup.y = static_cast<float>(*yVal);
                                }
                                powerup.delay =
                                    (*powerupTbl)["delay"].value_or(0.0F);
                                wave.powerups.push_back(powerup);
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
            LOG_INFO_CAT(::rtype::LogCategory::GameEngine,
                         "[EntityConfig] Loaded level: " << config.id);
            return true;
        }
        return false;
    } catch (const toml::parse_error& err) {
        LOG_ERROR_CAT(::rtype::LogCategory::GameEngine,
                      "[EntityConfig] Failed to parse level "
                          << filepath << ": " << err.what());
        return false;
    }
}

OptionalRef<EnemyConfig> EntityConfigRegistry::getEnemy(
    const std::string& id) const {
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

OptionalRef<LevelConfig> EntityConfigRegistry::getLevel(
    const std::string& id) const {
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

bool EntityConfigRegistry::loadEnemiesWithSearch(const std::string& filepath) {
    return loadEnemies(findConfigPath(filepath));
}

bool EntityConfigRegistry::loadProjectilesWithSearch(
    const std::string& filepath) {
    return loadProjectiles(findConfigPath(filepath));
}

bool EntityConfigRegistry::loadPlayersWithSearch(const std::string& filepath) {
    return loadPlayers(findConfigPath(filepath));
}

bool EntityConfigRegistry::loadPowerUpsWithSearch(const std::string& filepath) {
    return loadPowerUps(findConfigPath(filepath));
}

}  // namespace rtype::games::rtype::shared
