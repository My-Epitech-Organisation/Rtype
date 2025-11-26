/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Registry - View System
*/

#include <gtest/gtest.h>
#include "engine/ecs/core/Registry/Registry.hpp"
#include <vector>
#include <set>
#include <algorithm>

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

    Health() = default;
    Health(int current, int max) : current(current), max(max) {}
};

struct Damage {
    int amount = 10;

    Damage() = default;
    explicit Damage(int a) : amount(a) {}
};

// Tag components
struct PlayerTag {};
struct EnemyTag {};
struct DeadTag {};

// ============================================================================
// TEST FIXTURE
// ============================================================================

class RegistryViewTest : public ::testing::Test {
protected:
    Registry registry;

    void SetUp() override {
        // Create some test entities
        for (int i = 0; i < 10; ++i) {
            Entity e = registry.spawnEntity();
            registry.emplaceComponent<Position>(e, static_cast<float>(i), static_cast<float>(i * 2));
        }
    }

    Entity createFullEntity(float x, float y, float dx, float dy, int hp) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, x, y);
        registry.emplaceComponent<Velocity>(e, dx, dy);
        registry.emplaceComponent<Health>(e, hp, 100);
        return e;
    }
};

// ============================================================================
// SINGLE COMPONENT VIEW TESTS
// ============================================================================

TEST_F(RegistryViewTest, View_SingleComponent_IteratesAll) {
    int count = 0;

    registry.view<Position>().each([&count](Entity e, Position& pos) {
        (void)e;
        (void)pos;
        count++;
    });

    EXPECT_EQ(count, 10);
}

TEST_F(RegistryViewTest, View_SingleComponent_AccessComponents) {
    std::vector<float> xValues;

    registry.view<Position>().each([&xValues](Entity e, Position& pos) {
        (void)e;
        xValues.push_back(pos.x);
    });

    EXPECT_EQ(xValues.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(std::find(xValues.begin(), xValues.end(), static_cast<float>(i)) != xValues.end());
    }
}

TEST_F(RegistryViewTest, View_SingleComponent_ModifyComponents) {
    registry.view<Position>().each([](Entity e, Position& pos) {
        (void)e;
        pos.x += 100.0f;
    });

    registry.view<Position>().each([](Entity e, Position& pos) {
        (void)e;
        EXPECT_GE(pos.x, 100.0f);
    });
}

TEST_F(RegistryViewTest, View_EmptyPool_NoIterations) {
    int count = 0;

    registry.view<Velocity>().each([&count](Entity e, Velocity& vel) {
        (void)e;
        (void)vel;
        count++;
    });

    EXPECT_EQ(count, 0);
}

// ============================================================================
// MULTI-COMPONENT VIEW TESTS
// ============================================================================

TEST_F(RegistryViewTest, View_TwoComponents_IntersectionOnly) {
    // Add velocity to only 5 entities
    int addedVelocity = 0;
    registry.view<Position>().each([this, &addedVelocity](Entity e, Position& pos) {
        if (pos.x < 5.0f) {
            registry.emplaceComponent<Velocity>(e, pos.x, pos.y);
            addedVelocity++;
        }
    });

    int count = 0;
    registry.view<Position, Velocity>().each([&count](Entity e, Position& pos, Velocity& vel) {
        (void)e;
        (void)pos;
        (void)vel;
        count++;
    });

    EXPECT_EQ(count, 5);
}

TEST_F(RegistryViewTest, View_ThreeComponents_AllRequired) {
    // Create entities with varying component combinations
    Entity e1 = createFullEntity(1, 1, 1, 1, 100);
    Entity e2 = createFullEntity(2, 2, 2, 2, 80);
    Entity e3 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e3, 3.0f, 3.0f);
    registry.emplaceComponent<Velocity>(e3, 3.0f, 3.0f);
    // e3 has no Health

    int count = 0;
    registry.view<Position, Velocity, Health>().each(
        [&count](Entity e, Position& pos, Velocity& vel, Health& hp) {
            (void)e;
            (void)pos;
            (void)vel;
            (void)hp;
            count++;
        }
    );

    EXPECT_EQ(count, 2);  // Only e1 and e2 have all three
}

