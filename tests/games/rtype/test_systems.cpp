/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_systems - Unit tests for shared and server systems
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <vector>

#include "../../../src/games/rtype/shared/Components.hpp"
#include "../../../src/games/rtype/shared/Systems/AISystem/AISystem.hpp"
#include "../../../src/games/rtype/shared/Systems/AISystem/Behaviors/BehaviorRegistry.hpp"
#include "../../../src/games/rtype/shared/Systems/Movements/MovementSystem.hpp"
#include "../../../src/games/rtype/server/Systems/Cleanup/CleanupSystem.hpp"
#include "../../../src/games/rtype/server/Systems/Destroy/DestroySystem.hpp"
#include "../../../src/games/rtype/server/Systems/Spawner/SpawnerSystem.hpp"
#include "../../../../lib/ecs/src/ECS.hpp"
#include "../../../../lib/engine/src/IGameEngine.hpp"

using namespace rtype::games::rtype::shared;
using namespace rtype::games::rtype::server;

// =============================================================================
// MovementSystem Tests
// =============================================================================

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

TEST_F(MovementSystemTest, GetNameReturnsCorrectName) {
    EXPECT_EQ(movementSystem.getName(), "MovementSystem");
}

TEST_F(MovementSystemTest, UpdateMovesEntityWithPositiveVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0F, 0.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0F, 50.0F);

    movementSystem.update(registry, 1.0F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 100.0F);
    EXPECT_FLOAT_EQ(transform.y, 50.0F);
}

TEST_F(MovementSystemTest, UpdateMovesEntityWithNegativeVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 100.0F, 100.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, -50.0F, -25.0F);

    movementSystem.update(registry, 1.0F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 50.0F);
    EXPECT_FLOAT_EQ(transform.y, 75.0F);
}

TEST_F(MovementSystemTest, UpdateWithZeroVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 100.0F, 100.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0F, 0.0F);

    movementSystem.update(registry, 1.0F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 100.0F);
    EXPECT_FLOAT_EQ(transform.y, 100.0F);
}

TEST_F(MovementSystemTest, UpdateWithSmallDeltaTime) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0F, 0.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0F, 100.0F);

    movementSystem.update(registry, 0.016F);  // ~60 FPS

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_NEAR(transform.x, 1.6F, 0.01F);
    EXPECT_NEAR(transform.y, 1.6F, 0.01F);
}

TEST_F(MovementSystemTest, UpdateWithZeroDeltaTime) {
    registry.emplaceComponent<TransformComponent>(entity, 50.0F, 50.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0F, 100.0F);

    movementSystem.update(registry, 0.0F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 50.0F);
    EXPECT_FLOAT_EQ(transform.y, 50.0F);
}

TEST_F(MovementSystemTest, UpdateMultipleEntities) {
    auto entity2 = registry.spawnEntity();

    registry.emplaceComponent<TransformComponent>(entity, 0.0F, 0.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0F, 0.0F);

    registry.emplaceComponent<TransformComponent>(entity2, 0.0F, 0.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity2, 0.0F, 100.0F);

    movementSystem.update(registry, 1.0F);

    auto& transform1 = registry.getComponent<TransformComponent>(entity);
    auto& transform2 = registry.getComponent<TransformComponent>(entity2);

    EXPECT_FLOAT_EQ(transform1.x, 100.0F);
    EXPECT_FLOAT_EQ(transform1.y, 0.0F);
    EXPECT_FLOAT_EQ(transform2.x, 0.0F);
    EXPECT_FLOAT_EQ(transform2.y, 100.0F);

    registry.killEntity(entity2);
}

TEST_F(MovementSystemTest, UpdateDoesNotAffectEntitiesWithoutVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 50.0F, 50.0F, 0.0F);
    // No velocity component

    movementSystem.update(registry, 1.0F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 50.0F);
    EXPECT_FLOAT_EQ(transform.y, 50.0F);
}

TEST_F(MovementSystemTest, UpdateDoesNotAffectRotation) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0F, 0.0F, 45.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0F, 100.0F);

    movementSystem.update(registry, 1.0F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.rotation, 45.0F);
}

TEST_F(MovementSystemTest, UpdateWithEmptyRegistry) {
    // Kill the entity created in SetUp
    registry.killEntity(entity);

    // Should not crash with empty registry
    EXPECT_NO_THROW(movementSystem.update(registry, 0.016F));

    // Re-create entity for TearDown
    entity = registry.spawnEntity();
}

TEST_F(MovementSystemTest, UpdateWithEntityMissingTransform) {
    // Entity with only VelocityComponent
    registry.emplaceComponent<VelocityComponent>(entity, 100.0F, 100.0F);
    // No TransformComponent

    // Should not crash
    EXPECT_NO_THROW(movementSystem.update(registry, 0.016F));
}

