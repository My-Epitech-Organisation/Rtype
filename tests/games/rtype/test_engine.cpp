/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_engine - Tests for AGameEngine and ASystem base classes
*/

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "AGameEngine.hpp"
#include "ASystem.hpp"
#include "ECS.hpp"

using namespace rtype::engine;

// ============================================================================
// Test Fixtures - Concrete implementations for testing abstract classes
// ============================================================================

class TestSystem : public ASystem {
   public:
    explicit TestSystem(const std::string& name = "TestSystem")
        : ASystem(name) {}

    void update(ECS::Registry& registry, float deltaTime) override {
        updateCount_++;
        lastDeltaTime_ = deltaTime;
        (void)registry;
    }

    int getUpdateCount() const { return updateCount_; }
    float getLastDeltaTime() const { return lastDeltaTime_; }

   private:
    int updateCount_ = 0;
    float lastDeltaTime_ = 0.0f;
};

class TestGameEngine : public AGameEngine {
   public:
    bool initialize() override {
        setRunning(true);
        return true;
    }

    void update(float deltaTime) override {
        (void)deltaTime;
        setEntityCount(getEntityCount() + 1);
    }

    void shutdown() override { setRunning(false); }

    // Expose protected methods for testing
    void testEmitEvent(const GameEvent& event) { emitEvent(event); }

    void testSetRunning(bool running) { setRunning(running); }

    void testSetEntityCount(std::size_t count) { setEntityCount(count); }
};

// ============================================================================
// ASystem Tests
// ============================================================================

class ASystemTest : public ::testing::Test {
   protected:
    void SetUp() override { registry_ = std::make_unique<ECS::Registry>(); }

    std::unique_ptr<ECS::Registry> registry_;
};

TEST_F(ASystemTest, GetNameReturnsCorrectName) {
    TestSystem system("MyTestSystem");
    EXPECT_EQ(system.getName(), "MyTestSystem");
}

TEST_F(ASystemTest, GetNameWithEmptyString) {
    TestSystem system("");
    EXPECT_EQ(system.getName(), "");
}

TEST_F(ASystemTest, IsEnabledDefaultsToTrue) {
    TestSystem system;
    EXPECT_TRUE(system.isEnabled());
}

TEST_F(ASystemTest, SetEnabledToFalse) {
    TestSystem system;
    system.setEnabled(false);
    EXPECT_FALSE(system.isEnabled());
}

TEST_F(ASystemTest, SetEnabledToTrue) {
    TestSystem system;
    system.setEnabled(false);
    system.setEnabled(true);
    EXPECT_TRUE(system.isEnabled());
}

TEST_F(ASystemTest, UpdateIncreasesCount) {
    TestSystem system;
    system.update(*registry_, 0.016f);
    EXPECT_EQ(system.getUpdateCount(), 1);
}

TEST_F(ASystemTest, UpdateMultipleTimes) {
    TestSystem system;
    system.update(*registry_, 0.016f);
    system.update(*registry_, 0.032f);
    system.update(*registry_, 0.048f);
    EXPECT_EQ(system.getUpdateCount(), 3);
}

TEST_F(ASystemTest, UpdateStoresDeltaTime) {
    TestSystem system;
    system.update(*registry_, 0.123f);
    EXPECT_FLOAT_EQ(system.getLastDeltaTime(), 0.123f);
}

TEST_F(ASystemTest, UpdateWithZeroDeltaTime) {
    TestSystem system;
    system.update(*registry_, 0.0f);
    EXPECT_FLOAT_EQ(system.getLastDeltaTime(), 0.0f);
}

TEST_F(ASystemTest, UpdateWithNegativeDeltaTime) {
    TestSystem system;
    system.update(*registry_, -0.5f);
    EXPECT_FLOAT_EQ(system.getLastDeltaTime(), -0.5f);
}

TEST_F(ASystemTest, MultipleSystemsWithDifferentNames) {
    TestSystem system1("System1");
    TestSystem system2("System2");
    TestSystem system3("System3");

    EXPECT_EQ(system1.getName(), "System1");
    EXPECT_EQ(system2.getName(), "System2");
    EXPECT_EQ(system3.getName(), "System3");
}

TEST_F(ASystemTest, SystemsCanBeEnabledIndependently) {
    TestSystem system1("System1");
    TestSystem system2("System2");

    system1.setEnabled(false);

    EXPECT_FALSE(system1.isEnabled());
    EXPECT_TRUE(system2.isEnabled());
}

