/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Registry - Component Management
*/

#include <gtest/gtest.h>
#include "../../../lib/rtype_ecs/src/ecs/core/Registry/Registry.hpp"
#include <string>
#include <vector>
#include <memory>

using namespace ECS;

// ============================================================================
// TEST COMPONENTS
// ============================================================================

struct Position {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Position() = default;
    Position(float x, float y, float z = 0.0f) : x(x), y(y), z(z) {}

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
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

struct Name {
    std::string value;

    Name() = default;
    explicit Name(std::string v) : value(std::move(v)) {}
};

// Tag components (empty structs)
struct PlayerTag {};
struct EnemyTag {};
struct DeadTag {};

// Complex component with non-trivial destructor
struct Resource {
    std::unique_ptr<int> data;

    Resource() : data(std::make_unique<int>(42)) {}
    explicit Resource(int val) : data(std::make_unique<int>(val)) {}
};

// ============================================================================
// TEST FIXTURE
// ============================================================================

class RegistryComponentTest : public ::testing::Test {
protected:
    Registry registry;

    Entity createEntityWithPosition(float x, float y) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, x, y);
        return e;
    }
};

// ============================================================================
// EMPLACE COMPONENT TESTS
// ============================================================================

TEST_F(RegistryComponentTest, EmplaceComponent_BasicType_Success) {
    Entity e = registry.spawnEntity();

    Position& pos = registry.emplaceComponent<Position>(e, 10.0f, 20.0f);

    EXPECT_EQ(pos.x, 10.0f);
    EXPECT_EQ(pos.y, 20.0f);
}

TEST_F(RegistryComponentTest, EmplaceComponent_DefaultConstruction) {
    Entity e = registry.spawnEntity();

    Position& pos = registry.emplaceComponent<Position>(e);

    EXPECT_EQ(pos.x, 0.0f);
    EXPECT_EQ(pos.y, 0.0f);
}

TEST_F(RegistryComponentTest, EmplaceComponent_StringComponent) {
    Entity e = registry.spawnEntity();

    Name& name = registry.emplaceComponent<Name>(e, "TestEntity");

    EXPECT_EQ(name.value, "TestEntity");
}

TEST_F(RegistryComponentTest, EmplaceComponent_TagComponent) {
    Entity e = registry.spawnEntity();

    registry.emplaceComponent<PlayerTag>(e);

    EXPECT_TRUE(registry.hasComponent<PlayerTag>(e));
}

TEST_F(RegistryComponentTest, EmplaceComponent_MultipleComponents) {
    Entity e = registry.spawnEntity();

    registry.emplaceComponent<Position>(e, 1.0f, 2.0f);
    registry.emplaceComponent<Velocity>(e, 3.0f, 4.0f);
    registry.emplaceComponent<Health>(e, 50, 100);

    EXPECT_TRUE(registry.hasComponent<Position>(e));
    EXPECT_TRUE(registry.hasComponent<Velocity>(e));
    EXPECT_TRUE(registry.hasComponent<Health>(e));
}

TEST_F(RegistryComponentTest, EmplaceComponent_OnDeadEntity_Throws) {
    Entity e = registry.spawnEntity();
    registry.killEntity(e);

    EXPECT_THROW(
        registry.emplaceComponent<Position>(e, 0.0f, 0.0f),
        std::runtime_error
    );
}

TEST_F(RegistryComponentTest, EmplaceComponent_Replace_UpdatesValue) {
    Entity e = registry.spawnEntity();

    registry.emplaceComponent<Position>(e, 10.0f, 20.0f);
    registry.emplaceComponent<Position>(e, 30.0f, 40.0f);

    Position& pos = registry.getComponent<Position>(e);
    EXPECT_EQ(pos.x, 30.0f);
    EXPECT_EQ(pos.y, 40.0f);
}

TEST_F(RegistryComponentTest, EmplaceComponent_ResourceComponent) {
    Entity e = registry.spawnEntity();

    Resource& res = registry.emplaceComponent<Resource>(e, 123);

    EXPECT_NE(res.data, nullptr);
    EXPECT_EQ(*res.data, 123);
}

