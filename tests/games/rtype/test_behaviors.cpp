/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_behaviors - Unit tests for AI behaviors
*/

#include <gtest/gtest.h>

#include <cmath>

#include "../../../src/games/rtype/shared/Systems/AISystem/Behaviors/Behaviors.hpp"

using namespace rtype::games::rtype::shared;

// =============================================================================
// MoveLeftBehavior Tests
// =============================================================================

class MoveLeftBehaviorTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ai.behavior = AIBehavior::MoveLeft;
        ai.speed = 100.0F;
        transform.x = 500.0F;
        transform.y = 300.0F;
        velocity.vx = 0.0F;
        velocity.vy = 0.0F;
    }

    MoveLeftBehavior behavior;
    AIComponent ai;
    TransformComponent transform;
    VelocityComponent velocity;
};

TEST_F(MoveLeftBehaviorTest, GetTypeReturnsMoveLeft) {
    EXPECT_EQ(behavior.getType(), AIBehavior::MoveLeft);
}

TEST_F(MoveLeftBehaviorTest, GetNameReturnsCorrectName) {
    EXPECT_EQ(behavior.getName(), "MoveLeftBehavior");
}

TEST_F(MoveLeftBehaviorTest, ApplySetsNegativeXVelocity) {
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
}

TEST_F(MoveLeftBehaviorTest, ApplySetsZeroYVelocity) {
    velocity.vy = 50.0F;  // Start with some Y velocity
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(MoveLeftBehaviorTest, ApplyWithDifferentSpeed) {
    ai.speed = 200.0F;
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, -200.0F);
}

TEST_F(MoveLeftBehaviorTest, ApplyWithZeroSpeed) {
    ai.speed = 0.0F;
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
}

TEST_F(MoveLeftBehaviorTest, ApplyIgnoresDeltaTime) {
    behavior.apply(ai, transform, velocity, 1.0F);
    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);

    velocity.vx = 0.0F;
    behavior.apply(ai, transform, velocity, 0.001F);
    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
}

// =============================================================================
// SineWaveBehavior Tests
// =============================================================================

class SineWaveBehaviorTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ai.behavior = AIBehavior::SineWave;
        ai.speed = 100.0F;
        ai.stateTimer = 0.0F;
        transform.x = 500.0F;
        transform.y = 300.0F;
        velocity.vx = 0.0F;
        velocity.vy = 0.0F;
    }

    AIComponent ai;
    TransformComponent transform;
    VelocityComponent velocity;
};

TEST(SineWaveBehaviorConstructorTest, DefaultParameters) {
    SineWaveBehavior behavior;
    EXPECT_EQ(behavior.getType(), AIBehavior::SineWave);
    EXPECT_EQ(behavior.getName(), "SineWaveBehavior");
}

TEST(SineWaveBehaviorConstructorTest, CustomParameters) {
    SineWaveBehavior behavior(100.0F, 3.0F);
    EXPECT_EQ(behavior.getType(), AIBehavior::SineWave);
}

TEST_F(SineWaveBehaviorTest, ApplySetsNegativeXVelocity) {
    SineWaveBehavior behavior;
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
}

TEST_F(SineWaveBehaviorTest, ApplyUpdatesStateTimer) {
    SineWaveBehavior behavior;
    float initialTimer = ai.stateTimer;
    behavior.apply(ai, transform, velocity, 0.5F);
    EXPECT_FLOAT_EQ(ai.stateTimer, initialTimer + 0.5F);
}

TEST_F(SineWaveBehaviorTest, ApplyWithDifferentAmplitude) {
    SineWaveBehavior behavior(100.0F, 2.0F);
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
    // Y velocity should be based on cosine
    EXPECT_NE(velocity.vy, 0.0F);
}

TEST_F(SineWaveBehaviorTest, YVelocityOscillates) {
    SineWaveBehavior behavior(50.0F, 2.0F);

    std::vector<float> yVelocities;
    for (int i = 0; i < 10; ++i) {
        behavior.apply(ai, transform, velocity, 0.1F);
        yVelocities.push_back(velocity.vy);
    }

    // Check that Y velocity changes over time
    bool hasVariation = false;
    for (size_t i = 1; i < yVelocities.size(); ++i) {
        if (std::abs(yVelocities[i] - yVelocities[i - 1]) > 0.001F) {
            hasVariation = true;
            break;
        }
    }
    EXPECT_TRUE(hasVariation);
}

TEST_F(SineWaveBehaviorTest, ApplyAtTimerZero) {
    SineWaveBehavior behavior(50.0F, 2.0F);
    ai.stateTimer = 0.0F;
    behavior.apply(ai, transform, velocity, 0.0F);
    // cos(0) = 1, so vy should be amplitude * frequency * 1
    EXPECT_FLOAT_EQ(velocity.vy, 50.0F * 2.0F);
}

// =============================================================================
// ChaseBehavior Tests
// =============================================================================

class ChaseBehaviorTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ai.behavior = AIBehavior::Chase;
        ai.speed = 100.0F;
        ai.targetX = 0.0F;
        ai.targetY = 0.0F;
        transform.x = 100.0F;
        transform.y = 0.0F;
        velocity.vx = 0.0F;
        velocity.vy = 0.0F;
    }

    AIComponent ai;
    TransformComponent transform;
    VelocityComponent velocity;
};

TEST(ChaseBehaviorConstructorTest, DefaultStopDistance) {
    ChaseBehavior behavior;
    EXPECT_EQ(behavior.getType(), AIBehavior::Chase);
    EXPECT_EQ(behavior.getName(), "ChaseBehavior");
}

TEST(ChaseBehaviorConstructorTest, CustomStopDistance) {
    ChaseBehavior behavior(5.0F);
    EXPECT_EQ(behavior.getType(), AIBehavior::Chase);
}

TEST_F(ChaseBehaviorTest, ApplyMovesTowardTargetHorizontally) {
    ChaseBehavior behavior;
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    transform.x = 100.0F;
    transform.y = 0.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    EXPECT_LT(velocity.vx, 0.0F);  // Moving left toward target
    EXPECT_NEAR(velocity.vy, 0.0F, 0.001F);
}

TEST_F(ChaseBehaviorTest, ApplyMovesTowardTargetVertically) {
    ChaseBehavior behavior;
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    transform.x = 0.0F;
    transform.y = 100.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    EXPECT_NEAR(velocity.vx, 0.0F, 0.001F);
    EXPECT_LT(velocity.vy, 0.0F);  // Moving up toward target
}

TEST_F(ChaseBehaviorTest, ApplyMovesAtCorrectSpeed) {
    ChaseBehavior behavior;
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    transform.x = 100.0F;
    transform.y = 0.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    float speed = std::sqrt(velocity.vx * velocity.vx + velocity.vy * velocity.vy);
    EXPECT_NEAR(speed, ai.speed, 0.01F);
}

TEST_F(ChaseBehaviorTest, ApplyStopsWhenCloseToTarget) {
    ChaseBehavior behavior(5.0F);
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    transform.x = 0.5F;  // Very close to target
    transform.y = 0.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(ChaseBehaviorTest, ApplyStopsExactlyAtStopDistance) {
    ChaseBehavior behavior(1.0F);
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    transform.x = 1.0F;  // Exactly at stop distance
    transform.y = 0.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(ChaseBehaviorTest, ApplyDiagonalChase) {
    ChaseBehavior behavior;
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    transform.x = 100.0F;
    transform.y = 100.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    // Both velocities should be negative (moving toward origin)
    EXPECT_LT(velocity.vx, 0.0F);
    EXPECT_LT(velocity.vy, 0.0F);

    // Speed should still be correct
    float speed = std::sqrt(velocity.vx * velocity.vx + velocity.vy * velocity.vy);
    EXPECT_NEAR(speed, ai.speed, 0.01F);
}

TEST_F(ChaseBehaviorTest, ApplyChaseAwayFromOrigin) {
    ChaseBehavior behavior;
    ai.targetX = 200.0F;
    ai.targetY = 200.0F;
    transform.x = 0.0F;
    transform.y = 0.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    EXPECT_GT(velocity.vx, 0.0F);
    EXPECT_GT(velocity.vy, 0.0F);
}

// =============================================================================
// PatrolBehavior Tests
// =============================================================================

class PatrolBehaviorTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ai.behavior = AIBehavior::Patrol;
        ai.speed = 100.0F;
        transform.x = 500.0F;
        transform.y = 300.0F;
        velocity.vx = 0.0F;
        velocity.vy = 0.0F;
    }

    PatrolBehavior behavior;
    AIComponent ai;
    TransformComponent transform;
    VelocityComponent velocity;
};

TEST_F(PatrolBehaviorTest, GetTypeReturnsPatrol) {
    EXPECT_EQ(behavior.getType(), AIBehavior::Patrol);
}

TEST_F(PatrolBehaviorTest, GetNameReturnsCorrectName) {
    EXPECT_EQ(behavior.getName(), "PatrolBehavior");
}

TEST_F(PatrolBehaviorTest, ApplySetsNegativeXVelocity) {
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
}

TEST_F(PatrolBehaviorTest, ApplySetsZeroYVelocity) {
    velocity.vy = 50.0F;
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

// =============================================================================
// StationaryBehavior Tests
// =============================================================================

class StationaryBehaviorTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ai.behavior = AIBehavior::Stationary;
        ai.speed = 100.0F;
        transform.x = 500.0F;
        transform.y = 300.0F;
        velocity.vx = 50.0F;
        velocity.vy = -30.0F;
    }

    StationaryBehavior behavior;
    AIComponent ai;
    TransformComponent transform;
    VelocityComponent velocity;
};

TEST_F(StationaryBehaviorTest, GetTypeReturnsStationary) {
    EXPECT_EQ(behavior.getType(), AIBehavior::Stationary);
}