// ============================================================================
// AGameEngine Tests
// ============================================================================

class AGameEngineTest : public ::testing::Test {
   protected:
    void SetUp() override { engine_ = std::make_unique<TestGameEngine>(); }

    std::unique_ptr<TestGameEngine> engine_;
};

TEST_F(AGameEngineTest, IsRunningDefaultsToFalse) {
    EXPECT_FALSE(engine_->isRunning());
}

TEST_F(AGameEngineTest, InitializeSetsRunningToTrue) {
    EXPECT_TRUE(engine_->initialize());
    EXPECT_TRUE(engine_->isRunning());
}

TEST_F(AGameEngineTest, ShutdownSetsRunningToFalse) {
    engine_->initialize();
    engine_->shutdown();
    EXPECT_FALSE(engine_->isRunning());
}

TEST_F(AGameEngineTest, GetEntityCountDefaultsToZero) {
    EXPECT_EQ(engine_->getEntityCount(), 0u);
}

TEST_F(AGameEngineTest, UpdateIncreasesEntityCount) {
    engine_->initialize();
    engine_->update(0.016f);
    EXPECT_EQ(engine_->getEntityCount(), 1u);
}

TEST_F(AGameEngineTest, MultipleUpdatesIncreasesEntityCount) {
    engine_->initialize();
    engine_->update(0.016f);
    engine_->update(0.016f);
    engine_->update(0.016f);
    EXPECT_EQ(engine_->getEntityCount(), 3u);
}

TEST_F(AGameEngineTest, SetEntityCount) {
    engine_->testSetEntityCount(42);
    EXPECT_EQ(engine_->getEntityCount(), 42u);
}

TEST_F(AGameEngineTest, SetRunning) {
    engine_->testSetRunning(true);
    EXPECT_TRUE(engine_->isRunning());
    engine_->testSetRunning(false);
    EXPECT_FALSE(engine_->isRunning());
}

TEST_F(AGameEngineTest, GetPendingEventsInitiallyEmpty) {
    auto events = engine_->getPendingEvents();
    EXPECT_TRUE(events.empty());
}

TEST_F(AGameEngineTest, EmitEventAddsToPendingEvents) {
    GameEvent event{GameEventType::EntitySpawned, 1, 10.0f, 20.0f, 0.0f, 0};
    engine_->testEmitEvent(event);

    auto events = engine_->getPendingEvents();
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].type, GameEventType::EntitySpawned);
    EXPECT_EQ(events[0].entityNetworkId, 1u);
    EXPECT_FLOAT_EQ(events[0].x, 10.0f);
    EXPECT_FLOAT_EQ(events[0].y, 20.0f);
}

TEST_F(AGameEngineTest, EmitMultipleEvents) {
    GameEvent event1{GameEventType::EntitySpawned, 1, 0, 0, 0, 0};
    GameEvent event2{GameEventType::EntityUpdated, 2, 0, 0, 0, 0};
    GameEvent event3{GameEventType::EntityDestroyed, 3, 0, 0, 0, 0};

    engine_->testEmitEvent(event1);
    engine_->testEmitEvent(event2);
    engine_->testEmitEvent(event3);

    auto events = engine_->getPendingEvents();
    ASSERT_EQ(events.size(), 3u);
    EXPECT_EQ(events[0].entityNetworkId, 1u);
    EXPECT_EQ(events[1].entityNetworkId, 2u);
    EXPECT_EQ(events[2].entityNetworkId, 3u);
}

TEST_F(AGameEngineTest, ClearPendingEvents) {
    GameEvent event{GameEventType::EntitySpawned, 1, 0, 0, 0, 0};
    engine_->testEmitEvent(event);
    engine_->testEmitEvent(event);

    EXPECT_EQ(engine_->getPendingEvents().size(), 2u);

    engine_->clearPendingEvents();

    EXPECT_TRUE(engine_->getPendingEvents().empty());
}

TEST_F(AGameEngineTest, SetEventCallbackReceivesEvents) {
    std::vector<GameEvent> receivedEvents;
    engine_->setEventCallback(
        [&receivedEvents](const GameEvent& event) {
            receivedEvents.push_back(event);
        });

    GameEvent event{GameEventType::EntitySpawned, 42, 1.0f, 2.0f, 3.0f, 5};
    engine_->testEmitEvent(event);

    ASSERT_EQ(receivedEvents.size(), 1u);
    EXPECT_EQ(receivedEvents[0].entityNetworkId, 42u);
    EXPECT_FLOAT_EQ(receivedEvents[0].x, 1.0f);
    EXPECT_FLOAT_EQ(receivedEvents[0].y, 2.0f);
    EXPECT_FLOAT_EQ(receivedEvents[0].rotation, 3.0f);
    EXPECT_EQ(receivedEvents[0].entityType, 5);
}