TEST_F(MovementSystemTest, UpdateWithLargeDeltaTime) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0F, 0.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0F, 100.0F);

    movementSystem.update(registry, 100.0F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 10000.0F);
    EXPECT_FLOAT_EQ(transform.y, 10000.0F);
}

TEST_F(MovementSystemTest, UpdateWithNegativeDeltaTime) {
    registry.emplaceComponent<TransformComponent>(entity, 100.0F, 100.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 100.0F, 100.0F);

    // Negative deltaTime (edge case)
    movementSystem.update(registry, -0.5F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_FLOAT_EQ(transform.x, 50.0F);
    EXPECT_FLOAT_EQ(transform.y, 50.0F);
}

TEST_F(MovementSystemTest, UpdateWithVerySmallVelocity) {
    registry.emplaceComponent<TransformComponent>(entity, 0.0F, 0.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.001F, 0.001F);

    movementSystem.update(registry, 1.0F);

    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_NEAR(transform.x, 0.001F, 0.0001F);
    EXPECT_NEAR(transform.y, 0.001F, 0.0001F);
}

// =============================================================================
// AISystem Tests
// =============================================================================

class AISystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        BehaviorRegistry::instance().clear();
        registerDefaultBehaviors();
        entity = registry.spawnEntity();
    }

    void TearDown() override {
        if (registry.isAlive(entity)) {
            registry.killEntity(entity);
        }
        BehaviorRegistry::instance().clear();
    }

    ECS::Registry registry;
    ECS::Entity entity;
    AISystem aiSystem;
};

TEST_F(AISystemTest, GetNameReturnsCorrectName) {
    EXPECT_EQ(aiSystem.getName(), "AISystem");
}

