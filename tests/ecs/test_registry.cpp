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

TEST(RegistryTest, CreateMultipleEntities) {
    Registry registry;

    auto entity1 = registry.createEntity();
    auto entity2 = registry.createEntity();
    auto entity3 = registry.createEntity();

    EXPECT_NE(entity1.id(), entity2.id());
    EXPECT_NE(entity2.id(), entity3.id());
    EXPECT_NE(entity1.id(), entity3.id());
}

TEST(RegistryTest, EntitiesHaveIncreasingIds) {
    Registry registry;

    auto entity1 = registry.createEntity();
    auto entity2 = registry.createEntity();
    auto entity3 = registry.createEntity();

    EXPECT_LT(entity1.id(), entity2.id());
    EXPECT_LT(entity2.id(), entity3.id());
}

TEST(RegistryTest, DestroyMultipleEntities) {
    Registry registry;
    auto entity1 = registry.createEntity();
    auto entity2 = registry.createEntity();
    auto entity3 = registry.createEntity();

    EXPECT_EQ(registry.entityCount(), 3u);

    registry.destroyEntity(entity1);
    EXPECT_EQ(registry.entityCount(), 2u);

    registry.destroyEntity(entity2);
    EXPECT_EQ(registry.entityCount(), 1u);

    registry.destroyEntity(entity3);
    EXPECT_EQ(registry.entityCount(), 0u);
}

TEST(RegistryTest, ClearAndRecreateEntities) {
    Registry registry;
    registry.createEntity();
    registry.createEntity();
    registry.clear();

    EXPECT_EQ(registry.entityCount(), 0u);

    auto newEntity = registry.createEntity();
    EXPECT_TRUE(newEntity.valid());
    EXPECT_EQ(registry.entityCount(), 1u);
}

TEST(RegistryTest, EmptyRegistryHasZeroCount) {
    Registry registry;
    EXPECT_EQ(registry.entityCount(), 0u);
}

TEST(RegistryTest, DestroyInvalidEntity) {
    Registry registry;
    Entity invalidEntity;  // Default invalid entity

    // Should not crash or throw
    registry.destroyEntity(invalidEntity);
    EXPECT_EQ(registry.entityCount(), 0u);
}

TEST(RegistryTest, DestroyNonExistentEntity) {
    Registry registry;
    registry.createEntity();
    Entity nonExistent(9999);

    // Should not affect the registry
    registry.destroyEntity(nonExistent);
    EXPECT_EQ(registry.entityCount(), 1u);
}

TEST(RegistryTest, DestroyEntityTwice) {
    Registry registry;
    auto entity = registry.createEntity();

    registry.destroyEntity(entity);
    EXPECT_EQ(registry.entityCount(), 0u);

    // Destroying again should not crash
    registry.destroyEntity(entity);
    EXPECT_EQ(registry.entityCount(), 0u);
}

TEST(RegistryTest, CreateManyEntities) {
    Registry registry;

    for (int i = 0; i < 1000; ++i) {
        auto entity = registry.createEntity();
        EXPECT_TRUE(entity.valid());
    }

    EXPECT_EQ(registry.entityCount(), 1000u);
}

TEST(RegistryTest, DestroyMiddleEntity) {
    Registry registry;
    auto entity1 = registry.createEntity();
    auto entity2 = registry.createEntity();
    auto entity3 = registry.createEntity();

    registry.destroyEntity(entity2);

    EXPECT_EQ(registry.entityCount(), 2u);
}

TEST(RegistryTest, ClearEmptyRegistry) {
    Registry registry;

    // Should not crash
    registry.clear();
    EXPECT_EQ(registry.entityCount(), 0u);
}

TEST(RegistryTest, ClearMultipleTimes) {
    Registry registry;
    registry.createEntity();
    registry.createEntity();

    registry.clear();
    EXPECT_EQ(registry.entityCount(), 0u);

    registry.clear();
    EXPECT_EQ(registry.entityCount(), 0u);
}

TEST(RegistryTest, CreateAfterClearResetsIds) {
    Registry registry;
    auto entity1 = registry.createEntity();
    auto id1 = entity1.id();

    registry.clear();
    auto entity2 = registry.createEntity();

    // After clear, IDs should reset to 1
    EXPECT_EQ(entity2.id(), 1u);
}

TEST(RegistryTest, DestroyInReverseOrder) {
    Registry registry;
    auto entity1 = registry.createEntity();
    auto entity2 = registry.createEntity();
    auto entity3 = registry.createEntity();

    registry.destroyEntity(entity3);
    registry.destroyEntity(entity2);
    registry.destroyEntity(entity1);

    EXPECT_EQ(registry.entityCount(), 0u);
}