// ============================================================================
// GET OR EMPLACE TESTS
// ============================================================================

TEST_F(RegistryComponentTest, GetOrEmplace_NotExists_Creates) {
    Entity e = registry.spawnEntity();

    Position& pos = registry.getOrEmplace<Position>(e, 5.0f, 10.0f);

    EXPECT_EQ(pos.x, 5.0f);
    EXPECT_EQ(pos.y, 10.0f);
}

TEST_F(RegistryComponentTest, GetOrEmplace_Exists_ReturnsExisting) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e, 1.0f, 2.0f);

    // Args should be ignored since component exists
    Position& pos = registry.getOrEmplace<Position>(e, 100.0f, 200.0f);

    EXPECT_EQ(pos.x, 1.0f);
    EXPECT_EQ(pos.y, 2.0f);
}

TEST_F(RegistryComponentTest, GetOrEmplace_LazyInitialization) {
    Entity e = registry.spawnEntity();

    // First call creates
    Position& pos1 = registry.getOrEmplace<Position>(e);
    pos1.x = 42.0f;

    // Second call returns existing
    Position& pos2 = registry.getOrEmplace<Position>(e);

    EXPECT_EQ(pos2.x, 42.0f);
    EXPECT_EQ(&pos1, &pos2);
}

// ============================================================================
// HAS COMPONENT TESTS
// ============================================================================

TEST_F(RegistryComponentTest, HasComponent_Exists_ReturnsTrue) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);

    EXPECT_TRUE(registry.hasComponent<Position>(e));
}

TEST_F(RegistryComponentTest, HasComponent_NotExists_ReturnsFalse) {
    Entity e = registry.spawnEntity();

    EXPECT_FALSE(registry.hasComponent<Position>(e));
}

TEST_F(RegistryComponentTest, HasComponent_DeadEntity_ReturnsFalse) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.killEntity(e);

    EXPECT_FALSE(registry.hasComponent<Position>(e));
}

TEST_F(RegistryComponentTest, HasComponent_TagComponent) {
    Entity e = registry.spawnEntity();

    EXPECT_FALSE(registry.hasComponent<PlayerTag>(e));

    registry.emplaceComponent<PlayerTag>(e);

    EXPECT_TRUE(registry.hasComponent<PlayerTag>(e));
}

TEST_F(RegistryComponentTest, HasComponent_MultipleEntities) {
    Entity e1 = registry.spawnEntity();
    Entity e2 = registry.spawnEntity();

    registry.emplaceComponent<Position>(e1);

    EXPECT_TRUE(registry.hasComponent<Position>(e1));
    EXPECT_FALSE(registry.hasComponent<Position>(e2));
}

// ============================================================================
// GET COMPONENT TESTS
// ============================================================================

TEST_F(RegistryComponentTest, GetComponent_Exists_ReturnsReference) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e, 15.0f, 25.0f);

    Position& pos = registry.getComponent<Position>(e);

    EXPECT_EQ(pos.x, 15.0f);
    EXPECT_EQ(pos.y, 25.0f);
}

TEST_F(RegistryComponentTest, GetComponent_ModifyReference) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e, 0.0f, 0.0f);

    registry.getComponent<Position>(e).x = 100.0f;

    EXPECT_EQ(registry.getComponent<Position>(e).x, 100.0f);
}

TEST_F(RegistryComponentTest, GetComponent_NotExists_Throws) {
    Entity e = registry.spawnEntity();

    EXPECT_THROW(
        registry.getComponent<Position>(e),
        std::runtime_error
    );
}

TEST_F(RegistryComponentTest, GetComponent_DeadEntity_Throws) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.killEntity(e);

    EXPECT_THROW(
        registry.getComponent<Position>(e),
        std::runtime_error
    );
}

TEST_F(RegistryComponentTest, GetComponent_Const_Works) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e, 5.0f, 10.0f);

    const Registry& constReg = registry;
    const Position& pos = constReg.getComponent<Position>(e);

    EXPECT_EQ(pos.x, 5.0f);
    EXPECT_EQ(pos.y, 10.0f);
}