TEST_F(AISystemTest, UpdateWithMoveLeftBehavior) {
    registry.emplaceComponent<AIComponent>(entity, AIBehavior::MoveLeft, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0F, 0.0F);

    aiSystem.update(registry, 0.016F);

    auto& velocity = registry.getComponent<VelocityComponent>(entity);
    EXPECT_FLOAT_EQ(velocity.vx, -100.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(AISystemTest, UpdateWithStationaryBehavior) {
    registry.emplaceComponent<AIComponent>(entity, AIBehavior::Stationary, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 50.0F, 50.0F);

    aiSystem.update(registry, 0.016F);

    auto& velocity = registry.getComponent<VelocityComponent>(entity);
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(AISystemTest, UpdateWithChaseBehavior) {
    AIComponent ai;
    ai.behavior = AIBehavior::Chase;
    ai.speed = 100.0F;
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;

    registry.emplaceComponent<AIComponent>(entity, ai);
    registry.emplaceComponent<TransformComponent>(entity, 100.0F, 0.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0F, 0.0F);

    aiSystem.update(registry, 0.016F);

    auto& velocity = registry.getComponent<VelocityComponent>(entity);
    EXPECT_LT(velocity.vx, 0.0F);  // Moving toward target (left)
}

TEST_F(AISystemTest, UpdateWithSineWaveBehavior) {
    registry.emplaceComponent<AIComponent>(entity, AIBehavior::SineWave, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0F, 0.0F);

    aiSystem.update(registry, 0.016F);

    auto& velocity = registry.getComponent<VelocityComponent>(entity);
    EXPECT_FLOAT_EQ(velocity.vx, -100.0F);
    // Y velocity should have some sine wave component
}

TEST_F(AISystemTest, UpdateWithPatrolBehavior) {
    registry.emplaceComponent<AIComponent>(entity, AIBehavior::Patrol, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0F, 0.0F);

    aiSystem.update(registry, 0.016F);

    auto& velocity = registry.getComponent<VelocityComponent>(entity);
    EXPECT_FLOAT_EQ(velocity.vx, -100.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(AISystemTest, UpdateMultipleEntitiesWithDifferentBehaviors) {
    auto entity2 = registry.spawnEntity();

    registry.emplaceComponent<AIComponent>(entity, AIBehavior::MoveLeft, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0F, 0.0F);

    registry.emplaceComponent<AIComponent>(entity2, AIBehavior::Stationary, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity2, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity2, 50.0F, 50.0F);

    aiSystem.update(registry, 0.016F);

    auto& velocity1 = registry.getComponent<VelocityComponent>(entity);
    auto& velocity2 = registry.getComponent<VelocityComponent>(entity2);

    EXPECT_FLOAT_EQ(velocity1.vx, -100.0F);
    EXPECT_FLOAT_EQ(velocity2.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity2.vy, 0.0F);

    registry.killEntity(entity2);
}

TEST_F(AISystemTest, UpdateDoesNotAffectEntitiesWithoutAIComponent) {
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 50.0F, 50.0F);
    // No AI component

    aiSystem.update(registry, 0.016F);

    auto& velocity = registry.getComponent<VelocityComponent>(entity);
    EXPECT_FLOAT_EQ(velocity.vx, 50.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 50.0F);
}

TEST_F(AISystemTest, UpdateWithUnregisteredBehaviorDoesNothing) {
    // Clear registry and don't register any behaviors
    BehaviorRegistry::instance().clear();

    registry.emplaceComponent<AIComponent>(entity, AIBehavior::MoveLeft, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 50.0F, 50.0F);

    // Should not crash and velocity should remain unchanged
    aiSystem.update(registry, 0.016F);

    auto& velocity = registry.getComponent<VelocityComponent>(entity);
    EXPECT_FLOAT_EQ(velocity.vx, 50.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 50.0F);

    // Re-register behaviors for other tests
    registerDefaultBehaviors();
}

TEST_F(AISystemTest, UpdateWithEmptyRegistry) {
    // Kill the entity created in SetUp
    registry.killEntity(entity);

    // Should not crash with empty registry
    EXPECT_NO_THROW(aiSystem.update(registry, 0.016F));

    // Re-create entity for TearDown
    entity = registry.spawnEntity();
}

TEST_F(AISystemTest, UpdateWithEntityMissingVelocity) {
    // Entity with AI and Transform but no Velocity
    registry.emplaceComponent<AIComponent>(entity, AIBehavior::MoveLeft, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    // No VelocityComponent

    // Should not crash or affect anything
    EXPECT_NO_THROW(aiSystem.update(registry, 0.016F));
}

TEST_F(AISystemTest, UpdateWithEntityMissingTransform) {
    // Entity with AI and Velocity but no Transform
    registry.emplaceComponent<AIComponent>(entity, AIBehavior::MoveLeft, 100.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0F, 0.0F);
    // No TransformComponent

    // Should not crash or affect anything
    EXPECT_NO_THROW(aiSystem.update(registry, 0.016F));
}

TEST_F(AISystemTest, UpdateWithZeroDeltaTime) {
    registry.emplaceComponent<AIComponent>(entity, AIBehavior::SineWave, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0F, 0.0F);

    float initialStateTimer = registry.getComponent<AIComponent>(entity).stateTimer;
    aiSystem.update(registry, 0.0F);

    auto& ai = registry.getComponent<AIComponent>(entity);
    EXPECT_FLOAT_EQ(ai.stateTimer, initialStateTimer);
}

TEST_F(AISystemTest, UpdateWithLargeDeltaTime) {
    registry.emplaceComponent<AIComponent>(entity, AIBehavior::SineWave, 100.0F);
    registry.emplaceComponent<TransformComponent>(entity, 500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<VelocityComponent>(entity, 0.0F, 0.0F);

    aiSystem.update(registry, 10.0F);

    auto& ai = registry.getComponent<AIComponent>(entity);
    EXPECT_FLOAT_EQ(ai.stateTimer, 10.0F);
}

// =============================================================================
// CleanupSystem Tests
// =============================================================================

class CleanupSystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        config.leftBoundary = -100.0F;
        config.rightBoundary = 900.0F;
        config.topBoundary = -100.0F;
        config.bottomBoundary = 700.0F;
        entity = registry.spawnEntity();
    }

    void TearDown() override {
        if (registry.isAlive(entity)) {
            registry.killEntity(entity);
        }
    }

    ECS::Registry registry;
    ECS::Entity entity;
    CleanupConfig config;
    std::vector<rtype::engine::GameEvent> emittedEvents;
};

TEST(CleanupSystemNameTest, GetNameReturnsCorrectName) {
    CleanupConfig config;
    CleanupSystem cleanupSystem([](const rtype::engine::GameEvent&) {}, config);
    EXPECT_EQ(cleanupSystem.getName(), "CleanupSystem");
}

TEST_F(CleanupSystemTest, EntityInBoundsNotMarkedForDestruction) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, 400.0F, 300.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityLeftOfBoundaryMarkedForDestruction) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, -150.0F, 300.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityRightOfBoundaryMarkedForDestruction) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, 1000.0F, 300.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityAboveBoundaryMarkedForDestruction) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, 400.0F, -150.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityBelowBoundaryMarkedForDestruction) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, 400.0F, 750.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityAtExactBoundaryNotMarkedForDestruction) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, -100.0F, 300.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityWithoutEnemyTagNotProcessed) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, -150.0F, 300.0F, 0.0F);
    // No EnemyTag

    cleanupSystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityAlreadyMarkedNotDoubleMarked) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, -150.0F, 300.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);
    registry.emplaceComponent<DestroyTag>(entity);

    // Should not throw or cause issues
    EXPECT_NO_THROW(cleanupSystem.update(registry, 0.016F));
}

