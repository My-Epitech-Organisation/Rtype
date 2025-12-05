/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for AGameEngine abstract base class
*/

#include <gtest/gtest.h>

#include "engine/AGameEngine.hpp"

using namespace rtype::engine;

// ============================================================================
// TEST CONCRETE IMPLEMENTATION
// ============================================================================

/**
 * @brief Concrete implementation of AGameEngine for testing
 */
class TestGameEngine : public AGameEngine {
   public:
    TestGameEngine() = default;
    ~TestGameEngine() override = default;

    bool initialize() override {
        _initializeCalled = true;
        if (_shouldFailInit) {
            return false;
        }
        setRunning(true);
        setEntityCount(0);
        return true;
    }

    void update(float deltaTime) override {
        _lastDeltaTime = deltaTime;
        _updateCount++;
    }

    void shutdown() override {
        _shutdownCalled = true;
        setRunning(false);
    }

    // Test helpers
    void setShouldFailInit(bool shouldFail) { _shouldFailInit = shouldFail; }
    bool wasInitializeCalled() const { return _initializeCalled; }
    bool wasShutdownCalled() const { return _shutdownCalled; }
    float getLastDeltaTime() const { return _lastDeltaTime; }
    int getUpdateCount() const { return _updateCount; }

    // Expose protected methods for testing
    using AGameEngine::emitEvent;
    using AGameEngine::setEntityCount;
    using AGameEngine::setRunning;

   private:
    bool _shouldFailInit = false;
    bool _initializeCalled = false;
    bool _shutdownCalled = false;
    float _lastDeltaTime = 0.0f;
    int _updateCount = 0;
};

// ============================================================================
// TEST FIXTURE
// ============================================================================

class AGameEngineTest : public ::testing::Test {
   protected:
    TestGameEngine engine;
};

// ============================================================================
// INITIALIZATION TESTS
// ============================================================================

TEST_F(AGameEngineTest, Initialize_Success_ReturnsTrue) {
    EXPECT_TRUE(engine.initialize());
    EXPECT_TRUE(engine.wasInitializeCalled());
}

TEST_F(AGameEngineTest, Initialize_Failure_ReturnsFalse) {
    engine.setShouldFailInit(true);
    EXPECT_FALSE(engine.initialize());
}

TEST_F(AGameEngineTest, Initialize_SetsRunningState) {
    EXPECT_FALSE(engine.isRunning());
    engine.initialize();
    EXPECT_TRUE(engine.isRunning());
}

// ============================================================================
// RUNNING STATE TESTS
// ============================================================================

TEST_F(AGameEngineTest, IsRunning_DefaultFalse) {
    EXPECT_FALSE(engine.isRunning());
}

TEST_F(AGameEngineTest, SetRunning_True) {
    engine.setRunning(true);
    EXPECT_TRUE(engine.isRunning());
}

TEST_F(AGameEngineTest, SetRunning_False) {
    engine.setRunning(true);
    engine.setRunning(false);
    EXPECT_FALSE(engine.isRunning());
}

TEST_F(AGameEngineTest, SetRunning_MultipleChanges) {
    for (int i = 0; i < 10; ++i) {
        engine.setRunning(true);
        EXPECT_TRUE(engine.isRunning());
        engine.setRunning(false);
        EXPECT_FALSE(engine.isRunning());
    }
}

// ============================================================================
// UPDATE TESTS
// ============================================================================

TEST_F(AGameEngineTest, Update_RecordsDeltaTime) {
    engine.update(0.016f);
    EXPECT_FLOAT_EQ(engine.getLastDeltaTime(), 0.016f);
}

TEST_F(AGameEngineTest, Update_MultipleCalls_CountsCorrectly) {
    EXPECT_EQ(engine.getUpdateCount(), 0);
    engine.update(0.016f);
    engine.update(0.016f);
    engine.update(0.016f);
    EXPECT_EQ(engine.getUpdateCount(), 3);
}

TEST_F(AGameEngineTest, Update_ZeroDeltaTime) {
    engine.update(0.0f);
    EXPECT_FLOAT_EQ(engine.getLastDeltaTime(), 0.0f);
}

TEST_F(AGameEngineTest, Update_LargeDeltaTime) {
    engine.update(1.0f);
    EXPECT_FLOAT_EQ(engine.getLastDeltaTime(), 1.0f);
}

// ============================================================================
// SHUTDOWN TESTS
// ============================================================================

TEST_F(AGameEngineTest, Shutdown_CalledCorrectly) {
    engine.initialize();
    engine.shutdown();
    EXPECT_TRUE(engine.wasShutdownCalled());
}

