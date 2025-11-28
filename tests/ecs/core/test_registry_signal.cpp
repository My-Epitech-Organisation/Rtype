/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Registry - Signal/Observer Pattern
*/

#include <gtest/gtest.h>
#include "engine/ecs/core/Registry/Registry.hpp"
#include <vector>
#include <atomic>

using namespace ECS;

// ============================================================================
// TEST COMPONENTS
// ============================================================================

struct Position {
    float x = 0.0f;
    float y = 0.0f;

    Position() = default;
    Position(float x, float y) : x(x), y(y) {}
};

struct Velocity {
    float dx = 0.0f;
    float dy = 0.0f;

    Velocity() = default;
    Velocity(float dx, float dy) : dx(dx), dy(dy) {}
};

struct Health {
    int current = 100;
    int max = 100;
};

struct DeadTag {};

// ============================================================================
// TEST FIXTURE
// ============================================================================

class RegistrySignalTest : public ::testing::Test {
protected:
    Registry registry;
};

// ============================================================================
// ON CONSTRUCT TESTS
// ============================================================================

TEST_F(RegistrySignalTest, OnConstruct_CalledWhenComponentAdded) {
    std::vector<Entity> constructedEntities;

    registry.onConstruct<Position>([&constructedEntities](Entity e) {
        constructedEntities.push_back(e);
    });

    Entity e1 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e1, 1.0f, 2.0f);

    Entity e2 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e2, 3.0f, 4.0f);

    EXPECT_EQ(constructedEntities.size(), 2);
    EXPECT_EQ(constructedEntities[0], e1);
    EXPECT_EQ(constructedEntities[1], e2);
}