TEST_F(CleanupSystemTest, CustomCleanupConfig) {
    CleanupConfig customConfig;
    customConfig.leftBoundary = 0.0F;
    customConfig.rightBoundary = 100.0F;
    customConfig.topBoundary = 0.0F;
    customConfig.bottomBoundary = 100.0F;

    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        customConfig);

    registry.emplaceComponent<TransformComponent>(entity, 50.0F, 50.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, MultipleEntitiesOutOfBounds) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    auto entity2 = registry.spawnEntity();
    auto entity3 = registry.spawnEntity();

    // Entity 1: out of bounds (left)
    registry.emplaceComponent<TransformComponent>(entity, -150.0F, 300.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    // Entity 2: in bounds
    registry.emplaceComponent<TransformComponent>(entity2, 400.0F, 300.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity2);

    // Entity 3: out of bounds (bottom)
    registry.emplaceComponent<TransformComponent>(entity3, 400.0F, 800.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity3);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity2));
    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity3));

    registry.killEntity(entity2);
    registry.killEntity(entity3);
}

TEST_F(CleanupSystemTest, EntityAtCornerBoundaries) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Entity at top-left corner boundary
    registry.emplaceComponent<TransformComponent>(entity, -100.0F, -100.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    // At exactly the boundary, should NOT be destroyed
    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityAtRightBottomCorner) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Entity at bottom-right corner boundary
    registry.emplaceComponent<TransformComponent>(entity, 900.0F, 700.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityJustOutsideRightBoundary) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, 900.1F, 300.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, EntityJustOutsideTopBoundary) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, 400.0F, -100.1F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);

    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, UpdateWithEmptyRegistry) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Kill entity from SetUp
    registry.killEntity(entity);

    // Should not crash with empty registry
    EXPECT_NO_THROW(cleanupSystem.update(registry, 0.016F));

    // Re-create for TearDown
    entity = registry.spawnEntity();
}

TEST_F(CleanupSystemTest, EntityWithTransformButNoEnemyTag) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Out of bounds but no EnemyTag
    registry.emplaceComponent<TransformComponent>(entity, -500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<PlayerTag>(entity);  // Player, not enemy

    cleanupSystem.update(registry, 0.016F);

    // Should NOT be marked for destruction (only enemies are cleaned up)
    EXPECT_FALSE(registry.hasComponent<DestroyTag>(entity));
}

TEST_F(CleanupSystemTest, MultipleUpdatesDoNotDuplicateTag) {
    CleanupSystem cleanupSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    registry.emplaceComponent<TransformComponent>(entity, -500.0F, 300.0F, 0.0F);
    registry.emplaceComponent<EnemyTag>(entity);

    cleanupSystem.update(registry, 0.016F);
    EXPECT_TRUE(registry.hasComponent<DestroyTag>(entity));

    // Second update should not throw or cause issues
    EXPECT_NO_THROW(cleanupSystem.update(registry, 0.016F));
}

// =============================================================================
// DestroySystem Tests
// =============================================================================

class DestroySystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        entity = registry.spawnEntity();
        enemyCountDecremented = false;
    }

    void TearDown() override {
        // Entity might be destroyed by the system
    }

    ECS::Registry registry;
    ECS::Entity entity;
    std::vector<rtype::engine::GameEvent> emittedEvents;
    bool enemyCountDecremented;
};

TEST(DestroySystemNameTest, GetNameReturnsCorrectName) {
    DestroySystem destroySystem([](const rtype::engine::GameEvent&) {}, []() {});
    EXPECT_EQ(destroySystem.getName(), "DestroySystem");
}

TEST_F(DestroySystemTest, EntityWithDestroyTagIsDestroyed) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    registry.emplaceComponent<NetworkIdComponent>(entity, 1u);

    destroySystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.isAlive(entity));
}

TEST_F(DestroySystemTest, EntityWithoutDestroyTagNotDestroyed) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<NetworkIdComponent>(entity, 1u);
    // No DestroyTag

    destroySystem.update(registry, 0.016F);

    EXPECT_TRUE(registry.isAlive(entity));

    registry.killEntity(entity);
}

TEST_F(DestroySystemTest, DestroyEmitsEventForNetworkedEntity) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    registry.emplaceComponent<NetworkIdComponent>(entity, 42u);

    destroySystem.update(registry, 0.016F);

    ASSERT_EQ(emittedEvents.size(), 1u);
    EXPECT_EQ(emittedEvents[0].type, rtype::engine::GameEventType::EntityDestroyed);
    EXPECT_EQ(emittedEvents[0].entityNetworkId, 42u);
}

