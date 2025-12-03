/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_behavior_registry - Unit tests for BehaviorRegistry
*/

#include <gtest/gtest.h>

#include "../../../src/games/rtype/shared/Systems/AISystem/Behaviors/BehaviorRegistry.hpp"
#include "../../../src/games/rtype/shared/Systems/AISystem/Behaviors/Behaviors.hpp"

using namespace rtype::games::rtype::shared;

// =============================================================================
// BehaviorRegistry Tests
// =============================================================================

class BehaviorRegistryTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Clear registry before each test
        BehaviorRegistry::instance().clear();
    }

    void TearDown() override {
        // Clean up after each test
        BehaviorRegistry::instance().clear();
    }
};

TEST_F(BehaviorRegistryTest, InstanceReturnsSameInstance) {
    auto& instance1 = BehaviorRegistry::instance();
    auto& instance2 = BehaviorRegistry::instance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(BehaviorRegistryTest, InitiallyEmpty) {
    EXPECT_EQ(BehaviorRegistry::instance().size(), 0u);
}

TEST_F(BehaviorRegistryTest, RegisterMoveLeftBehavior) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<MoveLeftBehavior>();

    EXPECT_EQ(registry.size(), 1u);
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::MoveLeft));
}

TEST_F(BehaviorRegistryTest, RegisterSineWaveBehavior) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<SineWaveBehavior>();

    EXPECT_EQ(registry.size(), 1u);
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::SineWave));
}

TEST_F(BehaviorRegistryTest, RegisterChaseBehavior) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<ChaseBehavior>();

    EXPECT_EQ(registry.size(), 1u);
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::Chase));
}

TEST_F(BehaviorRegistryTest, RegisterPatrolBehavior) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<PatrolBehavior>();

    EXPECT_EQ(registry.size(), 1u);
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::Patrol));
}

TEST_F(BehaviorRegistryTest, RegisterStationaryBehavior) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<StationaryBehavior>();

    EXPECT_EQ(registry.size(), 1u);
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::Stationary));
}

TEST_F(BehaviorRegistryTest, RegisterMultipleBehaviors) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<MoveLeftBehavior>();
    registry.registerBehavior<SineWaveBehavior>();
    registry.registerBehavior<ChaseBehavior>();

    EXPECT_EQ(registry.size(), 3u);
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::MoveLeft));
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::SineWave));
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::Chase));
}

TEST_F(BehaviorRegistryTest, GetBehaviorReturnsCorrectType) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<MoveLeftBehavior>();

    auto behavior = registry.getBehavior(AIBehavior::MoveLeft);
    ASSERT_NE(behavior, nullptr);
    EXPECT_EQ(behavior->getType(), AIBehavior::MoveLeft);
    EXPECT_EQ(behavior->getName(), "MoveLeftBehavior");
}

TEST_F(BehaviorRegistryTest, GetBehaviorReturnsNullptrForUnregistered) {
    auto& registry = BehaviorRegistry::instance();

    auto behavior = registry.getBehavior(AIBehavior::MoveLeft);
    EXPECT_EQ(behavior, nullptr);
}

TEST_F(BehaviorRegistryTest, HasBehaviorReturnsFalseForUnregistered) {
    auto& registry = BehaviorRegistry::instance();
    EXPECT_FALSE(registry.hasBehavior(AIBehavior::MoveLeft));
}

TEST_F(BehaviorRegistryTest, ClearRemovesAllBehaviors) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<MoveLeftBehavior>();
    registry.registerBehavior<SineWaveBehavior>();
    registry.registerBehavior<ChaseBehavior>();

    EXPECT_EQ(registry.size(), 3u);

    registry.clear();

    EXPECT_EQ(registry.size(), 0u);
    EXPECT_FALSE(registry.hasBehavior(AIBehavior::MoveLeft));
    EXPECT_FALSE(registry.hasBehavior(AIBehavior::SineWave));
    EXPECT_FALSE(registry.hasBehavior(AIBehavior::Chase));
}

TEST_F(BehaviorRegistryTest, RegisterBehaviorWithCustomParameters) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<SineWaveBehavior>(100.0F, 3.0F);

    EXPECT_TRUE(registry.hasBehavior(AIBehavior::SineWave));
    auto behavior = registry.getBehavior(AIBehavior::SineWave);
    ASSERT_NE(behavior, nullptr);
}

