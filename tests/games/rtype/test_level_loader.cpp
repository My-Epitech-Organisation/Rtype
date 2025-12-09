/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_level_loader - Unit tests for Level Loading System
*/

#include <gtest/gtest.h>

#include <filesystem>

#include "games/rtype/shared/Components.hpp"
#include "games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"
#include "games/rtype/shared/Systems/Systems.hpp"

namespace rtype::games::rtype::shared {

// =============================================================================
// MapElementConfig Tests
// =============================================================================

class MapElementConfigTest : public ::testing::Test {
   protected:
    void SetUp() override {}
};

TEST_F(MapElementConfigTest, DefaultValues) {
    MapElementConfig config;
    EXPECT_EQ(config.type, MapElementType::Obstacle);
    EXPECT_EQ(config.x, 0.0F);
    EXPECT_EQ(config.y, 0.0F);
    EXPECT_EQ(config.width, 32.0F);
    EXPECT_EQ(config.height, 32.0F);
    EXPECT_EQ(config.health, 0);
    EXPECT_FALSE(config.isValid());  // Empty ID
}

TEST_F(MapElementConfigTest, ValidObstacle) {
    MapElementConfig config;
    config.id = "test_obstacle";
    config.type = MapElementType::Obstacle;
    config.width = 64.0F;
    config.height = 32.0F;

    EXPECT_TRUE(config.isValid());
    EXPECT_TRUE(config.hasCollision());
    EXPECT_FALSE(config.isDestroyable());
    EXPECT_EQ(config.getHitboxWidth(), 64.0F);
    EXPECT_EQ(config.getHitboxHeight(), 32.0F);
}

TEST_F(MapElementConfigTest, CustomHitbox) {
    MapElementConfig config;
    config.id = "test";
    config.width = 64.0F;
    config.height = 64.0F;
    config.hitboxWidth = 48.0F;
    config.hitboxHeight = 32.0F;

    EXPECT_EQ(config.getHitboxWidth(), 48.0F);
    EXPECT_EQ(config.getHitboxHeight(), 32.0F);
}

TEST_F(MapElementConfigTest, DestroyableTile) {
    MapElementConfig config;
    config.id = "breakable";
    config.type = MapElementType::DestroyableTile;
    config.health = 5;
    config.scoreValue = 100;

    EXPECT_TRUE(config.isValid());
    EXPECT_TRUE(config.hasCollision());
    EXPECT_TRUE(config.isDestroyable());
}

TEST_F(MapElementConfigTest, Decoration_NoCollision) {
    MapElementConfig config;
    config.id = "decoration";
    config.type = MapElementType::Decoration;

    EXPECT_TRUE(config.isValid());
    EXPECT_FALSE(config.hasCollision());
    EXPECT_FALSE(config.isDestroyable());
}

TEST_F(MapElementConfigTest, StringToMapElementType) {
    EXPECT_EQ(stringToMapElementType("obstacle"), MapElementType::Obstacle);
    EXPECT_EQ(stringToMapElementType("Obstacle"), MapElementType::Obstacle);
    EXPECT_EQ(stringToMapElementType("destroyable"),
              MapElementType::DestroyableTile);
    EXPECT_EQ(stringToMapElementType("DestroyableTile"),
              MapElementType::DestroyableTile);
    EXPECT_EQ(stringToMapElementType("decoration"), MapElementType::Decoration);
    EXPECT_EQ(stringToMapElementType("unknown"),
              MapElementType::Obstacle);  // Default
}

// =============================================================================
// StarfieldLayerConfig Tests
// =============================================================================

TEST(StarfieldLayerConfigTest, DefaultValues) {
    StarfieldLayerConfig config;
    EXPECT_EQ(config.scrollFactor, 0.0F);
    EXPECT_EQ(config.zIndex, 0);
    EXPECT_TRUE(config.isRepeating);
    EXPECT_EQ(config.scale, 1.0F);
    EXPECT_FALSE(config.isValid());  // Empty texture
}

TEST(StarfieldLayerConfigTest, ValidConfig) {
    StarfieldLayerConfig config;
    config.texturePath = "assets/stars.png";
    config.scrollFactor = 0.5F;
    config.zIndex = -10;

    EXPECT_TRUE(config.isValid());
}

// =============================================================================
// MapConfig Tests
// =============================================================================

TEST(MapConfigTest, DefaultValues) {
    MapConfig config;
    EXPECT_EQ(config.levelWidth, 10000.0F);
    EXPECT_EQ(config.viewportWidth, 1920.0F);
    EXPECT_EQ(config.viewportHeight, 1080.0F);
    EXPECT_EQ(config.getTotalElements(), 0);
    EXPECT_FALSE(config.hasContent());
}

TEST(MapConfigTest, HasContent) {
    MapConfig config;
    EXPECT_FALSE(config.hasContent());

    MapElementConfig obstacle;
    obstacle.id = "test";
    config.obstacles.push_back(obstacle);

    EXPECT_TRUE(config.hasContent());
    EXPECT_EQ(config.getTotalElements(), 1);
}

// =============================================================================
// LevelScrollState Tests
// =============================================================================

class LevelScrollStateTest : public ::testing::Test {
   protected:
    LevelScrollState state;