TEST_F(StationaryBehaviorTest, GetNameReturnsCorrectName) {
    EXPECT_EQ(behavior.getName(), "StationaryBehavior");
}

TEST_F(StationaryBehaviorTest, ApplySetsZeroXVelocity) {
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
}

TEST_F(StationaryBehaviorTest, ApplySetsZeroYVelocity) {
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(StationaryBehaviorTest, ApplyResetsExistingVelocity) {
    velocity.vx = 100.0F;
    velocity.vy = 200.0F;
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(StationaryBehaviorTest, ApplyIgnoresAISpeed) {
    ai.speed = 500.0F;
    behavior.apply(ai, transform, velocity, 0.016F);
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

// =============================================================================
// ChaseBehavior Edge Cases
// =============================================================================

TEST_F(ChaseBehaviorTest, ApplyWhenAtTarget) {
    ChaseBehavior behavior;
    ai.targetX = 100.0F;
    ai.targetY = 100.0F;
    transform.x = 100.0F;
    transform.y = 100.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    // Should stop when at target (distance = 0, which is < stopDistance)
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(ChaseBehaviorTest, ApplyJustOutsideStopDistance) {
    ChaseBehavior behavior(1.0F);  // Stop distance of 1
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    transform.x = 1.5F;  // Just outside stop distance
    transform.y = 0.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    // Should still be moving
    EXPECT_NE(velocity.vx, 0.0F);
}

TEST_F(ChaseBehaviorTest, ApplyWithZeroSpeed) {
    ChaseBehavior behavior;
    ai.speed = 0.0F;
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    transform.x = 100.0F;
    transform.y = 0.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    // With zero speed, velocity should be zero
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(ChaseBehaviorTest, ApplyWithNegativeTargetCoordinates) {
    ChaseBehavior behavior;
    ai.targetX = -100.0F;
    ai.targetY = -100.0F;
    transform.x = 0.0F;
    transform.y = 0.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    // Should move toward negative coordinates
    EXPECT_LT(velocity.vx, 0.0F);
    EXPECT_LT(velocity.vy, 0.0F);
}

TEST_F(ChaseBehaviorTest, ApplyWithLargeStopDistance) {
    ChaseBehavior behavior(1000.0F);  // Very large stop distance
    ai.targetX = 0.0F;
    ai.targetY = 0.0F;
    transform.x = 100.0F;
    transform.y = 0.0F;

    behavior.apply(ai, transform, velocity, 0.016F);

    // Should stop because within large stop distance
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

// =============================================================================
// SineWaveBehavior Edge Cases
// =============================================================================

TEST_F(SineWaveBehaviorTest, ApplyWithZeroAmplitude) {
    SineWaveBehavior behavior(0.0F, 2.0F);
    behavior.apply(ai, transform, velocity, 0.5F);

    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(SineWaveBehaviorTest, ApplyWithZeroFrequency) {
    SineWaveBehavior behavior(50.0F, 0.0F);
    behavior.apply(ai, transform, velocity, 0.5F);

    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);  // cos(0) * 0 = 0
}

TEST_F(SineWaveBehaviorTest, ApplyMultipleTimes) {
    SineWaveBehavior behavior(50.0F, 2.0F);

    for (int i = 0; i < 10; ++i) {
        behavior.apply(ai, transform, velocity, 0.1F);
    }

    EXPECT_FLOAT_EQ(ai.stateTimer, 1.0F);
    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
}

TEST_F(SineWaveBehaviorTest, ApplyWithLargeStateTimer) {
    SineWaveBehavior behavior(50.0F, 2.0F);
    ai.stateTimer = 1000.0F;

    behavior.apply(ai, transform, velocity, 0.1F);

    EXPECT_FLOAT_EQ(ai.stateTimer, 1000.1F);
    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
}

// =============================================================================
// MoveLeftBehavior Edge Cases
// =============================================================================

TEST_F(MoveLeftBehaviorTest, ApplyWithNegativeSpeed) {
    ai.speed = -100.0F;
    behavior.apply(ai, transform, velocity, 0.016F);

    // -(-100) = 100, so entity would move right
    EXPECT_FLOAT_EQ(velocity.vx, 100.0F);
}

TEST_F(MoveLeftBehaviorTest, ApplyMultipleTimes) {
    for (int i = 0; i < 10; ++i) {
        behavior.apply(ai, transform, velocity, 0.016F);
    }

    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

// =============================================================================
// PatrolBehavior Edge Cases
// =============================================================================

TEST_F(PatrolBehaviorTest, ApplyWithZeroSpeed) {
    ai.speed = 0.0F;
    behavior.apply(ai, transform, velocity, 0.016F);

    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST_F(PatrolBehaviorTest, ApplyMultipleTimes) {
    for (int i = 0; i < 10; ++i) {
        behavior.apply(ai, transform, velocity, 0.016F);
    }

    EXPECT_FLOAT_EQ(velocity.vx, -ai.speed);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}
