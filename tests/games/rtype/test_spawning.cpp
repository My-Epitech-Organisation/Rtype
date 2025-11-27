/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_spawning
*/

#include "../../../src/games/rtype/shared/Components.hpp"
#include <gtest/gtest.h>

using namespace rtype::games::rtype::shared;

TEST(ComponentsTest, TransformComponentDefault) {
    TransformComponent transform;

    EXPECT_FLOAT_EQ(transform.x, 0.0f);
    EXPECT_FLOAT_EQ(transform.y, 0.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 0.0f);
}

TEST(ComponentsTest, TransformComponentSetValues) {
    TransformComponent transform;
    transform.x = 10.0f;
    transform.y = 20.0f;
    transform.rotation = 45.0f;

    EXPECT_FLOAT_EQ(transform.x, 10.0f);
    EXPECT_FLOAT_EQ(transform.y, 20.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 45.0f);
}

TEST(ComponentsTest, VelocityComponentDefault) {
    VelocityComponent velocity;

    EXPECT_FLOAT_EQ(velocity.vx, 0.0f);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0f);
}

TEST(ComponentsTest, VelocityComponentSetValues) {
    VelocityComponent velocity;
    velocity.vx = 5.0f;
    velocity.vy = -3.0f;

    EXPECT_FLOAT_EQ(velocity.vx, 5.0f);
    EXPECT_FLOAT_EQ(velocity.vy, -3.0f);
}

TEST(ComponentsTest, NetworkIdComponentDefault) {
    NetworkIdComponent netId;

    EXPECT_EQ(netId.networkId, 0u);
}

TEST(ComponentsTest, NetworkIdComponentSetValue) {
    NetworkIdComponent netId;
    netId.networkId = 42;

    EXPECT_EQ(netId.networkId, 42u);
}

TEST(ComponentsTest, TransformComponentNegativeValues) {
    TransformComponent transform;
    transform.x = -100.0f;
    transform.y = -200.0f;
    transform.rotation = -90.0f;

    EXPECT_FLOAT_EQ(transform.x, -100.0f);
    EXPECT_FLOAT_EQ(transform.y, -200.0f);
    EXPECT_FLOAT_EQ(transform.rotation, -90.0f);
}

TEST(ComponentsTest, TransformComponentLargeValues) {
    TransformComponent transform;
    transform.x = 1000000.0f;
    transform.y = 1000000.0f;
    transform.rotation = 360.0f;

    EXPECT_FLOAT_EQ(transform.x, 1000000.0f);
    EXPECT_FLOAT_EQ(transform.y, 1000000.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 360.0f);
}

TEST(ComponentsTest, VelocityComponentNegativeValues) {
    VelocityComponent velocity;
    velocity.vx = -100.0f;
    velocity.vy = -200.0f;

    EXPECT_FLOAT_EQ(velocity.vx, -100.0f);
    EXPECT_FLOAT_EQ(velocity.vy, -200.0f);
}

TEST(ComponentsTest, VelocityComponentLargeValues) {
    VelocityComponent velocity;
    velocity.vx = 10000.0f;
    velocity.vy = 10000.0f;

    EXPECT_FLOAT_EQ(velocity.vx, 10000.0f);
    EXPECT_FLOAT_EQ(velocity.vy, 10000.0f);
}

TEST(ComponentsTest, NetworkIdComponentMaxValue) {
    NetworkIdComponent netId;
    netId.networkId = 4294967295u;  // Max unsigned int

    EXPECT_EQ(netId.networkId, 4294967295u);
}

TEST(ComponentsTest, MultipleTransformComponents) {
    TransformComponent t1, t2, t3;

    t1.x = 10.0f;
    t2.x = 20.0f;
    t3.x = 30.0f;

    // Ensure they are independent
    EXPECT_FLOAT_EQ(t1.x, 10.0f);
    EXPECT_FLOAT_EQ(t2.x, 20.0f);
    EXPECT_FLOAT_EQ(t3.x, 30.0f);
}

TEST(ComponentsTest, TransformComponentRotationWrap) {
    TransformComponent transform;
    transform.rotation = 720.0f;  // Two full rotations

    EXPECT_FLOAT_EQ(transform.rotation, 720.0f);
}

TEST(ComponentsTest, VelocityComponentZero) {
    VelocityComponent velocity;
    velocity.vx = 0.0f;
    velocity.vy = 0.0f;

    EXPECT_FLOAT_EQ(velocity.vx, 0.0f);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0f);
}

TEST(ComponentsTest, TransformComponentCopyValues) {
    TransformComponent original;
    original.x = 100.0f;
    original.y = 200.0f;
    original.rotation = 45.0f;

    TransformComponent copy = original;

    EXPECT_FLOAT_EQ(copy.x, 100.0f);
    EXPECT_FLOAT_EQ(copy.y, 200.0f);
    EXPECT_FLOAT_EQ(copy.rotation, 45.0f);
}

TEST(ComponentsTest, VelocityComponentCopyValues) {
    VelocityComponent original;
    original.vx = 50.0f;
    original.vy = -25.0f;

    VelocityComponent copy = original;

    EXPECT_FLOAT_EQ(copy.vx, 50.0f);
    EXPECT_FLOAT_EQ(copy.vy, -25.0f);
}

TEST(ComponentsTest, NetworkIdComponentCopyValue) {
    NetworkIdComponent original;
    original.networkId = 12345;

    NetworkIdComponent copy = original;

    EXPECT_EQ(copy.networkId, 12345u);
}
