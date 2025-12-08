/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_movement_system
*/

#include <gtest/gtest.h>

#include "../../../src/games/rtype/shared/Components.hpp"
#include "../../../src/games/rtype/shared/Systems/Movements/MovementSystem.hpp"
#include "../../../../lib/rtype_ecs/src/ECS.hpp"

using namespace rtype::games::rtype::shared;

class MovementSystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        entity = registry.spawnEntity();
    }

    void TearDown() override {
        if (registry.isAlive(entity)) {
            registry.killEntity(entity);
        }
    }

    ECS::Registry registry;
    ECS::Entity entity;
    MovementSystem movementSystem;
};

TEST_F(MovementSystemTest, UpdateMovement_StationaryEntity) {
    registry.emplaceComponent<TransformComponent>(entity, 10.0f, 20.0f, 45.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0f, 0.0f);
    const float deltaTime = 1.0f;

    movementSystem.update(registry, deltaTime);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 10.0f);
    EXPECT_FLOAT_EQ(transform.y, 20.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 45.0f);  // Should not change
}

TEST_F(MovementSystemTest, UpdateMovement_ConstantVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f, 0.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 5.0f, -3.0f);
    const float deltaTime = 1.0f;

    movementSystem.update(registry, deltaTime);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 5.0f);
    EXPECT_FLOAT_EQ(transform.y, -3.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 0.0f);
}

TEST_F(MovementSystemTest, UpdateMovement_FractionalDeltaTime) {
    registry.emplaceComponent<TransformComponent>(entity, 100.0f, 50.0f, 90.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 10.0f, 20.0f);
    const float deltaTime = 0.5f;

    movementSystem.update(registry, deltaTime);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 105.0f);
    EXPECT_FLOAT_EQ(transform.y, 60.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 90.0f);
}

TEST_F(MovementSystemTest, UpdateMovement_NegativeVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f, 0.0f);
    registry.emplaceComponent<VelocityComponent>(entity, -2.0f, -4.0f);
    const float deltaTime = 2.0f;

    movementSystem.update(registry, deltaTime);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, -4.0f);
    EXPECT_FLOAT_EQ(transform.y, -8.0f);
}

TEST_F(MovementSystemTest, UpdateMovement_ZeroDeltaTime) {
    registry.emplaceComponent<TransformComponent>(entity, 5.0f, 10.0f, 30.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 1.0f, 2.0f);
    const float deltaTime = 0.0f;

    movementSystem.update(registry, deltaTime);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 5.0f);
    EXPECT_FLOAT_EQ(transform.y, 10.0f);
    EXPECT_FLOAT_EQ(transform.rotation, 30.0f);
}

TEST_F(MovementSystemTest, UpdateMovement_HighPrecision) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f, 0.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 1.5f, -2.25f);
    const float deltaTime = 0.016f;  // ~60 FPS

    movementSystem.update(registry, deltaTime);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_NEAR(transform.x, 0.024f, 0.001f);
    EXPECT_NEAR(transform.y, -0.036f, 0.001f);
}