TEST_F(DestroySystemTest, DestroyDecrementsEnemyCountForEnemies) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    registry.emplaceComponent<EnemyTag>(entity);
    registry.emplaceComponent<NetworkIdComponent>(entity, 1u);

    destroySystem.update(registry, 0.016F);

    EXPECT_TRUE(enemyCountDecremented);
}

TEST_F(DestroySystemTest, DestroyDoesNotDecrementForNonEnemies) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    registry.emplaceComponent<NetworkIdComponent>(entity, 1u);
    // No EnemyTag

    destroySystem.update(registry, 0.016F);

    EXPECT_FALSE(enemyCountDecremented);
}

TEST_F(DestroySystemTest, DestroyMultipleEntities) {
    int decrementCount = 0;
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [&decrementCount]() { decrementCount++; });

    auto entity2 = registry.spawnEntity();
    auto entity3 = registry.spawnEntity();

    registry.emplaceComponent<DestroyTag>(entity);
    registry.emplaceComponent<EnemyTag>(entity);
    registry.emplaceComponent<NetworkIdComponent>(entity, 1u);

    registry.emplaceComponent<DestroyTag>(entity2);
    registry.emplaceComponent<EnemyTag>(entity2);
    registry.emplaceComponent<NetworkIdComponent>(entity2, 2u);

    registry.emplaceComponent<DestroyTag>(entity3);
    registry.emplaceComponent<NetworkIdComponent>(entity3, 3u);  // Not an enemy

    destroySystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.isAlive(entity));
    EXPECT_FALSE(registry.isAlive(entity2));
    EXPECT_FALSE(registry.isAlive(entity3));

    EXPECT_EQ(emittedEvents.size(), 3u);
    EXPECT_EQ(decrementCount, 2);  // Only 2 enemies
}

TEST_F(DestroySystemTest, DestroyEntityWithInvalidNetworkIdNoEvent) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    // NetworkIdComponent with invalid ID
    registry.emplaceComponent<NetworkIdComponent>(entity);  // Default is invalid

    destroySystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.isAlive(entity));
    EXPECT_TRUE(emittedEvents.empty());  // No event for invalid network ID
}

TEST_F(DestroySystemTest, DestroyEntityWithoutNetworkIdNoEvent) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    // No NetworkIdComponent

    destroySystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.isAlive(entity));
    EXPECT_TRUE(emittedEvents.empty());
}

TEST_F(DestroySystemTest, DestroyedEnemyEventHasCorrectEntityType) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    registry.emplaceComponent<EnemyTag>(entity);
    registry.emplaceComponent<NetworkIdComponent>(entity, 1u);

    destroySystem.update(registry, 0.016F);

    ASSERT_EQ(emittedEvents.size(), 1u);
    EXPECT_EQ(emittedEvents[0].entityType, 1u);  // network::EntityType::Bydos
}

TEST_F(DestroySystemTest, DestroyedNonEnemyEventHasPlayerEntityType) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    registry.emplaceComponent<NetworkIdComponent>(entity, 1u);
    // No EnemyTag - defaults to Player type in current implementation

    destroySystem.update(registry, 0.016F);

    ASSERT_EQ(emittedEvents.size(), 1u);
    EXPECT_EQ(emittedEvents[0].entityType, 0u);  // network::EntityType::Player
}

TEST_F(DestroySystemTest, UpdateWithEmptyRegistry) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    // Kill entity from SetUp
    registry.killEntity(entity);

    // Should not crash with empty registry
    EXPECT_NO_THROW(destroySystem.update(registry, 0.016F));

    EXPECT_TRUE(emittedEvents.empty());
    EXPECT_FALSE(enemyCountDecremented);
}

TEST_F(DestroySystemTest, UpdateWithNoDestroyTaggedEntities) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    // Entity exists but has no DestroyTag
    registry.emplaceComponent<NetworkIdComponent>(entity, 1u);
    registry.emplaceComponent<EnemyTag>(entity);

    destroySystem.update(registry, 0.016F);

    EXPECT_TRUE(registry.isAlive(entity));
    EXPECT_TRUE(emittedEvents.empty());
    EXPECT_FALSE(enemyCountDecremented);

    registry.killEntity(entity);
}

TEST_F(DestroySystemTest, DestroyEnemyWithValidNetworkId) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    registry.emplaceComponent<EnemyTag>(entity);
    registry.emplaceComponent<NetworkIdComponent>(entity, 999u);

    destroySystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.isAlive(entity));
    EXPECT_TRUE(enemyCountDecremented);
    ASSERT_EQ(emittedEvents.size(), 1u);
    EXPECT_EQ(emittedEvents[0].entityNetworkId, 999u);
    EXPECT_EQ(emittedEvents[0].entityType, 1u);  // network::EntityType::Bydos
}

