/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_components - Unit tests for shared components
*/

#include <gtest/gtest.h>

#include "../../../src/games/rtype/shared/Components.hpp"

using namespace rtype::games::rtype::shared;

// =============================================================================
// HealthComponent Tests
// =============================================================================

class HealthComponentTest : public ::testing::Test {
   protected:
    HealthComponent health;
};

TEST_F(HealthComponentTest, DefaultValues) {
    EXPECT_EQ(health.current, 100);
    EXPECT_EQ(health.max, 100);
    EXPECT_TRUE(health.isAlive());
}

TEST_F(HealthComponentTest, IsAliveWhenHealthPositive) {
    health.current = 1;
    EXPECT_TRUE(health.isAlive());
}

TEST_F(HealthComponentTest, IsDeadWhenHealthZero) {
    health.current = 0;
    EXPECT_FALSE(health.isAlive());
}

TEST_F(HealthComponentTest, IsDeadWhenHealthNegative) {
    health.current = -10;
    EXPECT_FALSE(health.isAlive());
}

TEST_F(HealthComponentTest, TakeDamageReducesHealth) {
    health.takeDamage(30);
    EXPECT_EQ(health.current, 70);
}

TEST_F(HealthComponentTest, TakeDamageCannotGoBelowZero) {
    health.takeDamage(150);
    EXPECT_EQ(health.current, 0);
    EXPECT_FALSE(health.isAlive());
}

TEST_F(HealthComponentTest, TakeDamageExactlyEqualsCurrent) {
    health.takeDamage(100);
    EXPECT_EQ(health.current, 0);
    EXPECT_FALSE(health.isAlive());
}

TEST_F(HealthComponentTest, TakeDamageWithZeroDamage) {
    health.takeDamage(0);
    EXPECT_EQ(health.current, 100);
}

TEST_F(HealthComponentTest, HealIncreasesHealth) {
    health.current = 50;
    health.heal(30);
    EXPECT_EQ(health.current, 80);
}

TEST_F(HealthComponentTest, HealCannotExceedMax) {
    health.current = 80;
    health.heal(50);
    EXPECT_EQ(health.current, health.max);
}

TEST_F(HealthComponentTest, HealExactlyToMax) {
    health.current = 50;
    health.heal(50);
    EXPECT_EQ(health.current, 100);
}

TEST_F(HealthComponentTest, HealWithZeroAmount) {
    health.current = 50;
    health.heal(0);
    EXPECT_EQ(health.current, 50);
}

TEST_F(HealthComponentTest, HealFromZero) {
    health.current = 0;
    health.heal(25);
    EXPECT_EQ(health.current, 25);
    EXPECT_TRUE(health.isAlive());
}

TEST_F(HealthComponentTest, CustomMaxHealth) {
    HealthComponent customHealth;
    customHealth.max = 200;
    customHealth.current = 200;

    customHealth.takeDamage(50);
    EXPECT_EQ(customHealth.current, 150);

    customHealth.heal(100);
    EXPECT_EQ(customHealth.current, 200);
}

// =============================================================================
// AIComponent Tests
// =============================================================================

class AIComponentTest : public ::testing::Test {
   protected:
    AIComponent ai;
};

TEST_F(AIComponentTest, DefaultValues) {
    EXPECT_EQ(ai.behavior, AIBehavior::MoveLeft);
    EXPECT_FLOAT_EQ(ai.speed, 100.0F);
    EXPECT_FLOAT_EQ(ai.stateTimer, 0.0F);
    EXPECT_FLOAT_EQ(ai.targetX, 0.0F);
    EXPECT_FLOAT_EQ(ai.targetY, 0.0F);
}

TEST_F(AIComponentTest, SetBehaviorSineWave) {
    ai.behavior = AIBehavior::SineWave;
    EXPECT_EQ(ai.behavior, AIBehavior::SineWave);
}

TEST_F(AIComponentTest, SetBehaviorChase) {
    ai.behavior = AIBehavior::Chase;
    EXPECT_EQ(ai.behavior, AIBehavior::Chase);
}

TEST_F(AIComponentTest, SetBehaviorPatrol) {
    ai.behavior = AIBehavior::Patrol;
    EXPECT_EQ(ai.behavior, AIBehavior::Patrol);
}

TEST_F(AIComponentTest, SetBehaviorStationary) {
    ai.behavior = AIBehavior::Stationary;
    EXPECT_EQ(ai.behavior, AIBehavior::Stationary);
}

TEST_F(AIComponentTest, SetCustomSpeed) {
    ai.speed = 250.0F;
    EXPECT_FLOAT_EQ(ai.speed, 250.0F);
}

TEST_F(AIComponentTest, SetTargetCoordinates) {
    ai.targetX = 100.0F;
    ai.targetY = 200.0F;
    EXPECT_FLOAT_EQ(ai.targetX, 100.0F);
    EXPECT_FLOAT_EQ(ai.targetY, 200.0F);
}

TEST_F(AIComponentTest, UpdateStateTimer) {
    ai.stateTimer = 5.5F;
    EXPECT_FLOAT_EQ(ai.stateTimer, 5.5F);
}

// =============================================================================
// NetworkIdComponent Tests
// =============================================================================

class NetworkIdComponentTest : public ::testing::Test {
   protected:
    NetworkIdComponent netId;
};

TEST_F(NetworkIdComponentTest, DefaultIsInvalid) {
    EXPECT_EQ(netId.networkId, INVALID_NETWORK_ID);
    EXPECT_FALSE(netId.isValid());
}

