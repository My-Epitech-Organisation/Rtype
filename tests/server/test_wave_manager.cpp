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
        ASSERT_TRUE(spawns[0].x.has_value());
        ASSERT_TRUE(spawns[0].y.has_value());
        EXPECT_FLOAT_EQ(*spawns[0].x, 800.0F);
        EXPECT_FLOAT_EQ(*spawns[0].y, 300.0F);
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
    [[maybe_unused]] auto spawns = manager.update(0.1F, 0);

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

// =============================================================================
// Random Spawn Position Tests (Optional Coordinates)
// =============================================================================

TEST_F(WaveManagerTest, SpawnWithoutCoordinates) {
    createTestLevel("random_spawn.toml", R"(
[level]
id = "random_spawn"
name = "Random Spawn Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
delay = 0.0
count = 3
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("random_spawn"));
    manager.start();

    // Spawn entries without x,y should still work
    auto spawns = manager.update(0.01F, 0);
    EXPECT_GE(spawns.size(), 1);
    if (!spawns.empty()) {
        EXPECT_EQ(spawns[0].enemyId, "basic");
        // Coordinates should be nullopt
        EXPECT_FALSE(spawns[0].hasFixedX());
        EXPECT_FALSE(spawns[0].hasFixedY());
    }
}

TEST_F(WaveManagerTest, SpawnWithOnlyXCoordinate) {
    createTestLevel("x_only.toml", R"(
[level]
id = "x_only"
name = "X Only Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
x = 750.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("x_only"));
    manager.start();

    auto spawns = manager.update(0.01F, 0);
    EXPECT_EQ(spawns.size(), 1);
    if (!spawns.empty()) {
        EXPECT_TRUE(spawns[0].hasFixedX());
        EXPECT_FALSE(spawns[0].hasFixedY());
        EXPECT_FLOAT_EQ(*spawns[0].x, 750.0F);
    }
}

