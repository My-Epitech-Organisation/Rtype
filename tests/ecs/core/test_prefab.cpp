/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PrefabManager branch coverage tests
*/

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "../../../lib/rtype_ecs/src/core/Prefab.hpp"
#include "../../../lib/rtype_ecs/src/core/Registry/Registry.hpp"

using namespace ECS;

namespace {
struct DummyComponent {
    int value = 0;
};
}  // namespace

class PrefabManagerTest : public ::testing::Test {
   protected:
    Registry registry;
    PrefabManager manager{registry};
};

TEST_F(PrefabManagerTest, InstantiateUnknownPrefabThrows) {
    EXPECT_THROW(manager.instantiate("missing"), std::runtime_error);
}

TEST_F(PrefabManagerTest, InstantiateMultipleUnknownPrefabThrows) {
    EXPECT_THROW(manager.instantiateMultiple("ghost", 2), std::runtime_error);
}

TEST_F(PrefabManagerTest, CreateFromDeadEntityThrows) {
    Entity e = registry.spawnEntity();
    registry.killEntity(e);

    EXPECT_THROW(manager.createFromEntity("dead", e), std::runtime_error);
}

TEST_F(PrefabManagerTest, CreateFromEntityWithNoComponentsThrows) {
    Entity e = registry.spawnEntity();

    EXPECT_THROW(manager.createFromEntity("empty", e), std::runtime_error);
}

TEST_F(PrefabManagerTest, CreateFromEntityNotImplementedPathThrows) {
    Entity e = registry.spawnEntity();
    registry.emplaceComponent<DummyComponent>(e, DummyComponent{42});

    EXPECT_THROW(manager.createFromEntity("full", e), std::runtime_error);
}

TEST_F(PrefabManagerTest, RegisterAndInstantiateAddsComponent) {
    manager.registerPrefab("dummy", [](Registry& reg, Entity entity) {
        reg.emplaceComponent<DummyComponent>(entity, DummyComponent{5});
    });

    auto entity = manager.instantiate("dummy");

    EXPECT_TRUE(manager.hasPrefab("dummy"));
    EXPECT_TRUE(registry.hasComponent<DummyComponent>(entity));
    EXPECT_EQ(registry.getComponent<DummyComponent>(entity).value, 5);
}

TEST_F(PrefabManagerTest, InstantiateWithCustomizerAppliesExtraWork) {
    manager.registerPrefab("base", [](Registry& reg, Entity entity) {
        reg.emplaceComponent<DummyComponent>(entity, DummyComponent{1});
    });

    auto entity = manager.instantiate(
        "base", [](Registry& reg, Entity e) {
            reg.getComponent<DummyComponent>(e).value = 7;
        });

    EXPECT_EQ(registry.getComponent<DummyComponent>(entity).value, 7);
}

TEST_F(PrefabManagerTest, InstantiateMultipleCreatesAllEntities) {
    manager.registerPrefab("multi", [](Registry& reg, Entity entity) {
        reg.emplaceComponent<DummyComponent>(entity, DummyComponent{3});
    });

    auto entities = manager.instantiateMultiple("multi", 3);

    EXPECT_EQ(entities.size(), 3u);
    for (auto entity : entities) {
        EXPECT_TRUE(registry.hasComponent<DummyComponent>(entity));
        EXPECT_EQ(registry.getComponent<DummyComponent>(entity).value, 3);
    }
}

TEST_F(PrefabManagerTest, UnregisterRemovesPrefab) {
    manager.registerPrefab("temp", [](Registry& reg, Entity entity) {
        reg.emplaceComponent<DummyComponent>(entity, DummyComponent{9});
    });

    manager.unregisterPrefab("temp");

    EXPECT_FALSE(manager.hasPrefab("temp"));
    EXPECT_THROW(manager.instantiate("temp"), std::runtime_error);
}

TEST_F(PrefabManagerTest, ClearRemovesAllPrefabs) {
    manager.registerPrefab("a", [](Registry&, Entity) {});
    manager.registerPrefab("b", [](Registry&, Entity) {});

    manager.clear();

    EXPECT_FALSE(manager.hasPrefab("a"));
    EXPECT_FALSE(manager.hasPrefab("b"));
    EXPECT_TRUE(manager.getPrefabNames().empty());
}

TEST_F(PrefabManagerTest, GetPrefabNamesIsSorted) {
    manager.registerPrefab("zeta", [](Registry&, Entity) {});
    manager.registerPrefab("alpha", [](Registry&, Entity) {});

    auto names = manager.getPrefabNames();

    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "alpha");
    EXPECT_EQ(names[1], "zeta");
}