TEST_F(AGameEngineTest, Shutdown_SetsRunningFalse) {
    engine.initialize();
    EXPECT_TRUE(engine.isRunning());
    engine.shutdown();
    EXPECT_FALSE(engine.isRunning());
}

TEST_F(AGameEngineTest, Shutdown_CanBeCalledMultipleTimes) {
    engine.initialize();
    engine.shutdown();
    engine.shutdown();  // Should not crash
    EXPECT_FALSE(engine.isRunning());
}

// ============================================================================
// ENTITY COUNT TESTS
// ============================================================================

TEST_F(AGameEngineTest, GetEntityCount_DefaultZero) {
    EXPECT_EQ(engine.getEntityCount(), 0);
}

TEST_F(AGameEngineTest, SetEntityCount_UpdatesCount) {
    engine.setEntityCount(42);
    EXPECT_EQ(engine.getEntityCount(), 42);
}

TEST_F(AGameEngineTest, SetEntityCount_Zero) {
    engine.setEntityCount(100);
    engine.setEntityCount(0);
    EXPECT_EQ(engine.getEntityCount(), 0);
}

TEST_F(AGameEngineTest, SetEntityCount_LargeValue) {
    engine.setEntityCount(1000000);
    EXPECT_EQ(engine.getEntityCount(), 1000000);
}