    void SetUp() override {
        state.scrollOffset = 0.0F;
        state.scrollSpeed = 100.0F;
        state.levelWidth = 5000.0F;
        state.viewportWidth = 1920.0F;
        state.spawnAheadDistance = 100.0F;
        state.despawnBehindDistance = 100.0F;
    }
};

TEST_F(LevelScrollStateTest, InitialValues) {
    EXPECT_EQ(state.getViewportLeft(), 0.0F);
    EXPECT_EQ(state.getViewportRight(), 1920.0F);
    EXPECT_EQ(state.getSpawnThreshold(), 2020.0F);
    EXPECT_EQ(state.getDespawnThreshold(), -100.0F);
    EXPECT_FALSE(state.isComplete());
}

TEST_F(LevelScrollStateTest, ScrollProgress) {
    state.scrollOffset = 1000.0F;

    EXPECT_EQ(state.getViewportLeft(), 1000.0F);
    EXPECT_EQ(state.getViewportRight(), 2920.0F);
    EXPECT_EQ(state.getSpawnThreshold(), 3020.0F);
    EXPECT_EQ(state.getDespawnThreshold(), 900.0F);
    EXPECT_FALSE(state.isComplete());
}

TEST_F(LevelScrollStateTest, LevelComplete) {
    // Level complete when scrollOffset >= levelWidth - viewportWidth
    state.scrollOffset = 5000.0F - 1920.0F;  // = 3080
    EXPECT_TRUE(state.isComplete());
}

TEST_F(LevelScrollStateTest, CoordinateConversion) {
    state.scrollOffset = 500.0F;

    // Level X 600 should be at screen X 100
    EXPECT_EQ(state.levelToScreenX(600.0F), 100.0F);

    // Screen X 100 should be at level X 600
    EXPECT_EQ(state.screenToLevelX(100.0F), 600.0F);
}

// =============================================================================
// LevelLoader Tests
// =============================================================================

class LevelLoaderTest : public ::testing::Test {
   protected:
    ECS::Registry registry;
    LevelLoader loader;

