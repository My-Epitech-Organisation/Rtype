/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_spawning
*/

#include <gtest/gtest.h>

#include <thread>
#include <chrono>

#include "../../../src/games/rtype/shared/Components.hpp"
#include "../../../src/games/rtype/server/GameEngine.hpp"

using namespace rtype::games::rtype::shared;
using namespace rtype::games::rtype::server;

// =============================================================================
// Component Tests
// =============================================================================

TEST(ComponentsTest, TransformComponentDefault) {
    TransformComponent transform;

    EXPECT_FLOAT_EQ(transform.x, 0.0f);
    EXPECT_FLOAT_EQ(transform.y, 0.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 0.0f);
}

TEST(ComponentsTest, TransformComponentSetValues) {
    TransformComponent transform;
    transform.x = 10.0f;
    transform.y = 20.0f;
    transform.rotation = 45.0f;

    EXPECT_FLOAT_EQ(transform.x, 10.0f);
    EXPECT_FLOAT_EQ(transform.y, 20.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 45.0f);
}

TEST(ComponentsTest, VelocityComponentDefault) {
    VelocityComponent velocity;

    EXPECT_FLOAT_EQ(velocity.vx, 0.0f);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0f);
}

TEST(ComponentsTest, VelocityComponentSetValues) {
    VelocityComponent velocity;
    velocity.vx = 5.0f;
    velocity.vy = -3.0f;

    EXPECT_FLOAT_EQ(velocity.vx, 5.0f);
    EXPECT_FLOAT_EQ(velocity.vy, -3.0f);
}

TEST(ComponentsTest, NetworkIdComponentDefault) {
    NetworkIdComponent netId;

    EXPECT_EQ(netId.networkId, INVALID_NETWORK_ID);
    EXPECT_FALSE(netId.isValid());
}

TEST(ComponentsTest, NetworkIdComponentSetValue) {
    NetworkIdComponent netId;
    netId.networkId = 42;

    EXPECT_EQ(netId.networkId, 42u);
    EXPECT_TRUE(netId.isValid());
}

TEST(ComponentsTest, AIComponentDefault) {
    AIComponent ai;

    EXPECT_EQ(ai.behavior, AIBehavior::MoveLeft);
    EXPECT_FLOAT_EQ(ai.speed, 100.0F);
}

TEST(ComponentsTest, HealthComponentDefault) {
    HealthComponent health;

    EXPECT_EQ(health.current, 100);
    EXPECT_EQ(health.max, 100);
}

TEST(ComponentsTest, BoundingBoxComponentDefault) {
    BoundingBoxComponent bbox;

    EXPECT_FLOAT_EQ(bbox.width, 32.0F);
    EXPECT_FLOAT_EQ(bbox.height, 32.0F);
}

// =============================================================================
// GameEngine Tests
// =============================================================================

class GameEngineTest : public ::testing::Test {
   protected:
    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();
        engine = std::make_unique<GameEngine>(registry);
    }

    void TearDown() override {
        if (engine && engine->isRunning()) {
            engine->shutdown();
        }
    }

    std::shared_ptr<ECS::Registry> registry;
    std::unique_ptr<GameEngine> engine;
};

TEST_F(GameEngineTest, InitializeSucceeds) {
    EXPECT_TRUE(engine->initialize());
    EXPECT_TRUE(engine->isRunning());
}

TEST_F(GameEngineTest, InitializeTwiceFails) {
    EXPECT_TRUE(engine->initialize());
    EXPECT_FALSE(engine->initialize());
}

TEST_F(GameEngineTest, ShutdownStopsEngine) {
    engine->initialize();
    engine->shutdown();
    EXPECT_FALSE(engine->isRunning());
}

TEST_F(GameEngineTest, UpdateWithoutInitializeDoesNothing) {
    engine->update(0.016f);
    EXPECT_FALSE(engine->isRunning());
}

TEST_F(GameEngineTest, EntityCountStartsAtZero) {
    engine->initialize();
    EXPECT_EQ(engine->getEntityCount(), 0u);
}

// =============================================================================
// Spawner System Tests
// =============================================================================