TEST_F(AGameEngineTest, SetEntityCount_MaxValue) {
    engine.setEntityCount(std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(engine.getEntityCount(), std::numeric_limits<std::size_t>::max());
}

// ============================================================================
// EVENT CALLBACK TESTS
// ============================================================================

TEST_F(AGameEngineTest, SetEventCallback_NullCallback) {
    engine.setEventCallback(nullptr);
    // Should not crash when emitting event
    GameEvent event{GameEventType::EntitySpawned, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);
}

TEST_F(AGameEngineTest, SetEventCallback_ValidCallback) {
    bool callbackCalled = false;
    GameEventType receivedType = GameEventType::EntityDestroyed;

    engine.setEventCallback([&](const GameEvent& event) {
        callbackCalled = true;
        receivedType = event.type;
    });

    GameEvent event{GameEventType::EntitySpawned, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedType, GameEventType::EntitySpawned);
}

TEST_F(AGameEngineTest, SetEventCallback_ReplaceCallback) {
    int callCount1 = 0;
    int callCount2 = 0;

    engine.setEventCallback([&](const GameEvent&) { callCount1++; });

    GameEvent event{GameEventType::EntitySpawned, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);

    EXPECT_EQ(callCount1, 1);
    EXPECT_EQ(callCount2, 0);

    engine.setEventCallback([&](const GameEvent&) { callCount2++; });
    engine.emitEvent(event);

    EXPECT_EQ(callCount1, 1);  // Old callback not called
    EXPECT_EQ(callCount2, 1);  // New callback called
}

// ============================================================================
// EMIT EVENT TESTS
// ============================================================================

TEST_F(AGameEngineTest, EmitEvent_AddsToQueue) {
    EXPECT_EQ(engine.getPendingEvents().size(), 0);

    GameEvent event{GameEventType::EntitySpawned, 1, 10.0f, 20.0f, 45.0f, 2};
    engine.emitEvent(event);

    auto events = engine.getPendingEvents();
    ASSERT_EQ(events.size(), 1);
    EXPECT_EQ(events[0].type, GameEventType::EntitySpawned);
    EXPECT_EQ(events[0].entityNetworkId, 1);
    EXPECT_FLOAT_EQ(events[0].x, 10.0f);
    EXPECT_FLOAT_EQ(events[0].y, 20.0f);
    EXPECT_FLOAT_EQ(events[0].rotation, 45.0f);
    EXPECT_EQ(events[0].entityType, 2);
}

TEST_F(AGameEngineTest, EmitEvent_MultipleEvents) {
    GameEvent event1{GameEventType::EntitySpawned, 1, 0.0f, 0.0f, 0.0f, 0};
    GameEvent event2{GameEventType::EntityUpdated, 2, 1.0f, 2.0f, 3.0f, 1};
    GameEvent event3{GameEventType::EntityDestroyed, 3, 0.0f, 0.0f, 0.0f, 0};

    engine.emitEvent(event1);
    engine.emitEvent(event2);
    engine.emitEvent(event3);

    auto events = engine.getPendingEvents();
    ASSERT_EQ(events.size(), 3);
    EXPECT_EQ(events[0].type, GameEventType::EntitySpawned);
    EXPECT_EQ(events[1].type, GameEventType::EntityUpdated);
    EXPECT_EQ(events[2].type, GameEventType::EntityDestroyed);
}

TEST_F(AGameEngineTest, EmitEvent_CallsCallbackAndQueues) {
    bool callbackCalled = false;
    engine.setEventCallback([&](const GameEvent&) { callbackCalled = true; });

    GameEvent event{GameEventType::EntitySpawned, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(engine.getPendingEvents().size(), 1);
}

// ============================================================================
// PENDING EVENTS TESTS
// ============================================================================

TEST_F(AGameEngineTest, GetPendingEvents_EmptyByDefault) {
    auto events = engine.getPendingEvents();
    EXPECT_TRUE(events.empty());
}

TEST_F(AGameEngineTest, GetPendingEvents_ReturnsCopy) {
    GameEvent event{GameEventType::EntitySpawned, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);

    auto events1 = engine.getPendingEvents();
    auto events2 = engine.getPendingEvents();

    // Both should be identical copies
    EXPECT_EQ(events1.size(), events2.size());
}

TEST_F(AGameEngineTest, ClearPendingEvents_RemovesAllEvents) {
    GameEvent event{GameEventType::EntitySpawned, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);
    engine.emitEvent(event);
    engine.emitEvent(event);

    EXPECT_EQ(engine.getPendingEvents().size(), 3);

    engine.clearPendingEvents();

    EXPECT_EQ(engine.getPendingEvents().size(), 0);
}

TEST_F(AGameEngineTest, ClearPendingEvents_OnEmpty_NoEffect) {
    engine.clearPendingEvents();
    EXPECT_EQ(engine.getPendingEvents().size(), 0);
}

TEST_F(AGameEngineTest, ClearPendingEvents_MultipleTimes) {
    GameEvent event{GameEventType::EntitySpawned, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);
    engine.clearPendingEvents();
    engine.clearPendingEvents();  // Should not crash
    EXPECT_EQ(engine.getPendingEvents().size(), 0);
}

// ============================================================================
// GAME EVENT TYPE TESTS
// ============================================================================

TEST_F(AGameEngineTest, GameEventType_EntitySpawned) {
    GameEvent event{GameEventType::EntitySpawned, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);
    EXPECT_EQ(engine.getPendingEvents()[0].type, GameEventType::EntitySpawned);
}

TEST_F(AGameEngineTest, GameEventType_EntityDestroyed) {
    GameEvent event{GameEventType::EntityDestroyed, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);
    EXPECT_EQ(engine.getPendingEvents()[0].type, GameEventType::EntityDestroyed);
}

TEST_F(AGameEngineTest, GameEventType_EntityUpdated) {
    GameEvent event{GameEventType::EntityUpdated, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(event);
    EXPECT_EQ(engine.getPendingEvents()[0].type, GameEventType::EntityUpdated);
}

// ============================================================================
// FULL LIFECYCLE TESTS
// ============================================================================

TEST_F(AGameEngineTest, FullLifecycle_InitUpdateShutdown) {
    // Initialize
    EXPECT_TRUE(engine.initialize());
    EXPECT_TRUE(engine.isRunning());
    EXPECT_EQ(engine.getEntityCount(), 0);

    // Simulate game loop
    for (int i = 0; i < 100; ++i) {
        engine.update(0.016f);
    }
    EXPECT_EQ(engine.getUpdateCount(), 100);

    // Shutdown
    engine.shutdown();
    EXPECT_FALSE(engine.isRunning());
}

TEST_F(AGameEngineTest, FullLifecycle_WithEvents) {
    bool spawnCalled = false;
    bool destroyCalled = false;

    engine.setEventCallback([&](const GameEvent& event) {
        if (event.type == GameEventType::EntitySpawned) {
            spawnCalled = true;
        } else if (event.type == GameEventType::EntityDestroyed) {
            destroyCalled = true;
        }
    });

    engine.initialize();

    // Simulate entity spawning
    engine.setEntityCount(5);
    GameEvent spawnEvent{GameEventType::EntitySpawned, 1, 100.0f, 200.0f, 0.0f, 1};
    engine.emitEvent(spawnEvent);

    // Simulate entity destruction
    GameEvent destroyEvent{GameEventType::EntityDestroyed, 1, 0.0f, 0.0f, 0.0f, 0};
    engine.emitEvent(destroyEvent);
    engine.setEntityCount(4);

    engine.shutdown();

    EXPECT_TRUE(spawnCalled);
    EXPECT_TRUE(destroyCalled);
    EXPECT_EQ(engine.getPendingEvents().size(), 2);
}

TEST_F(AGameEngineTest, FullLifecycle_MultipleInitShutdownCycles) {
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(engine.initialize());
        EXPECT_TRUE(engine.isRunning());
        engine.update(0.016f);
        engine.shutdown();
        EXPECT_FALSE(engine.isRunning());
    }
}