    void SetUp() override { registry.reserveEntities(100); }
};

TEST_F(LevelLoaderTest, InitialState) {
    EXPECT_FALSE(loader.isLoaded());
    EXPECT_FALSE(loader.isComplete());
    EXPECT_TRUE(loader.getLevelId().empty());
}

TEST_F(LevelLoaderTest, LoadLevelFromConfig) {
    LevelConfig config;
    config.id = "test_level";
    config.name = "Test Level";
    config.scrollSpeed = 150.0F;
    config.map.levelWidth = 3000.0F;

    // Add a wave to make config valid
    WaveConfig wave;
    wave.waveNumber = 1;
    wave.spawns.push_back({"enemy", 800.0F, 300.0F, 0.0F, 1});
    config.waves.push_back(wave);

    loader.loadLevel(config);

    EXPECT_TRUE(loader.isLoaded());
    EXPECT_EQ(loader.getLevelId(), "test_level");
    EXPECT_EQ(loader.getScrollState().scrollSpeed, 150.0F);
    EXPECT_EQ(loader.getScrollState().levelWidth, 3000.0F);
}

TEST_F(LevelLoaderTest, ScrollProgress) {
    LevelConfig config;
    config.id = "scroll_test";
    config.scrollSpeed = 100.0F;
    config.map.levelWidth = 2000.0F;
    config.map.viewportWidth = 800.0F;

    WaveConfig wave;
    wave.waveNumber = 1;
    wave.spawns.push_back({"enemy", 800.0F, 300.0F, 0.0F, 1});
    config.waves.push_back(wave);

    loader.loadLevel(config);
    EXPECT_EQ(loader.getScrollState().scrollOffset, 0.0F);

    // Simulate 1 second of scrolling
    loader.update(registry, 1.0F);
    EXPECT_FLOAT_EQ(loader.getScrollState().scrollOffset, 100.0F);

    // Simulate 5 more seconds
    loader.update(registry, 5.0F);
    EXPECT_FLOAT_EQ(loader.getScrollState().scrollOffset, 600.0F);
}

TEST_F(LevelLoaderTest, PauseResume) {
    LevelConfig config;
    config.id = "pause_test";
    config.scrollSpeed = 100.0F;

    WaveConfig wave;
    wave.waveNumber = 1;
    wave.spawns.push_back({"enemy", 800.0F, 300.0F, 0.0F, 1});
    config.waves.push_back(wave);

    loader.loadLevel(config);

    // Scroll for 1 second
    loader.update(registry, 1.0F);
    float offsetAfterScroll = loader.getScrollState().scrollOffset;

    // Pause and try to scroll
    loader.setPaused(true);
    loader.update(registry, 1.0F);
    EXPECT_EQ(loader.getScrollState().scrollOffset, offsetAfterScroll);

    // Resume
    loader.setPaused(false);
    loader.update(registry, 1.0F);
    EXPECT_GT(loader.getScrollState().scrollOffset, offsetAfterScroll);
}

TEST_F(LevelLoaderTest, SpawnObstacle) {
    LevelConfig config;
    config.id = "spawn_test";
    config.scrollSpeed = 0.0F;  // No scroll for testing
    config.map.viewportWidth = 1920.0F;

    // Add obstacle at x=1000 (within initial spawn threshold of 2020)
    MapElementConfig obstacle;
    obstacle.id = "test_obstacle";
    obstacle.type = MapElementType::Obstacle;
    obstacle.x = 1000.0F;
    obstacle.y = 500.0F;
    obstacle.width = 64.0F;
    obstacle.height = 32.0F;
    config.map.obstacles.push_back(obstacle);

    WaveConfig wave;
    wave.waveNumber = 1;
    wave.spawns.push_back({"enemy", 800.0F, 300.0F, 0.0F, 1});
    config.waves.push_back(wave);

    loader.loadLevel(config);
    loader.update(registry, 0.0F);  // Trigger spawn check

    // Verify obstacle was spawned
    int obstacleCount = 0;
    registry.view<ObstacleTag, TransformComponent>().each(
        [&obstacleCount](auto /*entity*/, const auto& /*tag*/,
                         const auto& transform) {
            ++obstacleCount;
            EXPECT_FLOAT_EQ(transform.x, 1000.0F);  // No scroll, same as level X
            EXPECT_FLOAT_EQ(transform.y, 500.0F);
        });

    EXPECT_EQ(obstacleCount, 1);
}

TEST_F(LevelLoaderTest, Reset) {
    LevelConfig config;
    config.id = "reset_test";
    config.scrollSpeed = 100.0F;

    MapElementConfig obstacle;
    obstacle.id = "obstacle";
    obstacle.x = 500.0F;
    obstacle.y = 500.0F;
    config.map.obstacles.push_back(obstacle);

    WaveConfig wave;
    wave.waveNumber = 1;
    wave.spawns.push_back({"enemy", 800.0F, 300.0F, 0.0F, 1});
    config.waves.push_back(wave);

    loader.loadLevel(config);
    loader.update(registry, 5.0F);  // Scroll and spawn

    // Should have spawned obstacle
    EXPECT_GT(loader.getScrollState().scrollOffset, 0.0F);

    loader.reset(registry);

    EXPECT_EQ(loader.getScrollState().scrollOffset, 0.0F);
    EXPECT_FALSE(loader.getScrollState().isPaused);

    // Obstacles should be removed
    int count = 0;
    registry.view<ObstacleTag>().each([&count](auto /*entity*/, const auto&) {
        ++count;
    });
    EXPECT_EQ(count, 0);
}

// =============================================================================
// MapScrollingSystem Tests
// =============================================================================

class MapScrollingSystemTest : public ::testing::Test {
   protected:
    ECS::Registry registry;
    MapScrollingSystem scrollSystem;
    LevelScrollState scrollState;