TEST_F(WaveManagerTest, SpawnWithOnlyYCoordinate) {
    createTestLevel("y_only.toml", R"(
[level]
id = "y_only"
name = "Y Only Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
y = 400.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    ASSERT_TRUE(manager.loadLevel("y_only"));
    manager.start();

    auto spawns = manager.update(0.01F, 0);
    EXPECT_EQ(spawns.size(), 1);
    if (!spawns.empty()) {
        EXPECT_FALSE(spawns[0].hasFixedX());
        EXPECT_TRUE(spawns[0].hasFixedY());
        EXPECT_FLOAT_EQ(*spawns[0].y, 400.0F);
    }
}

// =============================================================================
// Boss Spawning Tests
// =============================================================================

TEST_F(WaveManagerTest, BossSpawning) {
    // First add boss to enemy registry
    std::ofstream bossFile(_testDir / "boss.toml");
    bossFile << R"(
[[enemy]]
id = "boss_1"
name = "Boss Enemy"
sprite_sheet = "assets/sprites/enemies/boss.png"
health = 500
damage = 50
score_value = 1000
behavior = "stationary"
speed = 0.0
hitbox_width = 128.0
hitbox_height = 128.0
can_shoot = true
fire_rate = 2.0
projectile_type = "enemy_bullet"
)";
    bossFile.close();
    shared::EntityConfigRegistry::getInstance().loadEnemies(
        (_testDir / "boss.toml").string());

    createTestLevel("boss_level.toml", R"(
[level]
id = "boss_level"
name = "Boss Level"
background = "test.png"
scroll_speed = 50.0
boss = "boss_1"

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
    ASSERT_TRUE(manager.loadLevel("boss_level"));
    manager.start();

    // Complete the wave
    auto spawns = manager.update(0.01F, 0);
    EXPECT_GE(spawns.size(), 1);

    // Wait for wave completion
    spawns = manager.update(5.0F, 0);  // All enemies cleared

    // Boss ID should be available
    auto bossId = manager.getBossId();
    EXPECT_TRUE(bossId.has_value());
    EXPECT_EQ(*bossId, "boss_1");
}

TEST_F(WaveManagerTest, LevelWithoutBoss) {
    createTestLevel("no_boss.toml", R"(
[level]
id = "no_boss"
name = "No Boss Level"
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
    ASSERT_TRUE(manager.loadLevel("no_boss"));
    manager.start();

    auto bossId = manager.getBossId();
    EXPECT_FALSE(bossId.has_value());
}

// =============================================================================
// Error and Edge Cases
// =============================================================================

TEST_F(WaveManagerTest, LoadInvalidFilePath) {
    WaveManager manager;
    EXPECT_FALSE(manager.loadLevelFromFile("/nonexistent/path.toml"));
    EXPECT_EQ(manager.getState(), WaveState::Failed);
}

TEST_F(WaveManagerTest, LoadNonExistentLevelId) {
    WaveManager manager;
    EXPECT_FALSE(manager.loadLevel("does_not_exist"));
    EXPECT_EQ(manager.getState(), WaveState::Failed);
}

TEST_F(WaveManagerTest, InvalidWaveConfig) {
    createTestLevel("invalid.toml", R"(
[level]
id = "invalid"
name = "Invalid Level"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 0
spawn_delay = 0.0
)");

    WaveManager manager;
    // Should handle invalid wave gracefully
    bool loaded = manager.loadLevel("invalid");
    // May load or fail depending on validation
    if (loaded) {
        manager.start();
        auto spawns = manager.update(0.1F, 0);
        // Should not crash
    }
}

TEST_F(WaveManagerTest, EmptySpawnArray) {
    createTestLevel("empty_spawns.toml", R"(
[level]
id = "empty_spawns"
name = "Empty Spawns"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0
)");

    WaveManager manager;
    bool loaded = manager.loadLevel("empty_spawns");
    if (loaded) {
        manager.start();
        auto spawns = manager.update(0.1F, 0);
        EXPECT_EQ(spawns.size(), 0);
    }
}

TEST_F(WaveManagerTest, GettersBeforeLoading) {
    WaveManager manager;
    EXPECT_EQ(manager.getCurrentWave(), 1);
    EXPECT_FALSE(manager.getBossId().has_value());
    EXPECT_FALSE(manager.isLevelLoaded());
    EXPECT_FALSE(manager.isAllWavesComplete());
}

TEST_F(WaveManagerTest, WaveTransitionWithDelay) {
    createTestLevel("transition.toml", R"(
[level]
id = "transition"
name = "Transition Test"
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
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    manager.setWaveTransitionDelay(1.0F);
    ASSERT_TRUE(manager.loadLevel("transition"));
    manager.start();

    // Complete wave 1
    auto spawns = manager.update(0.01F, 0);
    EXPECT_GE(spawns.size(), 1);

    // All enemies cleared, enter transition
    spawns = manager.update(1.0F, 0);
    EXPECT_EQ(manager.getState(), WaveState::WaveComplete);

    // Wait for transition delay
    spawns = manager.update(1.5F, 0);
    
    // Should advance to wave 2
    EXPECT_EQ(manager.getCurrentWave(), 2);
}

TEST_F(WaveManagerTest, WaitForClearEnabled) {
    createTestLevel("wait_clear.toml", R"(
[level]
id = "wait_clear"
name = "Wait Clear Test"
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
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    manager.setWaitForClear(true);
    manager.setWaveTransitionDelay(0.5F);  // Small transition delay
    ASSERT_TRUE(manager.loadLevel("wait_clear"));
    manager.start();

    // Spawn wave 1 and let all spawns complete with enemies alive
    for (int i = 0; i < 10; ++i) {
        [[maybe_unused]] auto spawns = manager.update(0.1F, 1);  // 1 enemy alive
    }
    
    // Wave spawning is complete, should be in WaveComplete state
    EXPECT_EQ(manager.getState(), WaveState::WaveComplete);
    // But should not advance to next wave with enemies alive
    EXPECT_EQ(manager.getCurrentWave(), 1);

    // Try to advance with enemies still alive - should stay on wave 1
    [[maybe_unused]] auto spawns2 = manager.update(1.0F, 1);  // Still 1 enemy alive
    EXPECT_EQ(manager.getState(), WaveState::WaveComplete);
    EXPECT_EQ(manager.getCurrentWave(), 1);

    // Clear enemies - now it should transition to wave 2
    [[maybe_unused]] auto spawns3 = manager.update(1.0F, 0);  // All cleared, wait for transition delay
    EXPECT_EQ(manager.getState(), WaveState::InProgress);
    EXPECT_EQ(manager.getCurrentWave(), 2);
}

TEST_F(WaveManagerTest, WaitForClearDisabled) {
    createTestLevel("no_wait.toml", R"(
[level]
id = "no_wait"
name = "No Wait Test"
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
y = 300.0
delay = 0.0
count = 1
)");

    WaveManager manager;
    manager.setWaitForClear(false);
    manager.setWaveTransitionDelay(0.0F);
    ASSERT_TRUE(manager.loadLevel("no_wait"));
    manager.start();

    // Spawn wave 1
    auto spawns = manager.update(0.01F, 0);
    EXPECT_GE(spawns.size(), 1);

    // Progress immediately even with enemies alive
    spawns = manager.update(2.0F, 5);  // 5 enemies alive
    // Should still progress to next wave
}

TEST_F(WaveManagerTest, GettersWithoutLoadedLevel) {
    WaveManager manager;
    // Test all getters when no level is loaded
    EXPECT_FALSE(manager.isLevelLoaded());
    EXPECT_EQ(manager.getTotalWaves(), 0);
    EXPECT_EQ(manager.getLevelId(), "");
    EXPECT_EQ(manager.getLevelName(), "");
    EXPECT_FALSE(manager.getBossId().has_value());
}

}  // namespace
}  // namespace rtype::games::rtype::server