// ============================================================================
// REMOVE COMPONENT TESTS
// ============================================================================

TEST_F(RegistryComponentTest, RemoveComponent_Exists_Removes) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);

    registry.removeComponent<Position>(e);

    EXPECT_FALSE(registry.hasComponent<Position>(e));
}

TEST_F(RegistryComponentTest, RemoveComponent_NotExists_NoEffect) {
    Entity e = registry.spawnEntity();

    // Should not throw
    registry.removeComponent<Position>(e);

    EXPECT_FALSE(registry.hasComponent<Position>(e));
}

TEST_F(RegistryComponentTest, RemoveComponent_KeepsOtherComponents) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.emplaceComponent<Velocity>(e);
    registry.emplaceComponent<Health>(e);

    registry.removeComponent<Velocity>(e);

    EXPECT_TRUE(registry.hasComponent<Position>(e));
    EXPECT_FALSE(registry.hasComponent<Velocity>(e));
    EXPECT_TRUE(registry.hasComponent<Health>(e));
}

TEST_F(RegistryComponentTest, RemoveComponent_Resource_ProperlyDestructs) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Resource>(e, 999);

    // Should properly destruct the unique_ptr
    registry.removeComponent<Resource>(e);

    EXPECT_FALSE(registry.hasComponent<Resource>(e));
}

// ============================================================================
// CLEAR COMPONENTS TESTS
// ============================================================================

TEST_F(RegistryComponentTest, ClearComponents_RemovesFromAllEntities) {
    std::vector<Entity> entities;
    for (int i = 0; i < 10; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, static_cast<float>(i), 0.0f);
        entities.push_back(e);
    }

    registry.clearComponents<Position>();

    for (Entity e : entities) {
        EXPECT_FALSE(registry.hasComponent<Position>(e));
    }
}

TEST_F(RegistryComponentTest, ClearComponents_KeepsOtherTypes) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.emplaceComponent<Velocity>(e);

    registry.clearComponents<Position>();

    EXPECT_FALSE(registry.hasComponent<Position>(e));
    EXPECT_TRUE(registry.hasComponent<Velocity>(e));
}

TEST_F(RegistryComponentTest, ClearComponents_EmptyPool_NoEffect) {
    // Should not throw even if no Position components exist
    registry.clearComponents<Position>();
}

// ============================================================================
// COUNT COMPONENTS TESTS
// ============================================================================

TEST_F(RegistryComponentTest, CountComponents_ReturnsCorrectCount) {
    for (int i = 0; i < 5; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e);
    }

    EXPECT_EQ(registry.countComponents<Position>(), 5);
}

TEST_F(RegistryComponentTest, CountComponents_EmptyPool_ReturnsZero) {
    EXPECT_EQ(registry.countComponents<Position>(), 0);
}

TEST_F(RegistryComponentTest, CountComponents_AfterRemoval_UpdatesCount) {
    std::vector<Entity> entities;
    for (int i = 0; i < 10; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e);
        entities.push_back(e);
    }

    registry.removeComponent<Position>(entities[0]);
    registry.removeComponent<Position>(entities[1]);
    registry.removeComponent<Position>(entities[2]);

    EXPECT_EQ(registry.countComponents<Position>(), 7);
}

// ============================================================================
// PATCH TESTS
// ============================================================================

TEST_F(RegistryComponentTest, Patch_ModifiesComponent) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e, 10.0f, 20.0f);

    registry.patch<Position>(e, [](Position& pos) {
        pos.x += 5.0f;
        pos.y *= 2.0f;
    });

    Position& pos = registry.getComponent<Position>(e);
    EXPECT_EQ(pos.x, 15.0f);
    EXPECT_EQ(pos.y, 40.0f);
}

TEST_F(RegistryComponentTest, Patch_OnDeadEntity_Throws) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.killEntity(e);

    EXPECT_THROW(
        registry.patch<Position>(e, [](Position&) {}),
        std::runtime_error
    );
}