TEST_F(RegistrySignalTest, OnConstruct_NotCalledOnReplace) {
    int callCount = 0;

    registry.onConstruct<Position>([&callCount](Entity) {
        callCount++;
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e, 1.0f, 2.0f);  // First call
    registry.emplaceComponent<Position>(e, 3.0f, 4.0f);  // Replace - should NOT call

    EXPECT_EQ(callCount, 1);
}

TEST_F(RegistrySignalTest, OnConstruct_MultipleCallbacks) {
    int callback1Count = 0;
    int callback2Count = 0;

    registry.onConstruct<Position>([&callback1Count](Entity) {
        callback1Count++;
    });

    registry.onConstruct<Position>([&callback2Count](Entity) {
        callback2Count++;
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);

    EXPECT_EQ(callback1Count, 1);
    EXPECT_EQ(callback2Count, 1);
}

TEST_F(RegistrySignalTest, OnConstruct_DifferentComponentTypes) {
    int positionCount = 0;
    int velocityCount = 0;

    registry.onConstruct<Position>([&positionCount](Entity) {
        positionCount++;
    });

    registry.onConstruct<Velocity>([&velocityCount](Entity) {
        velocityCount++;
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.emplaceComponent<Velocity>(e);

    EXPECT_EQ(positionCount, 1);
    EXPECT_EQ(velocityCount, 1);
}

TEST_F(RegistrySignalTest, OnConstruct_TagComponent) {
    std::vector<Entity> taggedEntities;

    registry.onConstruct<DeadTag>([&taggedEntities](Entity e) {
        taggedEntities.push_back(e);
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<DeadTag>(e);

    EXPECT_EQ(taggedEntities.size(), 1);
    EXPECT_EQ(taggedEntities[0], e);
}

// ============================================================================
// ON DESTROY TESTS
// ============================================================================

TEST_F(RegistrySignalTest, OnDestroy_CalledWhenComponentRemoved) {
    std::vector<Entity> destroyedEntities;

    registry.onDestroy<Position>([&destroyedEntities](Entity e) {
        destroyedEntities.push_back(e);
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.removeComponent<Position>(e);

    EXPECT_EQ(destroyedEntities.size(), 1);
    EXPECT_EQ(destroyedEntities[0], e);
}

TEST_F(RegistrySignalTest, OnDestroy_CalledWhenEntityKilled) {
    std::vector<Entity> destroyedEntities;

    registry.onDestroy<Position>([&destroyedEntities](Entity e) {
        destroyedEntities.push_back(e);
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.killEntity(e);

    EXPECT_EQ(destroyedEntities.size(), 1);
    EXPECT_EQ(destroyedEntities[0], e);
}

TEST_F(RegistrySignalTest, OnDestroy_CalledForAllComponentsOnKill) {
    int positionDestroyCount = 0;
    int velocityDestroyCount = 0;

    registry.onDestroy<Position>([&positionDestroyCount](Entity) {
        positionDestroyCount++;
    });

    registry.onDestroy<Velocity>([&velocityDestroyCount](Entity) {
        velocityDestroyCount++;
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.emplaceComponent<Velocity>(e);
    registry.killEntity(e);

    EXPECT_EQ(positionDestroyCount, 1);
    EXPECT_EQ(velocityDestroyCount, 1);
}

TEST_F(RegistrySignalTest, OnDestroy_NotCalledIfNoComponent) {
    int destroyCount = 0;

    registry.onDestroy<Position>([&destroyCount](Entity) {
        destroyCount++;
    });

    Entity e = registry.spawnEntity();
    // No Position component added
    registry.killEntity(e);

    EXPECT_EQ(destroyCount, 0);
}

TEST_F(RegistrySignalTest, OnDestroy_MultipleCallbacks) {
    int callback1Count = 0;
    int callback2Count = 0;

    registry.onDestroy<Position>([&callback1Count](Entity) {
        callback1Count++;
    });

    registry.onDestroy<Position>([&callback2Count](Entity) {
        callback2Count++;
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.removeComponent<Position>(e);

    EXPECT_EQ(callback1Count, 1);
    EXPECT_EQ(callback2Count, 1);
}

// ============================================================================
// CLEAR COMPONENTS SIGNAL TESTS
// ============================================================================

TEST_F(RegistrySignalTest, ClearComponents_TriggersDestroyForAll) {
    int destroyCount = 0;

    registry.onDestroy<Position>([&destroyCount](Entity) {
        destroyCount++;
    });

    for (int i = 0; i < 5; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e);
    }

    registry.clearComponents<Position>();

    EXPECT_EQ(destroyCount, 5);
}

// ============================================================================
// SIGNAL COMBINED WITH VIEWS
// ============================================================================

TEST_F(RegistrySignalTest, Signal_AddComponentDuringView_Safe) {
    registry.onConstruct<Health>([this](Entity e) {
        // This callback tries to add another component
        // This should be safe as long as we're not modifying the same pool
        if (!registry.hasComponent<DeadTag>(e)) {
            // Don't do anything that would invalidate iteration
        }
    });

    for (int i = 0; i < 10; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e);
    }

    // Add Health through view iteration
    registry.view<Position>().each([this](Entity e, Position&) {
        registry.emplaceComponent<Health>(e);
    });

    EXPECT_EQ(registry.countComponents<Health>(), 10);
}

// ============================================================================
// SIGNAL USE CASES
// ============================================================================

TEST_F(RegistrySignalTest, UseCase_AutoInitialization) {
    // When Position is added, automatically add default Velocity
    registry.onConstruct<Position>([this](Entity e) {
        if (!registry.hasComponent<Velocity>(e)) {
            registry.emplaceComponent<Velocity>(e, 0.0f, 0.0f);
        }
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e, 10.0f, 20.0f);

    EXPECT_TRUE(registry.hasComponent<Velocity>(e));
}

TEST_F(RegistrySignalTest, UseCase_CleanupOnRemoval) {
    std::vector<Entity> cleanupLog;

    registry.onDestroy<Position>([&cleanupLog](Entity e) {
        cleanupLog.push_back(e);
        // Could do: release physics body, remove from spatial hash, etc.
    });

    Entity e1 = registry.spawnEntity();
    Entity e2 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e1);
    registry.emplaceComponent<Position>(e2);

    registry.killEntity(e1);

    EXPECT_EQ(cleanupLog.size(), 1);
    EXPECT_EQ(cleanupLog[0], e1);
}

TEST_F(RegistrySignalTest, UseCase_DeathTracking) {
    std::atomic<int> deathCount{0};

    registry.onConstruct<DeadTag>([&deathCount](Entity) {
        deathCount++;
    });

    // Simulate some entities dying
    for (int i = 0; i < 5; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Health>(e);
        // Mark as dead
        registry.emplaceComponent<DeadTag>(e);
    }

    EXPECT_EQ(deathCount.load(), 5);
}

TEST_F(RegistrySignalTest, UseCase_ChainedCallbacks) {
    std::vector<std::string> eventLog;

    registry.onConstruct<Position>([&eventLog](Entity) {
        eventLog.push_back("Position added");
    });

    registry.onConstruct<Velocity>([&eventLog](Entity) {
        eventLog.push_back("Velocity added");
    });

    registry.onDestroy<Position>([&eventLog](Entity) {
        eventLog.push_back("Position removed");
    });

    registry.onDestroy<Velocity>([&eventLog](Entity) {
        eventLog.push_back("Velocity removed");
    });

    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.emplaceComponent<Velocity>(e);
    registry.killEntity(e);

    ASSERT_EQ(eventLog.size(), 4);
    EXPECT_EQ(eventLog[0], "Position added");
    EXPECT_EQ(eventLog[1], "Velocity added");
    // Destroy order depends on internal ordering
    EXPECT_TRUE(
        (eventLog[2] == "Position removed" && eventLog[3] == "Velocity removed") ||
        (eventLog[2] == "Velocity removed" && eventLog[3] == "Position removed")
    );
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST_F(RegistrySignalTest, StressTest_ManySignals) {
    std::atomic<int> constructCount{0};
    std::atomic<int> destroyCount{0};

    registry.onConstruct<Position>([&constructCount](Entity) {
        constructCount++;
    });

    registry.onDestroy<Position>([&destroyCount](Entity) {
        destroyCount++;
    });

    std::vector<Entity> entities;
    for (int i = 0; i < 1000; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e);
        entities.push_back(e);
    }

    EXPECT_EQ(constructCount.load(), 1000);

    for (Entity e : entities) {
        registry.killEntity(e);
    }

    EXPECT_EQ(destroyCount.load(), 1000);
}
