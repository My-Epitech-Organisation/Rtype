/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Registry - Entity Management
*/

#include <gtest/gtest.h>
#include "../../../lib/rtype_ecs/src/core/Registry/Registry.hpp"
#include <vector>
#include <set>
#include <thread>

using namespace ECS;

// ============================================================================
// TEST FIXTURE
// ============================================================================

class RegistryEntityTest : public ::testing::Test {
protected:
    Registry registry;
};

// ============================================================================
// ENTITY CREATION TESTS
// ============================================================================

TEST_F(RegistryEntityTest, SpawnEntity_ReturnsValidEntity) {
    Entity e = registry.spawnEntity();

    EXPECT_TRUE(registry.isAlive(e));
    EXPECT_EQ(e.index(), 0);
    EXPECT_EQ(e.generation(), 0);
}

TEST_F(RegistryEntityTest, SpawnEntity_MultipleEntities_UniqueIndices) {
    std::set<std::uint32_t> indices;

    for (int i = 0; i < 100; ++i) {
        Entity e = registry.spawnEntity();
        EXPECT_TRUE(indices.insert(e.index()).second)
            << "Duplicate index at iteration " << i;
    }

    EXPECT_EQ(indices.size(), 100);
}

TEST_F(RegistryEntityTest, SpawnEntity_SequentialIndices) {
    Entity e0 = registry.spawnEntity();
    Entity e1 = registry.spawnEntity();
    Entity e2 = registry.spawnEntity();

    EXPECT_EQ(e0.index(), 0);
    EXPECT_EQ(e1.index(), 1);
    EXPECT_EQ(e2.index(), 2);
}

TEST_F(RegistryEntityTest, SpawnEntity_AfterReserve_StillWorks) {
    registry.reserveEntities(1000);

    Entity e = registry.spawnEntity();
    EXPECT_TRUE(registry.isAlive(e));
}

// ============================================================================
// ENTITY DESTRUCTION TESTS
// ============================================================================

TEST_F(RegistryEntityTest, KillEntity_EntityBecomesInvalid) {
    Entity e = registry.spawnEntity();
    EXPECT_TRUE(registry.isAlive(e));

    registry.killEntity(e);
    EXPECT_FALSE(registry.isAlive(e));
}

TEST_F(RegistryEntityTest, KillEntity_DoubleKill_NoEffect) {
    Entity e = registry.spawnEntity();
    registry.killEntity(e);
    registry.killEntity(e);  // Should not crash

    EXPECT_FALSE(registry.isAlive(e));
}

TEST_F(RegistryEntityTest, KillEntity_InvalidEntity_NoEffect) {
    Entity invalid(999, 0);
    registry.killEntity(invalid);  // Should not crash
}

TEST_F(RegistryEntityTest, KillEntity_NullEntity_NoEffect) {
    Entity null;  // Default constructed = null entity
    registry.killEntity(null);  // Should not crash
}

// ============================================================================
// ENTITY RECYCLING TESTS
// ============================================================================

TEST_F(RegistryEntityTest, EntityRecycling_IndexReused) {
    Entity e1 = registry.spawnEntity();
    std::uint32_t oldIndex = e1.index();

    registry.killEntity(e1);
    Entity e2 = registry.spawnEntity();

    EXPECT_EQ(e2.index(), oldIndex);
    EXPECT_EQ(e2.generation(), 1);  // Generation incremented
}

TEST_F(RegistryEntityTest, EntityRecycling_OldHandleInvalid) {
    Entity e1 = registry.spawnEntity();
    registry.killEntity(e1);
    Entity e2 = registry.spawnEntity();

    EXPECT_FALSE(registry.isAlive(e1));
    EXPECT_TRUE(registry.isAlive(e2));
}

TEST_F(RegistryEntityTest, EntityRecycling_MultipleRecycles) {
    Entity e = registry.spawnEntity();
    std::uint32_t index = e.index();

    for (int i = 0; i < 10; ++i) {
        registry.killEntity(e);
        e = registry.spawnEntity();

        EXPECT_EQ(e.index(), index);
        EXPECT_EQ(e.generation(), static_cast<std::uint32_t>(i + 1));
    }
}

// ============================================================================
// IS ALIVE TESTS
// ============================================================================

TEST_F(RegistryEntityTest, IsAlive_NewEntity_ReturnsTrue) {
    Entity e = registry.spawnEntity();
    EXPECT_TRUE(registry.isAlive(e));
}

TEST_F(RegistryEntityTest, IsAlive_DeadEntity_ReturnsFalse) {
    Entity e = registry.spawnEntity();
    registry.killEntity(e);
    EXPECT_FALSE(registry.isAlive(e));
}

TEST_F(RegistryEntityTest, IsAlive_NeverCreatedEntity_ReturnsFalse) {
    Entity fake(42, 0);
    EXPECT_FALSE(registry.isAlive(fake));
}