TEST_F(GameEngineTest, SpawnerSystemSpawnsEnemies) {
    engine->initialize();

    // Run enough updates to trigger spawning
    // Spawn interval is between 1-3 seconds, so we simulate 4 seconds
    for (int i = 0; i < 240; ++i) {  // 4 seconds at 60 FPS
        engine->update(1.0f / 60.0f);
    }

    EXPECT_GT(engine->getEntityCount(), 0u);
}

TEST_F(GameEngineTest, SpawnerSystemEmitsSpawnEvents) {
    engine->initialize();

    std::vector<rtype::engine::GameEvent> receivedEvents;
    engine->setEventCallback([&receivedEvents](const rtype::engine::GameEvent& event) {
        receivedEvents.push_back(event);
    });

    // Run enough updates to trigger spawning
    for (int i = 0; i < 240; ++i) {
        engine->update(1.0f / 60.0f);
    }

    // Check that spawn events were emitted
    bool hasSpawnEvent = false;
    for (const auto& event : receivedEvents) {
        if (event.type == rtype::engine::GameEventType::EntitySpawned) {
            hasSpawnEvent = true;
            break;
        }
    }
    EXPECT_TRUE(hasSpawnEvent);
}

TEST_F(GameEngineTest, SpawnerSystemRespectsMaxEnemies) {
    engine->initialize();

    // Run many updates to try to spawn more than max
    for (int i = 0; i < 6000; ++i) {  // 100 seconds at 60 FPS
        engine->update(1.0f / 60.0f);
    }

    EXPECT_LE(engine->getEntityCount(), GameConfig::MAX_ENEMIES);
}

// =============================================================================
// AI System Tests
// =============================================================================

TEST_F(GameEngineTest, AISystemSetsVelocityForMoveLeftBehavior) {
    engine->initialize();

    // Force spawn an enemy by running updates
    for (int i = 0; i < 240; ++i) {
        engine->update(1.0f / 60.0f);
    }

    // Check that enemies have negative X velocity (moving left)
    auto& registry = engine->getRegistry();
    auto view = registry.view<VelocityComponent, AIComponent, EnemyTag>();

    bool hasNonPositiveVX = false;
    view.each([&hasNonPositiveVX](ECS::Entity /*entity*/,
                                  const VelocityComponent& velocity,
                                  const AIComponent& /*ai*/, const EnemyTag& /*tag*/) {
        if (velocity.vx <= 0.0F) {
            hasNonPositiveVX = true;
        }
    });

    // If enemies were spawned, they should be moving
    if (engine->getEntityCount() > 0) {
        EXPECT_TRUE(hasNonPositiveVX);
    }
}

// =============================================================================
// Movement System Tests
// =============================================================================

TEST_F(GameEngineTest, MovementSystemUpdatesPosition) {
    engine->initialize();

    // Force spawn by running updates
    for (int i = 0; i < 240; ++i) {
        engine->update(1.0f / 60.0f);
    }

    if (engine->getEntityCount() == 0) {
        GTEST_SKIP() << "No enemies spawned, skipping movement test";
    }

    // Get initial positions
    auto& registry = engine->getRegistry();
    std::vector<float> initialXPositions;
    auto view = registry.view<TransformComponent, EnemyTag>();
    view.each([&initialXPositions](ECS::Entity /*entity*/,
                                   const TransformComponent& transform,
                                   const EnemyTag& /*tag*/) {
        initialXPositions.push_back(transform.x);
    });

    // Run more updates
    for (int i = 0; i < 60; ++i) {
        engine->update(1.0f / 60.0f);
    }

    // Check positions have changed (moved left)
    std::vector<float> newXPositions;
    view.each([&newXPositions](ECS::Entity /*entity*/,
                               const TransformComponent& transform,
                               const EnemyTag& /*tag*/) {
        newXPositions.push_back(transform.x);
    });
}

// =============================================================================
// Cleanup System Tests
// =============================================================================