TEST_F(DestroySystemTest, DestroyPlayerWithValidNetworkId) {
    DestroySystem destroySystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        [this]() { enemyCountDecremented = true; });

    registry.emplaceComponent<DestroyTag>(entity);
    registry.emplaceComponent<PlayerTag>(entity);  // Player, not enemy
    registry.emplaceComponent<NetworkIdComponent>(entity, 123u);

    destroySystem.update(registry, 0.016F);

    EXPECT_FALSE(registry.isAlive(entity));
    EXPECT_FALSE(enemyCountDecremented);  // Not an enemy
    ASSERT_EQ(emittedEvents.size(), 1u);
    EXPECT_EQ(emittedEvents[0].entityNetworkId, 123u);
    EXPECT_EQ(emittedEvents[0].entityType, 0u);  // network::EntityType::Player
}

// =============================================================================
// SpawnerSystem Tests
// =============================================================================

class SpawnerSystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        config.minSpawnInterval = 0.5F;
        config.maxSpawnInterval = 1.0F;
        config.maxEnemies = 10;
        config.spawnX = 800.0F;
        config.minSpawnY = 50.0F;
        config.maxSpawnY = 550.0F;
        config.bydosSlaveSpeed = 100.0F;
    }

    ECS::Registry registry;
    SpawnerConfig config;
    std::vector<rtype::engine::GameEvent> emittedEvents;
};

TEST(SpawnerSystemNameTest, GetNameReturnsCorrectName) {
    SpawnerConfig config;
    SpawnerSystem spawnerSystem([](const rtype::engine::GameEvent&) {}, config);
    EXPECT_EQ(spawnerSystem.getName(), "SpawnerSystem");
}

TEST_F(SpawnerSystemTest, InitialEnemyCountIsZero) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    EXPECT_EQ(spawnerSystem.getEnemyCount(), 0u);
}

TEST_F(SpawnerSystemTest, SpawnsEnemyAfterInterval) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Run updates until spawn occurs
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() > 0) break;
    }

    EXPECT_GT(spawnerSystem.getEnemyCount(), 0u);
}

TEST_F(SpawnerSystemTest, SpawnedEntityHasRequiredComponents) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Force a spawn
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() > 0) break;
    }

    // Find the spawned entity
    auto view = registry.view<TransformComponent, VelocityComponent, AIComponent,
                               HealthComponent, BoundingBoxComponent, NetworkIdComponent,
                               EnemyTag, BydosSlaveTag>();

    int entityCount = 0;
    view.each([&entityCount](ECS::Entity /*entity*/,
                              const TransformComponent& /*transform*/,
                              const VelocityComponent& /*velocity*/,
                              const AIComponent& /*ai*/,
                              const HealthComponent& /*health*/,
                              const BoundingBoxComponent& /*bbox*/,
                              const NetworkIdComponent& /*netId*/,
                              const EnemyTag& /*enemyTag*/,
                              const BydosSlaveTag& /*bydosTag*/) {
        entityCount++;
    });

    EXPECT_GT(entityCount, 0);
}

TEST_F(SpawnerSystemTest, SpawnedEntityHasCorrectPosition) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Force a spawn
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() > 0) break;
    }

    auto view = registry.view<TransformComponent, EnemyTag>();
    view.each([this](ECS::Entity /*entity*/,
                     const TransformComponent& transform,
                     const EnemyTag& /*tag*/) {
        EXPECT_FLOAT_EQ(transform.x, config.spawnX);
        EXPECT_GE(transform.y, config.minSpawnY);
        EXPECT_LE(transform.y, config.maxSpawnY);
    });
}

TEST_F(SpawnerSystemTest, SpawnedEntityHasCorrectVelocity) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Force a spawn
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() > 0) break;
    }

    auto view = registry.view<VelocityComponent, EnemyTag>();
    view.each([this](ECS::Entity /*entity*/,
                     const VelocityComponent& velocity,
                     const EnemyTag& /*tag*/) {
        EXPECT_FLOAT_EQ(velocity.vx, -config.bydosSlaveSpeed);
        EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
    });
}

TEST_F(SpawnerSystemTest, SpawnEmitsEvent) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Force a spawn
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (!emittedEvents.empty()) break;
    }

    ASSERT_FALSE(emittedEvents.empty());
    EXPECT_EQ(emittedEvents[0].type, rtype::engine::GameEventType::EntitySpawned);
    EXPECT_EQ(emittedEvents[0].entityType, 1u);  // network::EntityType::Bydos
}