    void SetUp() override {
        scrollState.scrollOffset = 0.0F;
        scrollState.viewportWidth = 1920.0F;
        scrollSystem.setScrollState(&scrollState);
    }
};

TEST_F(MapScrollingSystemTest, UpdatePositions) {
    // Create a map element entity
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<MapElementTag>(entity);
    registry.emplaceComponent<ScrollComponent>(entity, 1000.0F);  // Level X
    registry.emplaceComponent<TransformComponent>(entity, 1000.0F, 500.0F,
                                                   0.0F);

    // Initial position should be at level X (no scroll yet)
    scrollSystem.update(registry, 0.0F);
    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 1000.0F);

    // Scroll 200 pixels
    scrollState.scrollOffset = 200.0F;
    scrollSystem.update(registry, 0.0F);
    EXPECT_FLOAT_EQ(transform.x, 800.0F);  // 1000 - 200

    // Scroll more
    scrollState.scrollOffset = 500.0F;
    scrollSystem.update(registry, 0.0F);
    EXPECT_FLOAT_EQ(transform.x, 500.0F);  // 1000 - 500
}

TEST_F(MapScrollingSystemTest, NoScrollStateDoesNothing) {
    MapScrollingSystem system;
    // No scroll state set

    auto entity = registry.spawnEntity();
    registry.emplaceComponent<MapElementTag>(entity);
    registry.emplaceComponent<ScrollComponent>(entity, 1000.0F);
    registry.emplaceComponent<TransformComponent>(entity, 1000.0F, 500.0F,
                                                   0.0F);

    // Should not crash or modify position
    system.update(registry, 0.0F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 1000.0F);  // Unchanged
}

// =============================================================================
// EntityConfigRegistry Level Loading Tests
// =============================================================================

class EntityConfigLevelTest : public ::testing::Test {
   protected:
    void SetUp() override { EntityConfigRegistry::getInstance().clear(); }

    void TearDown() override { EntityConfigRegistry::getInstance().clear(); }
};

TEST_F(EntityConfigLevelTest, LoadTestLevel) {
    // This test requires the test_level.toml file to exist
    std::string levelPath = "config/game/levels/test_level.toml";

    if (!std::filesystem::exists(levelPath)) {
        GTEST_SKIP() << "Test level file not found: " << levelPath;
    }

    auto& registry = EntityConfigRegistry::getInstance();
    bool loaded = registry.loadLevel(levelPath);
    EXPECT_TRUE(loaded);

    auto levelOpt = registry.getLevel("test_level");
    ASSERT_TRUE(levelOpt.has_value());

    const auto& level = levelOpt->get();
    EXPECT_EQ(level.id, "test_level");
    EXPECT_EQ(level.name, "Test Level");
    EXPECT_FLOAT_EQ(level.scrollSpeed, 100.0F);

    // Check map elements were loaded
    EXPECT_GT(level.map.obstacles.size(), 0);
    EXPECT_GT(level.map.destroyableTiles.size(), 0);
    EXPECT_GT(level.map.starfieldLayers.size(), 0);
    EXPECT_GT(level.map.decorations.size(), 0);

    // Check starfield layers
    EXPECT_EQ(level.map.starfieldLayers.size(), 3);

    // Verify first obstacle properties
    if (!level.map.obstacles.empty()) {
        const auto& firstObstacle = level.map.obstacles[0];
        EXPECT_EQ(firstObstacle.type, MapElementType::Obstacle);
        EXPECT_GT(firstObstacle.width, 0.0F);
    }
}

}  // namespace rtype::games::rtype::shared
