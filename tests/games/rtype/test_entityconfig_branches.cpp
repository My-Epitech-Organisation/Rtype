/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_entityconfig_branches - Unit tests for EntityConfig branch coverage
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "../../../src/games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"

using namespace rtype::games::rtype::shared;

static std::filesystem::path makeTempFile(const std::string& name,
                                         const std::string& content) {
    auto dir = std::filesystem::temp_directory_path() /
               ("rtype_test_" + std::to_string(std::hash<std::string>{}(name)));
    std::filesystem::create_directories(dir);
    auto file = dir / name;
    std::ofstream ofs(file.string());
    ofs << content;
    ofs.close();
    return file;
}

TEST(EntityConfigBranches, LoadEnemiesWithColorAndInvalidEntry) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    const std::string toml = R"(enemy = [
  { id = "enemy_good", name = "Good", sprite_sheet = "sheet.png", health = 42, damage = 7, score_value = 10, behavior = "stationary", speed = 0.0, hitbox_width = 16.0, hitbox_height = 16.0, can_shoot = false, fire_rate = 1.0, projectile_type = "", color = [10, 20, 30, 40] },
  { name = "Bad", health = -5 }
])";

    auto file = makeTempFile("enemies.toml", toml);
    EXPECT_TRUE(reg.loadEnemies(file.string()));

    auto enemyOpt = reg.getEnemy("enemy_good");
    ASSERT_TRUE(enemyOpt.has_value());
    const EnemyConfig& e = enemyOpt->get();
    EXPECT_EQ(e.health, 42);
    EXPECT_EQ(e.colorR, 10);
    EXPECT_EQ(e.colorG, 20);
    EXPECT_EQ(e.colorB, 30);
    EXPECT_EQ(e.colorA, 40);

    // Invalid entry should not be present
    EXPECT_FALSE(reg.getEnemy("" /* no id */).has_value());
}

TEST(EntityConfigBranches, LoadProjectilesPiercingAndDefaults) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    const std::string toml = R"(projectile = [
  { id = "pierce", sprite_sheet = "p.png", damage = 15, speed = 400.0, lifetime = 3.0, piercing = true, max_hits = 3 }
])";

    auto file = makeTempFile("projectiles.toml", toml);
    EXPECT_TRUE(reg.loadProjectiles(file.string()));

    auto projOpt = reg.getProjectile("pierce");
    ASSERT_TRUE(projOpt.has_value());
    const ProjectileConfig& p = projOpt->get();
    EXPECT_TRUE(p.piercing);
    EXPECT_EQ(p.maxHits, 3);
}

TEST(EntityConfigBranches, LoadPowerUpsEffectAndColorFallback) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    const std::string toml = R"(powerup = [
  { id = "speed", name = "Speed", sprite_sheet = "s.png", effect = "SpeedBoost", duration = 2.5, value = 10, color = [1,2] }
])";

    auto file = makeTempFile("powerups.toml", toml);
    EXPECT_TRUE(reg.loadPowerUps(file.string()));

    auto puOpt = reg.getPowerUp("speed");
    ASSERT_TRUE(puOpt.has_value());
    const PowerUpConfig& pu = puOpt->get();
    // color array was too small, so defaults should remain
    EXPECT_EQ(pu.colorR, 255);
    EXPECT_EQ(pu.effect, PowerUpConfig::EffectType::SpeedBoost);
}

TEST(EntityConfigBranches, LoadLevelWithBossAndWaves) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    const std::string toml = R"([level]
id = "lvl1"
name = "First"
background = "bg.png"
scroll_speed = 60.0
boss = "bigboss"

[[wave]]
number = 1
spawn_delay = 0.5

  [[wave.spawn]]
  enemy = "enemy_good"
  x = 700.0
  y = 200.0
  delay = 0.0
  count = 2
)";

    auto file = makeTempFile("level.toml", toml);
    EXPECT_TRUE(reg.loadLevel(file.string()));

    auto levelOpt = reg.getLevel("lvl1");
    ASSERT_TRUE(levelOpt.has_value());
    const LevelConfig& lvl = levelOpt->get();
    EXPECT_EQ(lvl.bossId.has_value(), true);
    EXPECT_EQ(*lvl.bossId, "bigboss");
    ASSERT_FALSE(lvl.waves.empty());
    EXPECT_EQ(lvl.waves.front().spawns.front().enemyId, "enemy_good");
}

TEST(EntityConfigBranches, LoadFromDirectoryHandlesMissing) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    // Non-existent directory should return false
    EXPECT_FALSE(reg.loadFromDirectory("/this/path/does/not/exist_xyz"));
}