TEST_F(RegistryViewTest, View_ComponentOrder_DoesNotMatter) {
    Entity e = createFullEntity(5, 5, 1, 1, 50);

    bool found_pv = false;
    bool found_vp = false;

    registry.view<Position, Velocity>().each([&found_pv, e](Entity entity, Position&, Velocity&) {
        if (entity == e) found_pv = true;
    });

    registry.view<Velocity, Position>().each([&found_vp, e](Entity entity, Velocity&, Position&) {
        if (entity == e) found_vp = true;
    });

    EXPECT_TRUE(found_pv);
    EXPECT_TRUE(found_vp);
}

// ============================================================================
// VIEW WITH TAG COMPONENTS
// ============================================================================

TEST_F(RegistryViewTest, View_TagComponent_Works) {
    registry.view<Position>().each([this](Entity e, Position& pos) {
        if (pos.x < 3.0f) {
            registry.emplaceComponent<PlayerTag>(e);
        }
    });

    int count = 0;
    registry.view<Position, PlayerTag>().each([&count](Entity e, Position& pos, const PlayerTag&) {
        (void)e;
        (void)pos;
        count++;
    });

    EXPECT_EQ(count, 3);
}

TEST_F(RegistryViewTest, View_MultipleTagComponents) {
    Entity player = registry.spawnEntity();
    registry.emplaceComponent<PlayerTag>(player);
    registry.emplaceComponent<Health>(player, 100, 100);

    Entity enemy = registry.spawnEntity();
    registry.emplaceComponent<EnemyTag>(enemy);
    registry.emplaceComponent<Health>(enemy, 50, 50);

    int playerCount = 0;
    int enemyCount = 0;

    registry.view<Health, PlayerTag>().each([&playerCount](Entity, Health&, const PlayerTag&) {
        playerCount++;
    });

    registry.view<Health, EnemyTag>().each([&enemyCount](Entity, Health&, const EnemyTag&) {
        enemyCount++;
    });

    EXPECT_EQ(playerCount, 1);
    EXPECT_EQ(enemyCount, 1);
}

// ============================================================================
// EXCLUDE VIEW TESTS
// ============================================================================

TEST_F(RegistryViewTest, ExcludeView_SingleExclusion) {
    // Mark some entities as dead
    registry.view<Position>().each([this](Entity e, Position& pos) {
        if (pos.x >= 7.0f) {
            registry.emplaceComponent<DeadTag>(e);
        }
    });

    int aliveCount = 0;
    registry.view<Position>().exclude<DeadTag>().each(
        [&aliveCount](Entity e, Position& pos) {
            (void)e;
            (void)pos;
            aliveCount++;
        }
    );

    EXPECT_EQ(aliveCount, 7);  // 10 total - 3 dead (x=7,8,9)
}

TEST_F(RegistryViewTest, ExcludeView_MultipleExclusions) {
    Entity e1 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e1, 100.0f, 100.0f);
    registry.emplaceComponent<PlayerTag>(e1);

    Entity e2 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e2, 200.0f, 200.0f);
    registry.emplaceComponent<EnemyTag>(e2);

    Entity e3 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e3, 300.0f, 300.0f);
    // No tags

    int count = 0;
    registry.view<Position>().exclude<PlayerTag, EnemyTag>().each(
        [&count](Entity e, Position& pos) {
            (void)e;
            (void)pos;
            count++;
        }
    );

    // All 10 original entities (no tags) + e3 (no tags) = 11
    EXPECT_EQ(count, 11);
}

TEST_F(RegistryViewTest, ExcludeView_ExcludeAll_NoResults) {
    // All entities have Position
    int count = 0;
    registry.view<Position>().exclude<Position>().each(
        [&count](Entity e, Position& pos) {
            (void)e;
            (void)pos;
            count++;
        }
    );

    EXPECT_EQ(count, 0);
}

// ============================================================================
// CONST VIEW TESTS
// ============================================================================

TEST_F(RegistryViewTest, ConstView_Works) {
    const Registry& constReg = registry;

    int count = 0;
    constReg.view<Position>().each([&count](Entity e, Position& pos) {
        (void)e;
        (void)pos;
        count++;
    });

    EXPECT_EQ(count, 10);
}

// ============================================================================
// GROUP TESTS
// ============================================================================

