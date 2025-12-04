/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_movement_system
*/

#include <gtest/gtest.h>

#include "../../../src/games/rtype/shared/Components.hpp"
#include "../../../src/games/rtype/shared/Systems/MovementSystem.cpp"

using namespace rtype::games::rtype::shared;

TEST(MovementSystemTest, UpdateMovement_StationaryEntity) {
    TransformComponent transform{10.0f, 20.0f, 45.0f};
    VelocityComponent velocity{0.0f, 0.0f};
    const float deltaTime = 1.0f;

    updateMovement(transform, velocity, deltaTime);

    EXPECT_FLOAT_EQ(transform.x, 10.0f);
    EXPECT_FLOAT_EQ(transform.y, 20.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 45.0f);  // Should not change
}

TEST(MovementSystemTest, UpdateMovement_ConstantVelocity) {
    TransformComponent transform{0.0f, 0.0f, 0.0f};
    VelocityComponent velocity{5.0f, -3.0f};
    const float deltaTime = 1.0f;

    updateMovement(transform, velocity, deltaTime);

    EXPECT_FLOAT_EQ(transform.x, 5.0f);
    EXPECT_FLOAT_EQ(transform.y, -3.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 0.0f);
}

TEST(MovementSystemTest, UpdateMovement_FractionalDeltaTime) {
    TransformComponent transform{100.0f, 50.0f, 90.0f};
    VelocityComponent velocity{10.0f, 20.0f};
    const float deltaTime = 0.5f;

    updateMovement(transform, velocity, deltaTime);

    EXPECT_FLOAT_EQ(transform.x, 105.0f);
    EXPECT_FLOAT_EQ(transform.y, 60.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 90.0f);
}

TEST(MovementSystemTest, UpdateMovement_NegativeVelocity) {
    TransformComponent transform{0.0f, 0.0f, 0.0f};
    VelocityComponent velocity{-2.0f, -4.0f};
    const float deltaTime = 2.0f;

    updateMovement(transform, velocity, deltaTime);

    EXPECT_FLOAT_EQ(transform.x, -4.0f);
    EXPECT_FLOAT_EQ(transform.y, -8.0f);
}

TEST(MovementSystemTest, UpdateMovement_ZeroDeltaTime) {
    TransformComponent transform{5.0f, 10.0f, 30.0f};
    VelocityComponent velocity{1.0f, 2.0f};
    const float deltaTime = 0.0f;

    updateMovement(transform, velocity, deltaTime);

    EXPECT_FLOAT_EQ(transform.x, 5.0f);
    EXPECT_FLOAT_EQ(transform.y, 10.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 30.0f);
}

TEST(MovementSystemTest, UpdateMovement_HighPrecision) {
    TransformComponent transform{0.0f, 0.0f, 0.0f};
    VelocityComponent velocity{1.5f, -2.25f};
    const float deltaTime = 0.016f;  // ~60 FPS

    updateMovement(transform, velocity, deltaTime);

    EXPECT_NEAR(transform.x, 0.024f, 0.001f);
    EXPECT_NEAR(transform.y, -0.036f, 0.001f);
}