TEST(EntityConfigBranches, BehaviorStringVariants) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    const std::string toml = R"(enemy = [
  { id = "e_move", behavior = "move_left" },
  { id = "e_Move", behavior = "MoveLeft" },
  { id = "e_sine", behavior = "sine_wave" },
  { id = "e_Sine", behavior = "SineWave" },
  { id = "e_chase", behavior = "chase" },
  { id = "e_Chase", behavior = "Chase" },
  { id = "e_patrol", behavior = "patrol" },
  { id = "e_Patrol", behavior = "Patrol" },
  { id = "e_stationary", behavior = "stationary" },
  { id = "e_Stationary", behavior = "Stationary" },
  { id = "e_zigzag", behavior = "zigzag" },
  { id = "e_Zigzag", behavior = "ZigZag" },
  { id = "e_dive", behavior = "divebomb" },
  { id = "e_Dive", behavior = "DiveBomb" }
])";

    auto file = makeTempFile("enemies_var.toml", toml);
    EXPECT_TRUE(reg.loadEnemies(file.string()));

    auto check = [&](const std::string& id, AIBehavior expected) {
        auto opt = reg.getEnemy(id);
        ASSERT_TRUE(opt.has_value());
        EXPECT_EQ(opt->get().behavior, expected);
    };

    check("e_move", AIBehavior::MoveLeft);
    check("e_Move", AIBehavior::MoveLeft);
    check("e_sine", AIBehavior::SineWave);
    check("e_Sine", AIBehavior::SineWave);
    check("e_chase", AIBehavior::Chase);
    check("e_Chase", AIBehavior::Chase);
    check("e_patrol", AIBehavior::Patrol);
    check("e_Patrol", AIBehavior::Patrol);
    check("e_stationary", AIBehavior::Stationary);
    check("e_Stationary", AIBehavior::Stationary);
    check("e_zigzag", AIBehavior::ZigZag);
    check("e_Zigzag", AIBehavior::ZigZag);
    check("e_dive", AIBehavior::DiveBomb);
    check("e_Dive", AIBehavior::DiveBomb);
}

TEST(EntityConfigBranches, PowerUpEffectVariants) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    const std::string toml = R"(powerup = [
  { id = "p_health", effect = "health" },
  { id = "p_Health", effect = "Health" },
  { id = "p_speed", effect = "speed_boost" },
  { id = "p_Speed", effect = "SpeedBoost" },
  { id = "p_weapon", effect = "weapon_upgrade" },
  { id = "p_shield", effect = "shield" },
  { id = "p_boost1", effect = "extra_life" },
  { id = "p_boost2", effect = "HealthBoost" },
  { id = "p_boost3", effect = "health_boost" }
])";

    auto file = makeTempFile("powerups_var.toml", toml);
    EXPECT_TRUE(reg.loadPowerUps(file.string()));

    auto check = [&](const std::string& id, PowerUpConfig::EffectType expected) {
        auto opt = reg.getPowerUp(id);
        ASSERT_TRUE(opt.has_value());
        EXPECT_EQ(opt->get().effect, expected);
    };

    check("p_health", PowerUpConfig::EffectType::Health);
    check("p_Health", PowerUpConfig::EffectType::Health);
    check("p_speed", PowerUpConfig::EffectType::SpeedBoost);
    check("p_Speed", PowerUpConfig::EffectType::SpeedBoost);
    check("p_weapon", PowerUpConfig::EffectType::WeaponUpgrade);
    check("p_shield", PowerUpConfig::EffectType::Shield);
    check("p_boost1", PowerUpConfig::EffectType::HealthBoost);
    check("p_boost2", PowerUpConfig::EffectType::HealthBoost);
    check("p_boost3", PowerUpConfig::EffectType::HealthBoost);
}

TEST(EntityConfigBranches, LoadEnemiesMalformedReturnsFalse) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    // Intentionally malformed TOML (unterminated array)
    const std::string bad = "enemy = [ { id = \"bad\", name = \"bad\"";
    auto file = makeTempFile("enemies_malformed.toml", bad);
    EXPECT_FALSE(reg.loadEnemies(file.string()));
}

TEST(EntityConfigBranches, LoadPowerUpsMalformedReturnsFalse) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    const std::string bad = "powerup = [ { id = \"bad\", effect = \"health\""; // unterminated
    auto file = makeTempFile("powerups_malformed.toml", bad);
    EXPECT_FALSE(reg.loadPowerUps(file.string()));
}

TEST(EntityConfigBranches, LoadLevelInvalidReturnsFalse) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    // Level with no waves (invalid)
    const std::string toml = R"([level]
id = ""
name = "NoWaves"
background = "bg.png"
)";
    auto file = makeTempFile("level_invalid.toml", toml);
    EXPECT_FALSE(reg.loadLevel(file.string()));
}

TEST(EntityConfigBranches, UnknownBehaviorAndEffectFallbacks) {
    EntityConfigRegistry& reg = EntityConfigRegistry::getInstance();
    reg.clear();

    const std::string toml_e = R"(enemy = [ { id = "e_unk", behavior = "unknown_behavior" } ])";
    auto filee = makeTempFile("enemies_unknown.toml", toml_e);
    EXPECT_TRUE(reg.loadEnemies(filee.string()));
    auto eopt = reg.getEnemy("e_unk");
    ASSERT_TRUE(eopt.has_value());
    EXPECT_EQ(eopt->get().behavior, AIBehavior::MoveLeft); // fallback

    const std::string toml_p = R"(powerup = [ { id = "p_unk", effect = "unknown_effect" } ])";
    auto filep = makeTempFile("powerups_unknown.toml", toml_p);
    EXPECT_TRUE(reg.loadPowerUps(filep.string()));
    auto popt = reg.getPowerUp("p_unk");
    ASSERT_TRUE(popt.has_value());
    EXPECT_EQ(popt->get().effect, PowerUpConfig::EffectType::Health); // fallback
}

// End of file
