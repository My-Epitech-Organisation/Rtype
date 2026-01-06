/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for WaveManager and DataDrivenSpawnerSystem
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "games/rtype/server/Systems/WaveManager/WaveManager.hpp"
#include "games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"

namespace rtype::games::rtype::server {
namespace {

class WaveManagerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Create temporary test directory
        _testDir = std::filesystem::temp_directory_path() / "rtype_wave_test";
        std::filesystem::create_directories(_testDir / "levels");

        // Load enemy configs for testing
        auto& registry = shared::EntityConfigRegistry::getInstance();
        registry.clear();

        // Create a temporary enemies.toml for testing
        std::ofstream enemiesFile(_testDir / "enemies.toml");
        enemiesFile << R"(
[[enemy]]
id = "basic"
name = "Basic Enemy"
sprite_sheet = "assets/sprites/enemies/basic.png"
health = 50
damage = 10
score_value = 100
behavior = "move_left"
speed = 100.0
hitbox_width = 32.0
hitbox_height = 32.0
can_shoot = false

[[enemy]]
id = "shooter"
name = "Shooter Enemy"
sprite_sheet = "assets/sprites/enemies/shooter.png"
health = 100
damage = 20
score_value = 200
behavior = "stationary"
speed = 0.0
hitbox_width = 48.0
hitbox_height = 48.0
can_shoot = true
fire_rate = 1.5
projectile_type = "enemy_bullet"
)";
        enemiesFile.close();
        registry.loadEnemies((_testDir / "enemies.toml").string());
    }

    void TearDown() override {
        std::filesystem::remove_all(_testDir);
        shared::EntityConfigRegistry::getInstance().clear();
    }

    void createTestLevel(const std::string& filename,
                         const std::string& content) {
        std::ofstream file(_testDir / "levels" / filename);
        file << content;
        file.close();

        // Load the level into registry
        shared::EntityConfigRegistry::getInstance().loadLevel(
            (_testDir / "levels" / filename).string());
    }

    std::filesystem::path _testDir;
};

// =============================================================================
// WaveManager Loading Tests
// =============================================================================

TEST_F(WaveManagerTest, LoadValidLevel) {
    createTestLevel("test_level.toml", R"(
[level]
id = "test_level"
name = "Test Level"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.5

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 3
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("test_level"));
    EXPECT_EQ(manager.getLevelId(), "test_level");
    EXPECT_EQ(manager.getLevelName(), "Test Level");
    EXPECT_EQ(manager.getTotalWaves(), 1);
    EXPECT_EQ(manager.getState(), WaveState::NotStarted);
}

TEST_F(WaveManagerTest, LoadNonExistentLevel) {
    WaveManager manager;
    ASSERT_FALSE(manager.loadLevel("nonexistent_level"));
    EXPECT_EQ(manager.getState(), WaveState::Failed);
    EXPECT_FALSE(manager.getLastError().empty());
}

TEST_F(WaveManagerTest, LoadLevelWithBoss) {
    createTestLevel("boss_level.toml", R"(
[level]
id = "boss_level"
name = "Boss Level"
background = "test.png"
scroll_speed = 50.0
boss = "boss_1"

[[wave]]
number = 1
spawn_delay = 0.5

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("boss_level"));
    EXPECT_TRUE(manager.getBossId().has_value());
    EXPECT_EQ(*manager.getBossId(), "boss_1");
}

TEST_F(WaveManagerTest, LoadLevelWithoutBoss) {
    createTestLevel("no_boss.toml", R"(
[level]
id = "no_boss"
name = "No Boss Level"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.5

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("no_boss"));
    EXPECT_FALSE(manager.getBossId().has_value());
}

// =============================================================================
// WaveManager Wave Progression Tests
// =============================================================================

TEST_F(WaveManagerTest, StartLevel) {
    createTestLevel("start_test.toml", R"(
[level]
id = "start_test"
name = "Start Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.5

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("start_test"));
    manager.start();
    EXPECT_EQ(manager.getState(), WaveState::InProgress);
    EXPECT_EQ(manager.getCurrentWave(), 1);
}