TEST_F(NetworkIdComponentTest, ValidIdIsDetected) {
    netId.networkId = 1;
    EXPECT_TRUE(netId.isValid());
}

TEST_F(NetworkIdComponentTest, ZeroIdIsValid) {
    netId.networkId = 0;
    EXPECT_TRUE(netId.isValid());
}

TEST_F(NetworkIdComponentTest, MaxMinusOneIdIsValid) {
    netId.networkId = INVALID_NETWORK_ID - 1;
    EXPECT_TRUE(netId.isValid());
}

TEST_F(NetworkIdComponentTest, InvalidNetworkIdConstant) {
    EXPECT_EQ(INVALID_NETWORK_ID, std::numeric_limits<uint32_t>::max());
}

// =============================================================================
// TransformComponent Tests
// =============================================================================

TEST(TransformComponentTest, DefaultValues) {
    TransformComponent transform;
    EXPECT_FLOAT_EQ(transform.x, 0.0F);
    EXPECT_FLOAT_EQ(transform.y, 0.0F);
    EXPECT_FLOAT_EQ(transform.rotation, 0.0F);
}

TEST(TransformComponentTest, SetPosition) {
    TransformComponent transform;
    transform.x = 150.5F;
    transform.y = -200.3F;
    EXPECT_FLOAT_EQ(transform.x, 150.5F);
    EXPECT_FLOAT_EQ(transform.y, -200.3F);
}

TEST(TransformComponentTest, SetRotation) {
    TransformComponent transform;
    transform.rotation = 180.0F;
    EXPECT_FLOAT_EQ(transform.rotation, 180.0F);
}

TEST(TransformComponentTest, NegativeRotation) {
    TransformComponent transform;
    transform.rotation = -45.0F;
    EXPECT_FLOAT_EQ(transform.rotation, -45.0F);
}

// =============================================================================
// VelocityComponent Tests
// =============================================================================

TEST(VelocityComponentTest, DefaultValues) {
    VelocityComponent velocity;
    EXPECT_FLOAT_EQ(velocity.vx, 0.0F);
    EXPECT_FLOAT_EQ(velocity.vy, 0.0F);
}

TEST(VelocityComponentTest, SetVelocity) {
    VelocityComponent velocity;
    velocity.vx = 100.0F;
    velocity.vy = -50.0F;
    EXPECT_FLOAT_EQ(velocity.vx, 100.0F);
    EXPECT_FLOAT_EQ(velocity.vy, -50.0F);
}

TEST(VelocityComponentTest, NegativeVelocity) {
    VelocityComponent velocity;
    velocity.vx = -200.0F;
    velocity.vy = -150.0F;
    EXPECT_FLOAT_EQ(velocity.vx, -200.0F);
    EXPECT_FLOAT_EQ(velocity.vy, -150.0F);
}

// =============================================================================
// BoundingBoxComponent Tests
// =============================================================================

TEST(BoundingBoxComponentTest, DefaultValues) {
    BoundingBoxComponent bbox;
    EXPECT_FLOAT_EQ(bbox.width, 32.0F);
    EXPECT_FLOAT_EQ(bbox.height, 32.0F);
}

TEST(BoundingBoxComponentTest, SetCustomSize) {
    BoundingBoxComponent bbox;
    bbox.width = 64.0F;
    bbox.height = 128.0F;
    EXPECT_FLOAT_EQ(bbox.width, 64.0F);
    EXPECT_FLOAT_EQ(bbox.height, 128.0F);
}

// =============================================================================
// EntityType Tests
// =============================================================================

TEST(EntityTypeTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Unknown), 0);
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Player), 1);
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Enemy), 2);
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Projectile), 3);
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Pickup), 4);
    EXPECT_EQ(static_cast<uint8_t>(EntityType::Obstacle), 5);
}

// =============================================================================
// Tag Components Tests
// =============================================================================

TEST(TagComponentsTest, PlayerTagExists) {
    PlayerTag tag;
    (void)tag;  // Just verify it compiles
    SUCCEED();
}

TEST(TagComponentsTest, EnemyTagExists) {
    EnemyTag tag;
    (void)tag;
    SUCCEED();
}

TEST(TagComponentsTest, ProjectileTagExists) {
    ProjectileTag tag;
    (void)tag;
    SUCCEED();
}

TEST(TagComponentsTest, PickupTagExists) {
    PickupTag tag;
    (void)tag;
    SUCCEED();
}

TEST(TagComponentsTest, BydosSlaveTagExists) {
    BydosSlaveTag tag;
    (void)tag;
    SUCCEED();
}

TEST(TagComponentsTest, BydosMasterTagExists) {
    BydosMasterTag tag;
    (void)tag;
    SUCCEED();
}

TEST(TagComponentsTest, DestroyTagExists) {
    DestroyTag tag;
    (void)tag;
    SUCCEED();
}

TEST(TagComponentsTest, InvincibleTagExists) {
    InvincibleTag tag;
    (void)tag;
    SUCCEED();
}

TEST(TagComponentsTest, DisabledTagExists) {
    DisabledTag tag;
    (void)tag;
    SUCCEED();
}

// =============================================================================
// AIBehavior Enum Tests
// =============================================================================

TEST(AIBehaviorEnumTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(AIBehavior::MoveLeft), 0);
    EXPECT_EQ(static_cast<uint8_t>(AIBehavior::SineWave), 1);
    EXPECT_EQ(static_cast<uint8_t>(AIBehavior::Chase), 2);
    EXPECT_EQ(static_cast<uint8_t>(AIBehavior::Patrol), 3);
    EXPECT_EQ(static_cast<uint8_t>(AIBehavior::Stationary), 4);
}
