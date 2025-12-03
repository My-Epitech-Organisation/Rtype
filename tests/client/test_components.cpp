/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Client Components
*/

#include <gtest/gtest.h>
#include "../src/client/Components/Common/PositionComponent.hpp"
#include "../src/client/Components/Graphic/VelocityComponent.hpp"

TEST(PositionComponentTest, DefaultConstructor_SetsZero) {
    Position pos(0.0f, 0.0f);
    EXPECT_EQ(pos.x, 0.0f);
    EXPECT_EQ(pos.y, 0.0f);
    EXPECT_EQ(pos.initialX, 0.0f);
    EXPECT_EQ(pos.initialY, 0.0f);
}

TEST(PositionComponentTest, ParameterizedConstructor_SetsValues) {
    Position pos(10.0f, 20.0f);
    EXPECT_EQ(pos.x, 10.0f);
    EXPECT_EQ(pos.y, 20.0f);
    EXPECT_EQ(pos.initialX, 10.0f);
    EXPECT_EQ(pos.initialY, 20.0f);
}

TEST(VelocityComponentTest, DefaultConstructor_SetsZero) {
    Velocity vel;
    EXPECT_EQ(vel.x, 0.0f);
    EXPECT_EQ(vel.y, 0.0f);
}

TEST(VelocityComponentTest, ParameterizedConstructor_SetsValues) {
    Velocity vel(5.0f, -3.0f);
    EXPECT_EQ(vel.x, 5.0f);
    EXPECT_EQ(vel.y, -3.0f);
}
