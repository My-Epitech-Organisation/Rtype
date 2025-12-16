/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_ProjectileSystem - Unit tests for ProjectileSystem
*/

#include <gtest/gtest.h>
#include "../../../src/games/rtype/shared/Systems/Projectile/ProjectileSystem.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "../../../src/games/rtype/shared/Components/VelocityComponent.hpp"
#include "../../../src/games/rtype/shared/Components/Tags.hpp"
#include "../../../../lib/ecs/src/ECS.hpp"

using namespace rtype::games::rtype::shared;

class ProjectileSystemTest : public ::testing::Test {
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
    ProjectileSystem projectileSystem;
};

TEST_F(ProjectileSystemTest, UpdateMovesProjectileWithPositiveVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0f, 50.0f);
    registry.emplaceComponent<ProjectileTag>(entity);

    projectileSystem.update(registry, 1.0f);

    auto& position = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(position.x, 100.0f);
    EXPECT_FLOAT_EQ(position.y, 50.0f);
}

TEST_F(ProjectileSystemTest, UpdateMovesProjectileWithNegativeVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 100.0f, 100.0f);
    registry.emplaceComponent<VelocityComponent>(entity, -50.0f, -25.0f);
    registry.emplaceComponent<ProjectileTag>(entity);

    projectileSystem.update(registry, 1.0f);

    auto& position = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(position.x, 50.0f);
    EXPECT_FLOAT_EQ(position.y, 75.0f);
}

TEST_F(ProjectileSystemTest, UpdateWithZeroVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 100.0f, 100.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0f, 0.0f);
    registry.emplaceComponent<ProjectileTag>(entity);

    projectileSystem.update(registry, 1.0f);

    auto& position = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(position.x, 100.0f);
    EXPECT_FLOAT_EQ(position.y, 100.0f);
}

TEST_F(ProjectileSystemTest, UpdateWithZeroDeltaTime) {
    registry.emplaceComponent<TransformComponent>(entity, 50.0f, 50.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0f, 100.0f);
    registry.emplaceComponent<ProjectileTag>(entity);

    projectileSystem.update(registry, 0.0f);

    auto& position = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(position.x, 50.0f);
    EXPECT_FLOAT_EQ(position.y, 50.0f);
}

TEST_F(ProjectileSystemTest, UpdateIgnoresNonProjectileEntities) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0f, 100.0f);
    // No ProjectileTag

    projectileSystem.update(registry, 1.0f);

    auto& position = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(position.x, 0.0f);
    EXPECT_FLOAT_EQ(position.y, 0.0f);
}

TEST_F(ProjectileSystemTest, UpdateGracefullyHandlesMissingVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f);
    registry.emplaceComponent<ProjectileTag>(entity);
    // No VelocityComponent

    projectileSystem.update(registry, 1.0f);

    auto& position = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(position.x, 0.0f);
    EXPECT_FLOAT_EQ(position.y, 0.0f);
}

TEST_F(ProjectileSystemTest, UpdateWithNegativeDeltaTime) {
    registry.emplaceComponent<TransformComponent>(entity, 50.0f, 50.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0f, 100.0f);
    registry.emplaceComponent<ProjectileTag>(entity);

    projectileSystem.update(registry, -1.0f);

    auto& position = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(position.x, 50.0f);
    EXPECT_FLOAT_EQ(position.y, 50.0f);
}
