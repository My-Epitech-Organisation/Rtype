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