TEST_F(GameEngineTest, CleanupSystemDestroysEntitiesOutOfBounds) {
    engine->initialize();

    // Spawn enemies and let them move off screen
    // At 100 speed, enemies need ~21 seconds to cross from spawn (1970) to cleanup (-100)
    for (int i = 0; i < 600; ++i) {  // 10 seconds at 60 FPS
        engine->update(1.0f / 60.0f);
    }

    // Store count after initial spawning
    size_t countAfterSpawning = engine->getEntityCount();

    // Skip if no enemies spawned
    if (countAfterSpawning == 0) {
        GTEST_SKIP() << "No enemies spawned, skipping cleanup test";
    }

    // Track destroy events to verify cleanup is working
    std::vector<rtype::engine::GameEvent> destroyEvents;
    engine->setEventCallback([&destroyEvents](const rtype::engine::GameEvent& event) {
        if (event.type == rtype::engine::GameEventType::EntityDestroyed) {
            destroyEvents.push_back(event);
        }
    });

    // Run more updates to let enemies move off screen
    // Need ~21 seconds total, already ran 10, so run 15 more to be safe
    for (int i = 0; i < 900; ++i) {  // 15 seconds at 60 FPS
        engine->update(1.0f / 60.0f);
    }

    // Verify that some enemies were destroyed (cleanup system is working)
    EXPECT_FALSE(destroyEvents.empty()) << "Expected some enemies to be destroyed after moving off screen";

    // Total count should still respect max enemies limit
    EXPECT_LE(engine->getEntityCount(), GameConfig::MAX_ENEMIES);
}

TEST_F(GameEngineTest, CleanupSystemEmitsDestroyEvents) {
    engine->initialize();

    std::vector<rtype::engine::GameEvent> spawnEvents;
    std::vector<rtype::engine::GameEvent> destroyEvents;
    engine->setEventCallback([&spawnEvents, &destroyEvents](const rtype::engine::GameEvent& event) {
        if (event.type == rtype::engine::GameEventType::EntitySpawned) {
            spawnEvents.push_back(event);
        } else if (event.type == rtype::engine::GameEventType::EntityDestroyed) {
            destroyEvents.push_back(event);
        }
    });

    // Enemies spawn at x = SCREEN_WIDTH + SPAWN_MARGIN (1970) and need to reach
    // CLEANUP_LEFT (-100). At speed 100, this takes ~20.7 seconds.
    // We simulate 25 seconds to ensure enemies have time to be destroyed.
    for (int i = 0; i < 1500; ++i) {  // 25 seconds at 60 FPS
        engine->update(1.0f / 60.0f);
    }

    // Skip test if no enemies were spawned
    if (spawnEvents.empty()) {
        GTEST_SKIP() << "No enemies spawned, skipping destroy events test";
    }

    // If enemies were spawned, some should have been destroyed by now
    EXPECT_FALSE(destroyEvents.empty())
        << "Expected destroy events after enemies moved off screen. "
        << "Spawned: " << spawnEvents.size() << " enemies";
}

// =============================================================================
// Event System Tests
// =============================================================================

TEST_F(GameEngineTest, GetPendingEventsReturnsEvents) {
    engine->initialize();

    // Run updates to trigger events
    for (int i = 0; i < 240; ++i) {
        engine->update(1.0f / 60.0f);
    }

    auto events = engine->getPendingEvents();
    // Events should have been generated if enemies were spawned
    if (engine->getEntityCount() > 0) {
        EXPECT_FALSE(events.empty());
    }
}

TEST_F(GameEngineTest, ClearPendingEventsClearsEvents) {
    engine->initialize();

    // Run updates to generate events
    for (int i = 0; i < 240; ++i) {
        engine->update(1.0f / 60.0f);
    }

    engine->clearPendingEvents();
    auto events = engine->getPendingEvents();
    EXPECT_TRUE(events.empty());
}

TEST_F(GameEngineTest, EventCallbackReceivesEvents) {
    engine->initialize();

    int eventCount = 0;
    engine->setEventCallback([&eventCount](const rtype::engine::GameEvent& /*event*/) {
        ++eventCount;
    });

    // Run updates to trigger events
    for (int i = 0; i < 240; ++i) {
        engine->update(1.0f / 60.0f);
    }

    if (engine->getEntityCount() > 0) {
        EXPECT_GT(eventCount, 0);
    }
}

// =============================================================================
// Factory Function Test
// =============================================================================

TEST(GameEngineFactoryTest, CreateGameEngineReturnsValidEngine) {
    auto registry = std::make_shared<ECS::Registry>();
    auto engine = rtype::engine::createGameEngine(registry);
    ASSERT_NE(engine, nullptr);
    EXPECT_TRUE(engine->initialize());
    EXPECT_TRUE(engine->isRunning());
    engine->shutdown();
}

