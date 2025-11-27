/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_movement_system
*/

#include "../../../src/games/rtype/shared/Components.hpp"
#include "../../../src/games/rtype/shared/Systems/MovementSystem.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace rtype::games::rtype::shared;

TEST(MovementSystemTest, UpdateMovementBasic) {
    TransformComponent transform;
    transform.x = 0.0f;
    transform.y = 0.0f;

    VelocityComponent velocity;
    velocity.vx = 10.0f;
    velocity.vy = 5.0f;

    transform = updateMovement(transform, velocity, 1.0f);

    EXPECT_FLOAT_EQ(transform.x, 10.0f);
    EXPECT_FLOAT_EQ(transform.y, 5.0f);
}

TEST(MovementSystemTest, UpdateMovementWithDeltaTime) {
    TransformComponent transform;
    transform.x = 0.0f;
    transform.y = 0.0f;

    VelocityComponent velocity;
    velocity.vx = 100.0f;
    velocity.vy = 50.0f;

    // Simulate 60 FPS frame
    transform = updateMovement(transform, velocity, 0.016f);

    EXPECT_NEAR(transform.x, 1.6f, 0.001f);
    EXPECT_NEAR(transform.y, 0.8f, 0.001f);
}

TEST(MovementSystemTest, UpdateMovementNegativeVelocity) {
    TransformComponent transform;
    transform.x = 100.0f;
    transform.y = 100.0f;

    VelocityComponent velocity;
    velocity.vx = -20.0f;
    velocity.vy = -10.0f;

    transform = updateMovement(transform, velocity, 1.0f);

    EXPECT_FLOAT_EQ(transform.x, 80.0f);
    EXPECT_FLOAT_EQ(transform.y, 90.0f);
}

TEST(MovementSystemTest, UpdateMovementZeroVelocity) {
    TransformComponent transform;
    transform.x = 50.0f;
    transform.y = 25.0f;

    VelocityComponent velocity;
    velocity.vx = 0.0f;
    velocity.vy = 0.0f;

    transform = updateMovement(transform, velocity, 1.0f);

    EXPECT_FLOAT_EQ(transform.x, 50.0f);
    EXPECT_FLOAT_EQ(transform.y, 25.0f);
}

TEST(MovementSystemTest, UpdateMovementZeroDeltaTime) {
    TransformComponent transform;
    transform.x = 50.0f;
    transform.y = 25.0f;

    VelocityComponent velocity;
    velocity.vx = 100.0f;
    velocity.vy = 100.0f;

    transform = updateMovement(transform, velocity, 0.0f);

    // Position should not change with zero delta time
    EXPECT_FLOAT_EQ(transform.x, 50.0f);
    EXPECT_FLOAT_EQ(transform.y, 25.0f);
}

TEST(MovementSystemTest, UpdateMovementMultipleFrames) {
    TransformComponent transform;
    transform.x = 0.0f;
    transform.y = 0.0f;

    VelocityComponent velocity;
    velocity.vx = 10.0f;
    velocity.vy = 10.0f;

    // Simulate 10 frames at 60 FPS
    for (int i = 0; i < 10; ++i) {
        transform = updateMovement(transform, velocity, 0.016f);
    }

    // After 10 frames: 10 * 10 * 0.016 = 1.6
    EXPECT_NEAR(transform.x, 1.6f, 0.01f);
    EXPECT_NEAR(transform.y, 1.6f, 0.01f);
}

TEST(MovementSystemTest, UpdateMovementDiagonal) {
    TransformComponent transform;
    transform.x = 0.0f;
    transform.y = 0.0f;

    // 45-degree diagonal movement (normalized)
    float speed = 10.0f;
    float diag = speed / std::sqrt(2.0f);

    VelocityComponent velocity;
    velocity.vx = diag;
    velocity.vy = diag;

    transform = updateMovement(transform, velocity, 1.0f);

    // Distance traveled should be 'speed'
    float distance = std::sqrt(transform.x * transform.x + transform.y * transform.y);
    EXPECT_NEAR(distance, speed, 0.001f);
}

TEST(MovementSystemTest, UpdateMovementLargeValues) {
    TransformComponent transform;
    transform.x = 1000000.0f;
    transform.y = 1000000.0f;

    VelocityComponent velocity;
    velocity.vx = 5000.0f;
    velocity.vy = 5000.0f;

    transform = updateMovement(transform, velocity, 1.0f);

    EXPECT_FLOAT_EQ(transform.x, 1005000.0f);
    EXPECT_FLOAT_EQ(transform.y, 1005000.0f);
}

TEST(MovementSystemTest, UpdateMovementSmallDeltaTime) {
    TransformComponent transform;
    transform.x = 0.0f;
    transform.y = 0.0f;

    VelocityComponent velocity;
    velocity.vx = 1000.0f;
    velocity.vy = 1000.0f;

    // Very small delta time (high FPS)
    transform = updateMovement(transform, velocity, 0.001f);

    EXPECT_FLOAT_EQ(transform.x, 1.0f);
    EXPECT_FLOAT_EQ(transform.y, 1.0f);
}

TEST(MovementSystemTest, RotationNotAffectedByMovement) {
    TransformComponent transform;
    transform.x = 0.0f;
    transform.y = 0.0f;
    transform.rotation = 45.0f;

    VelocityComponent velocity;
    velocity.vx = 10.0f;
    velocity.vy = 10.0f;

    transform = updateMovement(transform, velocity, 1.0f);

    // Rotation should remain unchanged
    EXPECT_FLOAT_EQ(transform.rotation, 45.0f);
}
