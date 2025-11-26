/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_registry
*/

#include "ecs/ECS.hpp"
#include <gtest/gtest.h>

using namespace ECS;

TEST(RegistryTest, SpawnEntity) {
    Registry registry;
    auto entity = registry.spawnEntity();

    EXPECT_FALSE(entity.isNull());
    EXPECT_TRUE(registry.isAlive(entity));
}

TEST(RegistryTest, SpawnMultipleEntities) {
    Registry registry;

    auto entity1 = registry.spawnEntity();
    auto entity2 = registry.spawnEntity();

    EXPECT_TRUE(registry.isAlive(entity1));
    EXPECT_TRUE(registry.isAlive(entity2));
    EXPECT_NE(entity1.id, entity2.id);
}

TEST(RegistryTest, KillEntity) {
    Registry registry;
    auto entity = registry.spawnEntity();

    EXPECT_TRUE(registry.isAlive(entity));

    registry.killEntity(entity);
    EXPECT_FALSE(registry.isAlive(entity));
}

TEST(RegistryTest, EntityGenerations) {
    Registry registry;
    auto entity1 = registry.spawnEntity();
    auto idx = entity1.index();

    registry.killEntity(entity1);

    auto entity2 = registry.spawnEntity();

    // The new entity should reuse the index but have a different generation
    if (entity2.index() == idx) {
        EXPECT_NE(entity1.generation(), entity2.generation());
    }
}