TEST_F(BehaviorRegistryTest, RegisterChaseBehaviorWithStopDistance) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<ChaseBehavior>(10.0F);

    EXPECT_TRUE(registry.hasBehavior(AIBehavior::Chase));
    auto behavior = registry.getBehavior(AIBehavior::Chase);
    ASSERT_NE(behavior, nullptr);
    EXPECT_EQ(behavior->getType(), AIBehavior::Chase);
}

TEST_F(BehaviorRegistryTest, ReRegisterBehaviorOverwrites) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<MoveLeftBehavior>();

    auto behavior1 = registry.getBehavior(AIBehavior::MoveLeft);
    ASSERT_NE(behavior1, nullptr);

    registry.registerBehavior<MoveLeftBehavior>();

    auto behavior2 = registry.getBehavior(AIBehavior::MoveLeft);
    ASSERT_NE(behavior2, nullptr);

    // Size should still be 1
    EXPECT_EQ(registry.size(), 1u);
}

TEST_F(BehaviorRegistryTest, RegisterDefaultBehaviorsFunction) {
    registerDefaultBehaviors();

    auto& registry = BehaviorRegistry::instance();

    EXPECT_TRUE(registry.hasBehavior(AIBehavior::MoveLeft));
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::SineWave));
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::Chase));
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::Patrol));
    EXPECT_TRUE(registry.hasBehavior(AIBehavior::Stationary));

    EXPECT_EQ(registry.size(), 5u);
}

TEST_F(BehaviorRegistryTest, BehaviorCanBeApplied) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<MoveLeftBehavior>();

    auto behavior = registry.getBehavior(AIBehavior::MoveLeft);
    ASSERT_NE(behavior, nullptr);

    AIComponent ai;
    ai.speed = 100.0F;
    TransformComponent transform;
    VelocityComponent velocity;

    behavior->apply(ai, transform, velocity, 0.016F);

    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(BehaviorRegistryTest, AllRegisteredBehaviorsCanBeApplied) {
    registerDefaultBehaviors();

    auto& registry = BehaviorRegistry::instance();

    AIComponent ai;
    ai.speed = 100.0F;
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    TransformComponent transform;
    transform.x = 100.0F;
    transform.y = 100.0F;
    VelocityComponent velocity;

    // Test all behaviors can be applied without crashing
    std::vector<AIBehavior> behaviors = {AIBehavior::MoveLeft, AIBehavior::SineWave,
                                          AIBehavior::Chase, AIBehavior::Patrol,
                                          AIBehavior::Stationary};

    for (auto behaviorType : behaviors) {
        auto behavior = registry.getBehavior(behaviorType);
        ASSERT_NE(behavior, nullptr) << "Behavior not found for type: "
                                      << static_cast<int>(behaviorType);

        // Reset velocity for each test
        velocity.vx = 0.0F;
        velocity.vy = 0.0F;

        // Should not throw
        EXPECT_NO_THROW(behavior->apply(ai, transform, velocity, 0.016F));
    }
}

// =============================================================================
// BehaviorRegistry Size Tests
// =============================================================================

TEST_F(BehaviorRegistryTest, SizeAfterRegistration) {
    auto& registry = BehaviorRegistry::instance();

    EXPECT_EQ(registry.size(), 0u);

    registry.registerBehavior<MoveLeftBehavior>();
    EXPECT_EQ(registry.size(), 1u);

    registry.registerBehavior<SineWaveBehavior>();
    EXPECT_EQ(registry.size(), 2u);

    registry.registerBehavior<ChaseBehavior>();
    EXPECT_EQ(registry.size(), 3u);

    registry.registerBehavior<PatrolBehavior>();
    EXPECT_EQ(registry.size(), 4u);

    registry.registerBehavior<StationaryBehavior>();
    EXPECT_EQ(registry.size(), 5u);
}

TEST_F(BehaviorRegistryTest, SizeAfterClear) {
    auto& registry = BehaviorRegistry::instance();

    registry.registerBehavior<MoveLeftBehavior>();
    registry.registerBehavior<SineWaveBehavior>();
    EXPECT_EQ(registry.size(), 2u);

    registry.clear();
    EXPECT_EQ(registry.size(), 0u);
}

