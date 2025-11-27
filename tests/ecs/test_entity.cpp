/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_entity
*/

#include "../../src/engine/ecs/Entity.hpp"
#include <gtest/gtest.h>

using namespace rtype::engine::ecs;

TEST(EntityTest, DefaultConstructorCreatesInvalidEntity) {
    Entity entity;

    EXPECT_FALSE(entity.valid());
    EXPECT_EQ(entity.id(), 0u);
}

TEST(EntityTest, ConstructorWithIdCreatesValidEntity) {
    Entity entity(42);

    EXPECT_TRUE(entity.valid());
    EXPECT_EQ(entity.id(), 42u);
}

TEST(EntityTest, EntityWithIdZeroIsInvalid) {
    Entity entity(0);

    EXPECT_FALSE(entity.valid());
}

TEST(EntityTest, EqualityOperator) {
    Entity entity1(1);
    Entity entity2(1);
    Entity entity3(2);

    EXPECT_TRUE(entity1 == entity2);
    EXPECT_FALSE(entity1 == entity3);
}

TEST(EntityTest, InequalityOperator) {
    Entity entity1(1);
    Entity entity2(1);
    Entity entity3(2);

    EXPECT_FALSE(entity1 != entity2);
    EXPECT_TRUE(entity1 != entity3);
}

TEST(EntityTest, DefaultEntitiesAreEqual) {
    Entity entity1;
    Entity entity2;

    EXPECT_TRUE(entity1 == entity2);
}

TEST(EntityTest, LargeEntityId) {
    EntityId largeId = 4294967295u;  // Max unsigned int
    Entity entity(largeId);

    EXPECT_TRUE(entity.valid());
    EXPECT_EQ(entity.id(), largeId);
}

TEST(EntityTest, CopyConstructor) {
    Entity original(100);
    Entity copy(original);

    EXPECT_EQ(copy.id(), 100u);
    EXPECT_TRUE(copy.valid());
    EXPECT_TRUE(original == copy);
}

TEST(EntityTest, AssignmentOperator) {
    Entity entity1(50);
    Entity entity2(100);

    entity2 = entity1;

    EXPECT_EQ(entity2.id(), 50u);
    EXPECT_TRUE(entity1 == entity2);
}

TEST(EntityTest, SelfEquality) {
    Entity entity(42);

    EXPECT_TRUE(entity == entity);
    EXPECT_FALSE(entity != entity);
}

TEST(EntityTest, SmallEntityId) {
    Entity entity(1);

    EXPECT_TRUE(entity.valid());
    EXPECT_EQ(entity.id(), 1u);
}

TEST(EntityTest, ConsecutiveIds) {
    Entity entity1(1);
    Entity entity2(2);
    Entity entity3(3);

    EXPECT_EQ(entity1.id(), 1u);
    EXPECT_EQ(entity2.id(), 2u);
    EXPECT_EQ(entity3.id(), 3u);
}

TEST(EntityTest, InvalidEntityComparisons) {
    Entity invalid1;
    Entity invalid2;
    Entity valid(1);

    EXPECT_TRUE(invalid1 == invalid2);
    EXPECT_FALSE(invalid1 == valid);
    EXPECT_TRUE(invalid1 != valid);
}