TEST_F(RegistryEntityTest, IsAlive_WrongGeneration_ReturnsFalse) {
    Entity e = registry.spawnEntity();
    Entity wrongGen(e.index(), e.generation() + 1);

    EXPECT_FALSE(registry.isAlive(wrongGen));
}

TEST_F(RegistryEntityTest, IsAlive_NullEntity_ReturnsFalse) {
    Entity null;  // Default constructed = null entity
    EXPECT_FALSE(registry.isAlive(null));
}

// ============================================================================
// TOMBSTONE CLEANUP TESTS
// ============================================================================

TEST_F(RegistryEntityTest, CleanupTombstones_EmptyRegistry_ReturnsZero) {
    size_t cleaned = registry.cleanupTombstones();
    EXPECT_EQ(cleaned, 0);
}

TEST_F(RegistryEntityTest, CleanupTombstones_NoTombstones_ReturnsZero) {
    registry.spawnEntity();
    registry.spawnEntity();

    size_t cleaned = registry.cleanupTombstones();
    EXPECT_EQ(cleaned, 0);
}

// ============================================================================
// REMOVE ENTITIES IF TESTS
// ============================================================================

struct MarkerComponent {
    bool marked = true;
};

TEST_F(RegistryEntityTest, RemoveEntitiesIf_MatchingPredicate_RemovesEntities) {
    std::vector<Entity> entities;
    for (int i = 0; i < 10; ++i) {
        entities.push_back(registry.spawnEntity());
    }

    // Mark some entities for removal by adding a marker component
    for (int i = 0; i < 5; ++i) {
        registry.emplaceComponent<MarkerComponent>(entities[i]);
    }

    size_t removed = registry.removeEntitiesIf([this](Entity e) {
        return registry.hasComponent<MarkerComponent>(e);
    });

    EXPECT_EQ(removed, 5);

    // Check which entities remain
    for (int i = 0; i < 5; ++i) {
        EXPECT_FALSE(registry.isAlive(entities[i]));
    }
    for (int i = 5; i < 10; ++i) {
        EXPECT_TRUE(registry.isAlive(entities[i]));
    }
}

TEST_F(RegistryEntityTest, RemoveEntitiesIf_NoMatch_RemovesNothing) {
    for (int i = 0; i < 10; ++i) {
        registry.spawnEntity();
    }

    size_t removed = registry.removeEntitiesIf([](Entity) { return false; });
    EXPECT_EQ(removed, 0);
}

TEST_F(RegistryEntityTest, RemoveEntitiesIf_AllMatch_RemovesAll) {
    for (int i = 0; i < 5; ++i) {
        registry.spawnEntity();
    }

    size_t removed = registry.removeEntitiesIf([](Entity) { return true; });
    EXPECT_EQ(removed, 5);
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST_F(RegistryEntityTest, StressTest_CreateManyEntities) {
    constexpr size_t count = 10000;

    for (size_t i = 0; i < count; ++i) {
        Entity e = registry.spawnEntity();
        EXPECT_TRUE(registry.isAlive(e));
    }
}

TEST_F(RegistryEntityTest, StressTest_CreateAndDestroy) {
    constexpr size_t iterations = 1000;

    for (size_t i = 0; i < iterations; ++i) {
        Entity e1 = registry.spawnEntity();
        Entity e2 = registry.spawnEntity();
        Entity e3 = registry.spawnEntity();

        registry.killEntity(e2);

        EXPECT_TRUE(registry.isAlive(e1));
        EXPECT_FALSE(registry.isAlive(e2));
        EXPECT_TRUE(registry.isAlive(e3));

        registry.killEntity(e1);
        registry.killEntity(e3);
    }
}

// ============================================================================
// THREAD SAFETY TESTS (basic)
// ============================================================================

TEST_F(RegistryEntityTest, ThreadSafety_ConcurrentSpawn) {
    constexpr int numThreads = 4;
    constexpr int entitiesPerThread = 100;
    std::vector<std::thread> threads;
    std::vector<std::vector<Entity>> threadEntities(numThreads);

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([this, t, &threadEntities]() {
            for (int i = 0; i < entitiesPerThread; ++i) {
                threadEntities[t].push_back(registry.spawnEntity());
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Verify all entities are unique and alive
    std::set<std::uint32_t> allIndices;
    for (const auto& entities : threadEntities) {
        for (Entity e : entities) {
            EXPECT_TRUE(registry.isAlive(e));
            allIndices.insert(e.index());
        }
    }

    EXPECT_EQ(allIndices.size(), numThreads * entitiesPerThread);
}

TEST_F(RegistryEntityTest, ThreadSafety_ConcurrentIsAlive) {
    std::vector<Entity> entities;
    for (int i = 0; i < 100; ++i) {
        entities.push_back(registry.spawnEntity());
    }

    std::vector<std::thread> threads;
    for (int t = 0; t < 4; ++t) {
        threads.emplace_back([this, &entities]() {
            for (int i = 0; i < 1000; ++i) {
                for (Entity e : entities) {
                    volatile bool alive = registry.isAlive(e);
                    (void)alive;
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}