TEST_F(SpawnerSystemTest, RespectsMaxEnemies) {
    config.maxEnemies = 3;
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Run many updates to try spawning more than max
    for (int i = 0; i < 500; ++i) {
        spawnerSystem.update(registry, 0.1F);
    }

    EXPECT_LE(spawnerSystem.getEnemyCount(), config.maxEnemies);
}

TEST_F(SpawnerSystemTest, SpawnIntervalVariation) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    std::vector<float> spawnTimes;
    float totalTime = 0.0F;
    size_t lastCount = 0;

    for (int i = 0; i < 200; ++i) {
        spawnerSystem.update(registry, 0.1F);
        totalTime += 0.1F;

        if (spawnerSystem.getEnemyCount() > lastCount) {
            spawnTimes.push_back(totalTime);
            lastCount = spawnerSystem.getEnemyCount();
            totalTime = 0.0F;
        }

        if (spawnTimes.size() >= 5) break;
    }

    // Verify spawn times are within configured range
    for (size_t i = 1; i < spawnTimes.size(); ++i) {
        EXPECT_GE(spawnTimes[i], config.minSpawnInterval - 0.2F);  // Allow small margin
        EXPECT_LE(spawnTimes[i], config.maxSpawnInterval + 0.2F);
    }
}

TEST_F(SpawnerSystemTest, NoSpawnBeforeInterval) {
    config.minSpawnInterval = 10.0F;
    config.maxSpawnInterval = 20.0F;

    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Update for less than minimum interval
    for (int i = 0; i < 50; ++i) {
        spawnerSystem.update(registry, 0.1F);  // 5 seconds total
    }

    // Should not have spawned yet
    EXPECT_EQ(spawnerSystem.getEnemyCount(), 0u);
}

TEST_F(SpawnerSystemTest, SpawnedEntityHasCorrectAIComponent) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Force a spawn
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() > 0) break;
    }

    auto view = registry.view<AIComponent, EnemyTag>();
    view.each([this](ECS::Entity /*entity*/,
                     const AIComponent& ai,
                     const EnemyTag& /*tag*/) {
        const std::array<AIBehavior, 6> allowed = {
            AIBehavior::MoveLeft, AIBehavior::SineWave, AIBehavior::ZigZag,
            AIBehavior::DiveBomb, AIBehavior::Stationary, AIBehavior::Chase};
        const bool found = std::find(allowed.begin(), allowed.end(),
                                      ai.behavior) != allowed.end();
        EXPECT_TRUE(found);
        EXPECT_FLOAT_EQ(ai.speed, config.bydosSlaveSpeed);
    });
}

TEST_F(SpawnerSystemTest, SpawnedEntityHasCorrectHealth) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Force a spawn
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() > 0) break;
    }

    auto view = registry.view<HealthComponent, EnemyTag>();
    view.each([](ECS::Entity /*entity*/,
                 const HealthComponent& health,
                 const EnemyTag& /*tag*/) {
        EXPECT_EQ(health.current, 10);  // BYDOS_SLAVE_HEALTH
        EXPECT_EQ(health.max, 10);
    });
}

TEST_F(SpawnerSystemTest, SpawnedEntityHasCorrectBoundingBox) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Force a spawn
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() > 0) break;
    }

    auto view = registry.view<BoundingBoxComponent, EnemyTag>();
    view.each([](ECS::Entity /*entity*/,
                 const BoundingBoxComponent& bbox,
                 const EnemyTag& /*tag*/) {
        EXPECT_FLOAT_EQ(bbox.width, 32.0F);  // BYDOS_SLAVE_SIZE
        EXPECT_FLOAT_EQ(bbox.height, 32.0F);
    });
}

TEST_F(SpawnerSystemTest, NetworkIdIncrementsForEachSpawn) {
    config.maxEnemies = 5;
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Spawn multiple enemies
    for (int i = 0; i < 300; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() >= 3) break;
    }

    // Check that network IDs are unique
    std::vector<uint32_t> networkIds;
    auto view = registry.view<NetworkIdComponent, EnemyTag>();
    view.each([&networkIds](ECS::Entity /*entity*/,
                            const NetworkIdComponent& netId,
                            const EnemyTag& /*tag*/) {
        networkIds.push_back(netId.networkId);
    });

    // All network IDs should be unique
    std::sort(networkIds.begin(), networkIds.end());
    auto last = std::unique(networkIds.begin(), networkIds.end());
    EXPECT_EQ(last, networkIds.end());
}

