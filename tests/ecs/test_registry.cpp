/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_registry
*/

#include "../../src/engine/ecs/Registry.hpp"
#include <gtest/gtest.h>

using namespace rtype::engine::ecs;

TEST(RegistryTest, CreateEntity) {
    Registry registry;
    auto entity = registry.createEntity();

    EXPECT_TRUE(entity.valid());
    EXPECT_GT(entity.id(), 0u);
}

TEST(RegistryTest, EntityCount) {
    Registry registry;
    EXPECT_EQ(registry.entityCount(), 0u);

    registry.createEntity();
    EXPECT_EQ(registry.entityCount(), 1u);

    registry.createEntity();
    EXPECT_EQ(registry.entityCount(), 2u);
}

TEST(RegistryTest, DestroyEntity) {
    Registry registry;
    auto entity = registry.createEntity();

    EXPECT_EQ(registry.entityCount(), 1u);

    registry.destroyEntity(entity);
    EXPECT_EQ(registry.entityCount(), 0u);
}

TEST(RegistryTest, Clear) {
    Registry registry;
    registry.createEntity();
    registry.createEntity();
    registry.createEntity();

    EXPECT_EQ(registry.entityCount(), 3u);

    registry.clear();
    EXPECT_EQ(registry.entityCount(), 0u);
}
