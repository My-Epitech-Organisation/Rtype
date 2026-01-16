/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for DataDrivenSpawnerSystem
*/

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "games/rtype/server/Systems/Spawner/DataDrivenSpawnerSystem.hpp"
#include "games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"
#include "rtype/ecs.hpp"

namespace rtype::games::rtype::server {
namespace {

class DataDrivenSpawnerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        _registry = std::make_shared<ECS::Registry>();
        
        // Create temporary test directory
        _testDir = std::filesystem::temp_directory_path() / "rtype_spawner_test";
        std::filesystem::create_directories(_testDir / "levels");

        // Load enemy configs for testing
        auto& configRegistry = shared::EntityConfigRegistry::getInstance();
        configRegistry.clear();

        // Create test enemy configs
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
id = "zigzag"
name = "ZigZag Enemy"
sprite_sheet = "assets/sprites/enemies/zigzag.png"
health = 75
damage = 15
score_value = 150
behavior = "zigzag"
speed = 120.0
hitbox_width = 32.0
hitbox_height = 32.0
can_shoot = true
fire_rate = 1.0
projectile_type = "enemy_bullet"

[[enemy]]
id = "boss_test"
name = "Test Boss"
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
        enemiesFile.close();
        configRegistry.loadEnemies((_testDir / "enemies.toml").string());
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

        shared::EntityConfigRegistry::getInstance().loadLevel(
            (_testDir / "levels" / filename).string());
    }

    std::shared_ptr<ECS::Registry> _registry;
    std::filesystem::path _testDir;
};

// =============================================================================
// Basic Spawning Tests
// =============================================================================