TEST_F(WaveManagerTest, SpawnRequestsGenerated) {
    createTestLevel("spawn_test.toml", R"(
[level]
id = "spawn_test"
name = "Spawn Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("spawn_test"));
    manager.start();

    // First update should generate spawn request
    auto spawns = manager.update(0.1F, 0);
    EXPECT_EQ(spawns.size(), 1);
    if (!spawns.empty()) {
        EXPECT_EQ(spawns[0].enemyId, "basic");
        EXPECT_FLOAT_EQ(spawns[0].x, 800.0F);
        EXPECT_FLOAT_EQ(spawns[0].y, 300.0F);
    }
}

TEST_F(WaveManagerTest, WaveCompletionWithClear) {
    createTestLevel("clear_test.toml", R"(
[level]
id = "clear_test"
name = "Clear Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 1

[[wave]]
number = 2
spawn_delay = 0.0

[[wave.spawn]]
enemy = "shooter"
x = 800.0
y = 400.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    manager.setWaitForClear(true);
    manager.setWaveTransitionDelay(0.0F);
    ASSERT_TRUE(manager.loadLevel("clear_test"));
    manager.start();

    // Spawn first wave enemy
    auto spawns = manager.update(0.1F, 0);
    EXPECT_EQ(spawns.size(), 1);

    // Wave should complete after all spawns done
    spawns = manager.update(0.1F, 1);  // 1 enemy alive
    EXPECT_EQ(manager.getState(), WaveState::WaveComplete);

    // Should not advance while enemies alive
    spawns = manager.update(0.5F, 1);
    EXPECT_EQ(manager.getCurrentWave(), 1);

    // Should advance when enemies cleared
    spawns = manager.update(0.1F, 0);
    EXPECT_EQ(manager.getCurrentWave(), 2);
    EXPECT_EQ(manager.getState(), WaveState::InProgress);
}

TEST_F(WaveManagerTest, AllWavesComplete) {
    createTestLevel("complete_test.toml", R"(
[level]
id = "complete_test"
name = "Complete Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    manager.setWaitForClear(false);
    manager.setWaveTransitionDelay(0.0F);
    ASSERT_TRUE(manager.loadLevel("complete_test"));
    manager.start();

    // Spawn all enemies
    auto spawns = manager.update(0.1F, 0);
    EXPECT_EQ(spawns.size(), 1);

    // Wave completes after all spawns done
    spawns = manager.update(0.1F, 0);  // Wave becomes WaveComplete

    // Advance to next (which doesn't exist, so AllComplete)
    spawns = manager.update(0.1F, 0);

    EXPECT_TRUE(manager.isAllWavesComplete());
    EXPECT_EQ(manager.getState(), WaveState::AllComplete);
}

TEST_F(WaveManagerTest, Reset) {
    createTestLevel("reset_test.toml", R"(
[level]
id = "reset_test"
name = "Reset Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("reset_test"));
    manager.start();
    manager.update(0.1F, 0);

    manager.reset();
    EXPECT_EQ(manager.getState(), WaveState::NotStarted);
    EXPECT_EQ(manager.getCurrentWave(), 1);
}

// =============================================================================
// Edge Cases
// =============================================================================

TEST_F(WaveManagerTest, EmptyWaveSpawns) {
    // Create level with wave but skip unknown enemy
    createTestLevel("empty_wave.toml", R"(
[level]
id = "empty_wave"
name = "Empty Wave Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "unknown_enemy_type"
x = 800.0
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("empty_wave"));
    manager.start();

    // Should not crash, just skip unknown enemy
    auto spawns = manager.update(0.1F, 0);
    EXPECT_EQ(spawns.size(), 0);  // Unknown enemy skipped
}

TEST_F(WaveManagerTest, ZeroSpawnDelay) {
    createTestLevel("zero_delay.toml", R"(
[level]
id = "zero_delay"
name = "Zero Delay Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 3
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("zero_delay"));
    manager.start();

    // All enemies should spawn immediately with zero delay
    auto spawns = manager.update(0.01F, 0);
    EXPECT_GE(spawns.size(), 1);  // At least first enemy spawns immediately
}

TEST_F(WaveManagerTest, MultipleSpawnEntries) {
    createTestLevel("multi_spawn.toml", R"(
[level]
id = "multi_spawn"
name = "Multi Spawn Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 100.0
delay = 0.0
count = 1

[[wave.spawn]]
enemy = "shooter"
x = 800.0
y = 500.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("multi_spawn"));
    manager.start();

    auto spawns = manager.update(0.01F, 0);
    EXPECT_GE(spawns.size(), 2);  // Both spawn entries should trigger
}

TEST_F(WaveManagerTest, StartWithoutLoading) {
    WaveManager manager;
    manager.start();  // Should not crash
    EXPECT_NE(manager.getState(), WaveState::InProgress);
}

TEST_F(WaveManagerTest, UpdateWithoutStarting) {
    createTestLevel("no_start.toml", R"(
[level]
id = "no_start"
name = "No Start Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
x = 800.0
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("no_start"));
    
    // Don't call start()
    auto spawns = manager.update(0.1F, 0);
    EXPECT_EQ(spawns.size(), 0);  // Nothing should spawn
    EXPECT_EQ(manager.getState(), WaveState::NotStarted);
}

}  // namespace
}  // namespace rtype::games::rtype::server