TEST_F(RegistryComponentTest, Patch_ComponentNotExists_Throws) {
    Entity e = registry.spawnEntity();

    EXPECT_THROW(
        registry.patch<Position>(e, [](Position&) {}),
        std::runtime_error
    );
}

// ============================================================================
// RESERVE AND COMPACT TESTS
// ============================================================================

TEST_F(RegistryComponentTest, ReserveComponents_DoesNotAffectCount) {
    registry.reserveComponents<Position>(1000);

    EXPECT_EQ(registry.countComponents<Position>(), 0);
}

TEST_F(RegistryComponentTest, Compact_AfterRemoval_Works) {
    std::vector<Entity> entities;
    for (int i = 0; i < 100; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e);
        entities.push_back(e);
    }

    // Remove half
    for (int i = 0; i < 50; ++i) {
        registry.removeComponent<Position>(entities[i]);
    }

    // Should not throw and should reclaim memory
    registry.compact();

    EXPECT_EQ(registry.countComponents<Position>(), 50);
}

TEST_F(RegistryComponentTest, CompactComponent_SpecificType) {
    for (int i = 0; i < 100; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e);
    }

    // Should not throw
    registry.compactComponent<Position>();
}

// ============================================================================
// ENTITY COMPONENTS TRACKING TESTS
// ============================================================================

TEST_F(RegistryComponentTest, GetEntityComponents_ReturnsAllTypes) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.emplaceComponent<Velocity>(e);
    registry.emplaceComponent<PlayerTag>(e);

    const auto& components = registry.getEntityComponents(e);

    EXPECT_EQ(components.size(), 3);
}

TEST_F(RegistryComponentTest, GetEntityComponents_EmptyEntity_ReturnsEmpty) {
    Entity e = registry.spawnEntity();

    const auto& components = registry.getEntityComponents(e);

    EXPECT_TRUE(components.empty());
}

TEST_F(RegistryComponentTest, GetEntityComponents_DeadEntity_ReturnsEmpty) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.killEntity(e);

    const auto& components = registry.getEntityComponents(e);

    EXPECT_TRUE(components.empty());
}

// ============================================================================
// KILL ENTITY COMPONENT CLEANUP TESTS
// ============================================================================

TEST_F(RegistryComponentTest, KillEntity_RemovesAllComponents) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<Position>(e);
    registry.emplaceComponent<Velocity>(e);
    registry.emplaceComponent<Health>(e);

    EXPECT_EQ(registry.countComponents<Position>(), 1);
    EXPECT_EQ(registry.countComponents<Velocity>(), 1);
    EXPECT_EQ(registry.countComponents<Health>(), 1);

    registry.killEntity(e);

    EXPECT_EQ(registry.countComponents<Position>(), 0);
    EXPECT_EQ(registry.countComponents<Velocity>(), 0);
    EXPECT_EQ(registry.countComponents<Health>(), 0);
}

// ============================================================================
// STRESS TESTS
// ============================================================================

TEST_F(RegistryComponentTest, StressTest_ManyEntitiesManyComponents) {
    constexpr size_t count = 1000;

    std::vector<Entity> entities;
    for (size_t i = 0; i < count; ++i) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, static_cast<float>(i), 0.0f);
        registry.emplaceComponent<Velocity>(e, 1.0f, 1.0f);
        if (i % 2 == 0) {
            registry.emplaceComponent<PlayerTag>(e);
        }
        entities.push_back(e);
    }

    EXPECT_EQ(registry.countComponents<Position>(), count);
    EXPECT_EQ(registry.countComponents<Velocity>(), count);
    EXPECT_EQ(registry.countComponents<PlayerTag>(), count / 2);

    // Remove every third entity
    for (size_t i = 0; i < count; i += 3) {
        registry.killEntity(entities[i]);
    }

    // Count remaining
    size_t remaining = 0;
    for (Entity e : entities) {
        if (registry.isAlive(e)) {
            remaining++;
        }
    }

    // Should be roughly 2/3 remaining
    EXPECT_GT(remaining, count / 2);
    EXPECT_LT(remaining, count);
}