TEST_F(BehaviorRegistryTest, SizeIsNoexcept) {
    auto& registry = BehaviorRegistry::instance();
    EXPECT_TRUE(noexcept(registry.size()));
}

TEST_F(BehaviorRegistryTest, GetBehaviorAfterClear) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<MoveLeftBehavior>();

    EXPECT_NE(registry.getBehavior(AIBehavior::MoveLeft), nullptr);

    registry.clear();

    EXPECT_EQ(registry.getBehavior(AIBehavior::MoveLeft), nullptr);
}

TEST_F(BehaviorRegistryTest, HasBehaviorAfterClear) {
    auto& registry = BehaviorRegistry::instance();
    registry.registerBehavior<MoveLeftBehavior>();

    EXPECT_TRUE(registry.hasBehavior(AIBehavior::MoveLeft));

    registry.clear();

    EXPECT_FALSE(registry.hasBehavior(AIBehavior::MoveLeft));
}

TEST_F(BehaviorRegistryTest, RegisterAllBehaviorsIndividually) {
    auto& registry = BehaviorRegistry::instance();

    registry.registerBehavior<MoveLeftBehavior>();
    EXPECT_EQ(registry.size(), 1u);

    registry.registerBehavior<SineWaveBehavior>(50.0F, 2.0F);
    EXPECT_EQ(registry.size(), 2u);

    registry.registerBehavior<ChaseBehavior>(5.0F);
    EXPECT_EQ(registry.size(), 3u);

    registry.registerBehavior<PatrolBehavior>();
    EXPECT_EQ(registry.size(), 4u);

    registry.registerBehavior<StationaryBehavior>();
    EXPECT_EQ(registry.size(), 5u);
}

TEST_F(BehaviorRegistryTest, GetBehaviorForAllTypes) {
    registerDefaultBehaviors();

    auto& registry = BehaviorRegistry::instance();

    auto moveLeft = registry.getBehavior(AIBehavior::MoveLeft);
    ASSERT_NE(moveLeft, nullptr);
    EXPECT_EQ(moveLeft->getName(), "MoveLeftBehavior");

    auto sineWave = registry.getBehavior(AIBehavior::SineWave);
    ASSERT_NE(sineWave, nullptr);
    EXPECT_EQ(sineWave->getName(), "SineWaveBehavior");

    auto chase = registry.getBehavior(AIBehavior::Chase);
    ASSERT_NE(chase, nullptr);
    EXPECT_EQ(chase->getName(), "ChaseBehavior");

    auto patrol = registry.getBehavior(AIBehavior::Patrol);
    ASSERT_NE(patrol, nullptr);
    EXPECT_EQ(patrol->getName(), "PatrolBehavior");

    auto stationary = registry.getBehavior(AIBehavior::Stationary);
    ASSERT_NE(stationary, nullptr);
    EXPECT_EQ(stationary->getName(), "StationaryBehavior");
}

TEST_F(BehaviorRegistryTest, ApplyAllBehaviorsSequentially) {
    registerDefaultBehaviors();

    auto& registry = BehaviorRegistry::instance();

    AIComponent ai;
    ai.speed = 100.0F;
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    TransformComponent transform;
    transform.x = 100.0F;
    transform.y = 100.0F;
    VelocityComponent velocity;

    // Apply MoveLeft
    auto moveLeft = registry.getBehavior(AIBehavior::MoveLeft);
    moveLeft->apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, -100.0F);

    // Apply Stationary
    auto stationary = registry.getBehavior(AIBehavior::Stationary);
    stationary->apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);

    // Apply Chase
    auto chase = registry.getBehavior(AIBehavior::Chase);
    chase->apply(ai, transform, velocity, 0.016F);
    EXPECT_LT(velocity.vx, 0.0F);
}

TEST_F(BehaviorRegistryTest, RegisterDefaultBehaviorsTwice) {
    registerDefaultBehaviors();
    EXPECT_EQ(BehaviorRegistry::instance().size(), 5u);

    // Registering again should not add duplicates
    registerDefaultBehaviors();
    EXPECT_EQ(BehaviorRegistry::instance().size(), 5u);
}