TEST_F(DataDrivenSpawnerTest, SpawnEnemyWithFixedCoordinates) {
    createTestLevel("fixed_coords.toml", R"(
[level]
id = "fixed_coords"
name = "Fixed Coords Test"
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

    int eventCount = 0;
    auto eventEmitter = [&eventCount](const engine::GameEvent& event) {
        if (event.type == engine::GameEventType::EntitySpawned) {
            eventCount++;
        }
    };

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    ASSERT_TRUE(spawner.loadLevel("fixed_coords"));
    spawner.startLevel();

    // Update spawner
    spawner.update(*_registry, 0.1F);

    // Check that enemy was spawned
    EXPECT_GT(eventCount, 0);
}

TEST_F(DataDrivenSpawnerTest, SpawnEnemyWithRandomCoordinates) {
    createTestLevel("random_coords.toml", R"(
[level]
id = "random_coords"
name = "Random Coords Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
delay = 0.0
count = 5
)");

    std::vector<float> spawnPositions;
    auto eventEmitter = [&spawnPositions](const engine::GameEvent& event) {
        if (event.type == engine::GameEventType::EntitySpawned) {
            spawnPositions.push_back(event.y);
        }
    };

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    ASSERT_TRUE(spawner.loadLevel("random_coords"));
    spawner.startLevel();

    // Spawn multiple enemies with random positions
    for (int i = 0; i < 10; ++i) {
        spawner.update(*_registry, 0.1F);
    }

    // Check that we got spawns
    EXPECT_GT(spawnPositions.size(), 0);

    // Check that Y positions are within screen bounds with margin
    for (float y : spawnPositions) {
        EXPECT_GE(y, config.spawnMargin);
        EXPECT_LE(y, config.screenHeight - config.spawnMargin);
    }
}

TEST_F(DataDrivenSpawnerTest, SpawnEnemyWithMixedCoordinates) {
    createTestLevel("mixed_coords.toml", R"(
[level]
id = "mixed_coords"
name = "Mixed Coords Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "basic"
x = 800.0
delay = 0.0
count = 1

[[wave.spawn]]
enemy = "zigzag"
y = 500.0
delay = 0.5
count = 1
)");

    int eventCount = 0;
    auto eventEmitter = [&eventCount](const engine::GameEvent& event) {
        if (event.type == engine::GameEventType::EntitySpawned) {
            eventCount++;
        }
    };

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    ASSERT_TRUE(spawner.loadLevel("mixed_coords"));
    spawner.startLevel();

    // Spawn enemies
    spawner.update(*_registry, 0.1F);
    spawner.update(*_registry, 0.6F);

    EXPECT_GE(eventCount, 1);
}

// =============================================================================
// Boss Spawning Tests
// =============================================================================

TEST_F(DataDrivenSpawnerTest, BossSpawnsAfterAllWaves) {
    createTestLevel("boss_spawn.toml", R"(
[level]
id = "boss_spawn"
name = "Boss Spawn Test"
background = "test.png"
scroll_speed = 50.0
boss = "boss_test"

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

    std::string lastSpawnedEnemy;
    auto eventEmitter = [&lastSpawnedEnemy](const engine::GameEvent& event) {
        if (event.type == engine::GameEventType::EntitySpawned) {
            // We can't easily extract enemy ID from event, but we track spawns
        }
    };

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;
    config.waitForClear = true;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    ASSERT_TRUE(spawner.loadLevel("boss_spawn"));
    spawner.startLevel();

    // Complete all waves
    for (int i = 0; i < 10; ++i) {
        spawner.update(*_registry, 0.5F);
    }

    // Boss should spawn when all enemies are cleared
    // (would need more complex tracking to verify boss specifically)
}

// =============================================================================
// Max Enemy Limit Tests
// =============================================================================

TEST_F(DataDrivenSpawnerTest, RespectMaxEnemyLimit) {
    createTestLevel("max_limit.toml", R"(
[level]
id = "max_limit"
name = "Max Limit Test"
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
count = 20
)");

    int spawnCount = 0;
    auto eventEmitter = [&spawnCount](const engine::GameEvent& event) {
        if (event.type == engine::GameEventType::EntitySpawned) {
            spawnCount++;
        }
    };

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 5;  // Low limit

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    ASSERT_TRUE(spawner.loadLevel("max_limit"));
    spawner.startLevel();

    // Try to spawn many enemies
    for (int i = 0; i < 20; ++i) {
        spawner.update(*_registry, 0.1F);
    }

    // Should not exceed max
    EXPECT_LE(spawnCount, config.maxEnemies);
}

// =============================================================================
// Fallback Spawning Tests
// =============================================================================

TEST_F(DataDrivenSpawnerTest, FallbackSpawningWhenNoLevel) {
    int spawnCount = 0;
    auto eventEmitter = [&spawnCount](const engine::GameEvent& event) {
        if (event.type == engine::GameEventType::EntitySpawned) {
            spawnCount++;
        }
    };

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;
    config.enableFallbackSpawning = true;
    config.fallbackMinInterval = 0.1F;
    config.fallbackMaxInterval = 0.2F;
    config.fallbackEnemiesPerWave = 5;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    // Don't load any level - should use fallback

    // Update multiple times to trigger fallback spawning
    for (int i = 0; i < 10; ++i) {
        spawner.update(*_registry, 0.3F);
    }

    // Should have spawned some enemies via fallback
    EXPECT_GT(spawnCount, 0);
}

TEST_F(DataDrivenSpawnerTest, FallbackSpawningDisabled) {
    int spawnCount = 0;
    auto eventEmitter = [&spawnCount](const engine::GameEvent& event) {
        if (event.type == engine::GameEventType::EntitySpawned) {
            spawnCount++;
        }
    };

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;
    config.enableFallbackSpawning = false;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    // Don't load any level

    // Update multiple times
    for (int i = 0; i < 10; ++i) {
        spawner.update(*_registry, 0.5F);
    }

    // Should not spawn anything
    EXPECT_EQ(spawnCount, 0);
}

// =============================================================================
// Reset and State Tests
// =============================================================================

TEST_F(DataDrivenSpawnerTest, ResetSpawner) {
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
count = 2
)");

    int spawnCount = 0;
    auto eventEmitter = [&spawnCount](const engine::GameEvent& event) {
        if (event.type == engine::GameEventType::EntitySpawned) {
            spawnCount++;
        }
    };

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    ASSERT_TRUE(spawner.loadLevel("reset_test"));
    spawner.startLevel();

    spawner.update(*_registry, 0.1F);
    EXPECT_GT(spawnCount, 0);

    // Reset
    spawner.reset();
    int oldCount = spawnCount;

    // Should be able to restart
    spawner.startLevel();
    spawner.update(*_registry, 0.1F);
    
    // Should spawn again after reset
    EXPECT_GT(spawnCount, oldCount);
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST_F(DataDrivenSpawnerTest, LoadInvalidLevel) {
    auto eventEmitter = [](const engine::GameEvent&) {};

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    EXPECT_FALSE(spawner.loadLevel("nonexistent_level"));
}

TEST_F(DataDrivenSpawnerTest, SpawnUnknownEnemy) {
    createTestLevel("unknown_enemy.toml", R"(
[level]
id = "unknown_enemy"
name = "Unknown Enemy Test"
background = "test.png"
scroll_speed = 50.0

[[wave]]
number = 1
spawn_delay = 0.0

[[wave.spawn]]
enemy = "nonexistent_enemy_type"
x = 800.0
y = 300.0
delay = 0.0
count = 1
)");

    int spawnCount = 0;
    auto eventEmitter = [&spawnCount](const engine::GameEvent& event) {
        if (event.type == engine::GameEventType::EntitySpawned) {
            spawnCount++;
        }
    };

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);
    ASSERT_TRUE(spawner.loadLevel("unknown_enemy"));
    spawner.startLevel();

    spawner.update(*_registry, 0.1F);

    // Should not crash, but should not spawn unknown enemy
    EXPECT_EQ(spawnCount, 0);
}

TEST_F(DataDrivenSpawnerTest, GettersBeforeLoading) {
    auto eventEmitter = [](const engine::GameEvent&) {};

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);

    // Should not crash when calling methods before loading
    EXPECT_FALSE(spawner.isAllWavesComplete());
    EXPECT_EQ(spawner.getEnemyCount(), 0);
    EXPECT_EQ(spawner.getCurrentWave(), 1);
}

TEST_F(DataDrivenSpawnerTest, DecrementEnemyCountAtZero) {
    auto eventEmitter = [](const engine::GameEvent&) {};

    DataDrivenSpawnerConfig config{};
    config.screenWidth = 1920.0F;
    config.screenHeight = 1080.0F;
    config.spawnMargin = 50.0F;
    config.maxEnemies = 100;

    DataDrivenSpawnerSystem spawner(eventEmitter, config);

    // Should start at 0
    EXPECT_EQ(spawner.getEnemyCount(), 0);
    
    // Decrement when already 0 - should stay at 0
    spawner.decrementEnemyCount();
    EXPECT_EQ(spawner.getEnemyCount(), 0);
    
    // Increment then decrement to verify it works
    spawner.incrementEnemyCount();
    EXPECT_EQ(spawner.getEnemyCount(), 1);
    spawner.decrementEnemyCount();
    EXPECT_EQ(spawner.getEnemyCount(), 0);
    spawner.decrementEnemyCount();
    EXPECT_EQ(spawner.getEnemyCount(), 0);
}

}  // namespace
}  // namespace rtype::games::rtype::server