TEST_F(AGameEngineTest, EventCallbackCalledForEachEvent) {
    int callCount = 0;
    engine_->setEventCallback([&callCount](const GameEvent& event) {
        (void)event;
        callCount++;
    });

    engine_->testEmitEvent({GameEventType::EntitySpawned, 1, 0, 0, 0, 0});
    engine_->testEmitEvent({GameEventType::EntityUpdated, 2, 0, 0, 0, 0});
    engine_->testEmitEvent({GameEventType::EntityDestroyed, 3, 0, 0, 0, 0});

    EXPECT_EQ(callCount, 3);
}

TEST_F(AGameEngineTest, NoCallbackDoesNotCrash) {
    // No callback set - should not crash
    GameEvent event{GameEventType::EntitySpawned, 1, 0, 0, 0, 0};
    EXPECT_NO_THROW(engine_->testEmitEvent(event));
}

TEST_F(AGameEngineTest, ReplaceEventCallback) {
    int callback1Count = 0;
    int callback2Count = 0;

    engine_->setEventCallback([&callback1Count](const GameEvent& event) {
        (void)event;
        callback1Count++;
    });

    engine_->testEmitEvent({GameEventType::EntitySpawned, 1, 0, 0, 0, 0});

    engine_->setEventCallback([&callback2Count](const GameEvent& event) {
        (void)event;
        callback2Count++;
    });

    engine_->testEmitEvent({GameEventType::EntitySpawned, 2, 0, 0, 0, 0});

    EXPECT_EQ(callback1Count, 1);
    EXPECT_EQ(callback2Count, 1);
}

// ============================================================================
// GameEventType Tests
// ============================================================================

TEST(GameEventTypeTest, EntitySpawnedValue) {
    EXPECT_EQ(static_cast<uint8_t>(GameEventType::EntitySpawned), 0);
}

TEST(GameEventTypeTest, EntityDestroyedValue) {
    EXPECT_EQ(static_cast<uint8_t>(GameEventType::EntityDestroyed), 1);
}

TEST(GameEventTypeTest, EntityUpdatedValue) {
    EXPECT_EQ(static_cast<uint8_t>(GameEventType::EntityUpdated), 2);
}

// ============================================================================
// GameEvent Tests
// ============================================================================

TEST(GameEventTest, DefaultConstruction) {
    GameEvent event{};
    EXPECT_EQ(static_cast<uint8_t>(event.type), 0);
    EXPECT_EQ(event.entityNetworkId, 0u);
    EXPECT_FLOAT_EQ(event.x, 0.0f);
    EXPECT_FLOAT_EQ(event.y, 0.0f);
    EXPECT_FLOAT_EQ(event.rotation, 0.0f);
    EXPECT_EQ(event.entityType, 0);
}

TEST(GameEventTest, BraceInitialization) {
    GameEvent event{GameEventType::EntityUpdated, 100, 50.5f, 75.25f, 90.0f,
                    3};

    EXPECT_EQ(event.type, GameEventType::EntityUpdated);
    EXPECT_EQ(event.entityNetworkId, 100u);
    EXPECT_FLOAT_EQ(event.x, 50.5f);
    EXPECT_FLOAT_EQ(event.y, 75.25f);
    EXPECT_FLOAT_EQ(event.rotation, 90.0f);
    EXPECT_EQ(event.entityType, 3);
}

TEST(GameEventTest, CopyEvent) {
    GameEvent original{GameEventType::EntitySpawned, 42, 1.0f, 2.0f, 3.0f, 5};
    GameEvent copy = original;

    EXPECT_EQ(copy.type, original.type);
    EXPECT_EQ(copy.entityNetworkId, original.entityNetworkId);
    EXPECT_FLOAT_EQ(copy.x, original.x);
    EXPECT_FLOAT_EQ(copy.y, original.y);
    EXPECT_FLOAT_EQ(copy.rotation, original.rotation);
    EXPECT_EQ(copy.entityType, original.entityType);
}