TEST_F(RegistryViewTest, Group_CachesEntities) {
    // Add velocity to half
    registry.view<Position>().each([this](Entity e, Position& pos) {
        if (pos.x < 5.0f) {
            registry.emplaceComponent<Velocity>(e, 1.0f, 1.0f);
        }
    });

    auto group = registry.createGroup<Position, Velocity>();

    int count = 0;
    group.each([&count](Entity e, Position& pos, Velocity& vel) {
        (void)e;
        (void)pos;
        (void)vel;
        count++;
    });

    EXPECT_EQ(count, 5);
}

TEST_F(RegistryViewTest, Group_Rebuild_UpdatesCache) {
    registry.view<Position>().each([this](Entity e, Position& pos) {
        (void)pos;
        registry.emplaceComponent<Velocity>(e, 1.0f, 1.0f);
    });

    auto group = registry.createGroup<Position, Velocity>();

    int count1 = 0;
    group.each([&count1](Entity, Position&, Velocity&) { count1++; });
    EXPECT_EQ(count1, 10);

    // Add more entities
    for (int i = 0; i < 5; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, 100.0f, 100.0f);
        registry.emplaceComponent<Velocity>(e, 1.0f, 1.0f);
    }

    // Group needs rebuild to see new entities
    group.rebuild();

    int count2 = 0;
    group.each([&count2](Entity, Position&, Velocity&) { count2++; });
    EXPECT_EQ(count2, 15);
}

// ============================================================================
// PARALLEL VIEW TESTS
// ============================================================================

TEST_F(RegistryViewTest, ParallelView_IteratesAll) {
    std::atomic<int> count{0};

    registry.parallelView<Position>().each([&count](Entity e, Position& pos) {
        (void)e;
        (void)pos;
        count++;
    });

    EXPECT_EQ(count.load(), 10);
}

TEST_F(RegistryViewTest, ParallelView_MultiComponent) {
    registry.view<Position>().each([this](Entity e, Position& pos) {
        (void)pos;
        registry.emplaceComponent<Velocity>(e, 1.0f, 1.0f);
    });

    std::atomic<int> count{0};

    registry.parallelView<Position, Velocity>().each(
        [&count](Entity e, Position& pos, Velocity& vel) {
            (void)e;
            (void)pos;
            (void)vel;
            count++;
        }
    );

    EXPECT_EQ(count.load(), 10);
}

TEST_F(RegistryViewTest, ParallelView_ModifyComponents_ThreadSafe) {
    std::atomic<int> totalUpdates{0};

    registry.parallelView<Position>().each([&totalUpdates](Entity e, Position& pos) {
        (void)e;
        pos.x += 1.0f;  // Each entity modifies its own component
        totalUpdates++;
    });

    EXPECT_EQ(totalUpdates.load(), 10);

    // Verify all were updated
    registry.view<Position>().each([](Entity e, Position& pos) {
        (void)e;
        EXPECT_GE(pos.x, 1.0f);
    });
}

// ============================================================================
// VIEW EDGE CASES
// ============================================================================

TEST_F(RegistryViewTest, View_EntityKilledDuringIteration_SafeWithCopy) {
    std::vector<Entity> toKill;

    registry.view<Position>().each([&toKill](Entity e, Position& pos) {
        if (pos.x >= 5.0f) {
            toKill.push_back(e);
        }
    });

    for (Entity e : toKill) {
        registry.killEntity(e);
    }

    int remaining = 0;
    registry.view<Position>().each([&remaining](Entity, Position&) {
        remaining++;
    });

    EXPECT_EQ(remaining, 5);
}

TEST_F(RegistryViewTest, View_SmallestPoolOptimization) {
    // Create 1000 entities with Position
    for (int i = 0; i < 990; ++i) {  // Already have 10 from SetUp
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, static_cast<float>(i), 0.0f);
    }

    // Add Velocity to only 5 entities
    int added = 0;
    registry.view<Position>().each([this, &added](Entity e, Position&) {
        if (added < 5) {
            registry.emplaceComponent<Velocity>(e, 1.0f, 1.0f);
            added++;
        }
    });

    // View should iterate from smallest pool (Velocity with 5 entities)
    int count = 0;
    registry.view<Position, Velocity>().each([&count](Entity, Position&, Velocity&) {
        count++;
    });

    EXPECT_EQ(count, 5);
}