TEST_F(SpawnerSystemTest, SpawnEventHasCorrectCoordinates) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Force a spawn
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (!emittedEvents.empty()) break;
    }

    ASSERT_FALSE(emittedEvents.empty());
    float minX = config.spawnX - config.stationarySpawnInset;
    EXPECT_LE(emittedEvents[0].x, config.spawnX);
    EXPECT_GE(emittedEvents[0].x, minX);
    EXPECT_GE(emittedEvents[0].y, config.minSpawnY);
    EXPECT_LE(emittedEvents[0].y, config.maxSpawnY);
    EXPECT_FLOAT_EQ(emittedEvents[0].rotation, 0.0F);
}

TEST_F(SpawnerSystemTest, UpdateWithZeroDeltaTime) {
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Multiple updates with zero delta time should not spawn
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.0F);
    }

    EXPECT_EQ(spawnerSystem.getEnemyCount(), 0u);
}

TEST_F(SpawnerSystemTest, MaxEnemiesZeroNeverSpawns) {
    config.maxEnemies = 0;
    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Run many updates
    for (int i = 0; i < 200; ++i) {
        spawnerSystem.update(registry, 0.1F);
    }

    EXPECT_EQ(spawnerSystem.getEnemyCount(), 0u);
}

TEST_F(SpawnerSystemTest, StopsSpawningWhenMaxReached) {
    config.maxEnemies = 2;
    config.minSpawnInterval = 0.1F;
    config.maxSpawnInterval = 0.2F;

    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Run until max is reached
    for (int i = 0; i < 100; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() >= config.maxEnemies) break;
    }

    size_t countAtMax = spawnerSystem.getEnemyCount();
    EXPECT_EQ(countAtMax, config.maxEnemies);

    // Run more updates - count should not increase
    for (int i = 0; i < 50; ++i) {
        spawnerSystem.update(registry, 0.1F);
    }

    EXPECT_EQ(spawnerSystem.getEnemyCount(), config.maxEnemies);
}

TEST_F(SpawnerSystemTest, SpawnTimerAccumulatesCorrectly) {
    config.minSpawnInterval = 1.0F;
    config.maxSpawnInterval = 1.0F;  // Fixed interval for predictability

    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Update with small increments
    for (int i = 0; i < 5; ++i) {
        spawnerSystem.update(registry, 0.1F);  // 0.5 seconds total
    }

    // Should not have spawned yet (< 1 second)
    EXPECT_EQ(spawnerSystem.getEnemyCount(), 0u);

    // Update to reach 1 second
    for (int i = 0; i < 6; ++i) {
        spawnerSystem.update(registry, 0.1F);  // 0.6 more seconds = 1.1 total
    }

    // Should have spawned now
    EXPECT_GE(spawnerSystem.getEnemyCount(), 1u);
}

TEST_F(SpawnerSystemTest, SpawnYPositionWithinRange) {
    config.minSpawnY = 100.0F;
    config.maxSpawnY = 200.0F;

    SpawnerSystem spawnerSystem(
        [this](const rtype::engine::GameEvent& event) {
            emittedEvents.push_back(event);
        },
        config);

    // Spawn multiple enemies and check Y positions
    for (int i = 0; i < 300; ++i) {
        spawnerSystem.update(registry, 0.1F);
        if (spawnerSystem.getEnemyCount() >= 5) break;
    }

    auto view = registry.view<TransformComponent, EnemyTag>();
    view.each([this](ECS::Entity /*entity*/,
                     const TransformComponent& transform,
                     const EnemyTag& /*tag*/) {
        EXPECT_GE(transform.y, config.minSpawnY);
        EXPECT_LE(transform.y, config.maxSpawnY);
    });
}

// =============================================================================
// SpawnerConfig Tests
// =============================================================================

TEST(SpawnerConfigTest, DefaultValues) {
    SpawnerConfig config;

    EXPECT_FLOAT_EQ(config.minSpawnInterval, 1.0F);
    EXPECT_FLOAT_EQ(config.maxSpawnInterval, 3.0F);
    EXPECT_EQ(config.maxEnemies, 50u);
    EXPECT_FLOAT_EQ(config.spawnX, 800.0F);
    EXPECT_FLOAT_EQ(config.minSpawnY, 50.0F);
    EXPECT_FLOAT_EQ(config.maxSpawnY, 550.0F);
    EXPECT_FLOAT_EQ(config.bydosSlaveSpeed, 100.0F);
}

// =============================================================================
// CleanupConfig Tests
// =============================================================================

TEST(CleanupConfigTest, DefaultValues) {
    CleanupConfig config;

    EXPECT_FLOAT_EQ(config.leftBoundary, -100.0F);
    EXPECT_FLOAT_EQ(config.rightBoundary, 900.0F);
    EXPECT_FLOAT_EQ(config.topBoundary, -100.0F);
    EXPECT_FLOAT_EQ(config.bottomBoundary, 700.0F);
}
