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

// =============================================================================
// ShootCooldownComponent Tests
// =============================================================================

class ShootCooldownComponentTest : public ::testing::Test {
   protected:
    ShootCooldownComponent cooldown;
};

TEST_F(ShootCooldownComponentTest, DefaultValues) {
    EXPECT_FLOAT_EQ(cooldown.cooldownTime, 0.25F);
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.0F);
    EXPECT_EQ(cooldown.currentWeaponSlot, 0);
}

TEST_F(ShootCooldownComponentTest, ConstructWithCustomCooldown) {
    ShootCooldownComponent custom(0.5F);
    EXPECT_FLOAT_EQ(custom.cooldownTime, 0.5F);
    EXPECT_FLOAT_EQ(custom.currentCooldown, 0.0F);
}

TEST_F(ShootCooldownComponentTest, CanShootWhenCooldownZero) {
    cooldown.currentCooldown = 0.0F;
    EXPECT_TRUE(cooldown.canShoot());
}

TEST_F(ShootCooldownComponentTest, CanShootWhenCooldownNegative) {
    cooldown.currentCooldown = -0.1F;
    EXPECT_TRUE(cooldown.canShoot());
}

TEST_F(ShootCooldownComponentTest, CannotShootWhenCooldownPositive) {
    cooldown.currentCooldown = 0.1F;
    EXPECT_FALSE(cooldown.canShoot());
}

TEST_F(ShootCooldownComponentTest, TriggerCooldownSetsCurrent) {
    cooldown.triggerCooldown();
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, cooldown.cooldownTime);
}

TEST_F(ShootCooldownComponentTest, UpdateReducesCooldown) {
    cooldown.currentCooldown = 0.5F;
    cooldown.update(0.2F);
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.3F);
}

TEST_F(ShootCooldownComponentTest, UpdateClampsToZero) {
    cooldown.currentCooldown = 0.1F;
    cooldown.update(0.5F);
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.0F);
}

TEST_F(ShootCooldownComponentTest, UpdateDoesNothingWhenZero) {
    cooldown.currentCooldown = 0.0F;
    cooldown.update(0.1F);
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.0F);
}

TEST_F(ShootCooldownComponentTest, ResetSetsCooldownToZero) {
    cooldown.currentCooldown = 0.5F;
    cooldown.reset();
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.0F);
}

TEST_F(ShootCooldownComponentTest, SetCooldownTimeChangesValue) {
    cooldown.setCooldownTime(1.0F);
    EXPECT_FLOAT_EQ(cooldown.cooldownTime, 1.0F);
}

TEST_F(ShootCooldownComponentTest, SetWeaponSlotChangesSlot) {
    cooldown.setWeaponSlot(2);
    EXPECT_EQ(cooldown.currentWeaponSlot, 2);
}

TEST_F(ShootCooldownComponentTest, FullCycleShootAndRecover) {
    EXPECT_TRUE(cooldown.canShoot());
    cooldown.triggerCooldown();
    EXPECT_FALSE(cooldown.canShoot());
    cooldown.update(0.25F);
    EXPECT_TRUE(cooldown.canShoot());
}

// =============================================================================
// ChargeComponent Tests
// =============================================================================

class ChargeComponentTest : public ::testing::Test {
   protected:
    ChargeComponent charge;
};

TEST_F(ChargeComponentTest, DefaultValues) {
    EXPECT_FLOAT_EQ(charge.currentCharge, 0.0F);
    EXPECT_FLOAT_EQ(charge.chargeRate, 0.5F);
    EXPECT_FLOAT_EQ(charge.maxCharge, 1.0F);
    EXPECT_FALSE(charge.isCharging);
    EXPECT_FLOAT_EQ(charge.minChargeThreshold, 0.0F);
}

TEST_F(ChargeComponentTest, ConstructWithCustomRate) {
    ChargeComponent custom(1.0F);
    EXPECT_FLOAT_EQ(custom.chargeRate, 1.0F);
}

TEST_F(ChargeComponentTest, StartChargingSetsFlag) {
    charge.startCharging();
    EXPECT_TRUE(charge.isCharging);
}

TEST_F(ChargeComponentTest, ReleaseReturnsChargeAndResets) {
    charge.currentCharge = 0.75F;
    charge.isCharging = true;
    float released = charge.release();
    EXPECT_FLOAT_EQ(released, 0.75F);
    EXPECT_FLOAT_EQ(charge.currentCharge, 0.0F);
    EXPECT_FALSE(charge.isCharging);
}

TEST_F(ChargeComponentTest, UpdateIncreasesChargeWhenCharging) {
    charge.startCharging();
    charge.update(1.0F);
    EXPECT_FLOAT_EQ(charge.currentCharge, 0.5F);
}

TEST_F(ChargeComponentTest, UpdateDoesNothingWhenNotCharging) {
    charge.update(1.0F);
    EXPECT_FLOAT_EQ(charge.currentCharge, 0.0F);
}

TEST_F(ChargeComponentTest, UpdateClampsToMaxCharge) {
    charge.startCharging();
    charge.update(10.0F);
    EXPECT_FLOAT_EQ(charge.currentCharge, charge.maxCharge);
}

TEST_F(ChargeComponentTest, UpdateDoesNothingWhenAtMax) {
    charge.currentCharge = charge.maxCharge;
    charge.startCharging();
    charge.update(1.0F);
    EXPECT_FLOAT_EQ(charge.currentCharge, charge.maxCharge);
}

TEST_F(ChargeComponentTest, IsPoweredShotWhenAboveThreshold) {
    charge.minChargeThreshold = 0.5F;
    charge.currentCharge = 0.6F;
    EXPECT_TRUE(charge.isPoweredShot());
}

TEST_F(ChargeComponentTest, IsNotPoweredShotWhenBelowThreshold) {
    charge.minChargeThreshold = 0.5F;
    charge.currentCharge = 0.4F;
    EXPECT_FALSE(charge.isPoweredShot());
}

TEST_F(ChargeComponentTest, IsPoweredShotWhenAtThreshold) {
    charge.minChargeThreshold = 0.5F;
    charge.currentCharge = 0.5F;
    EXPECT_TRUE(charge.isPoweredShot());
}

TEST_F(ChargeComponentTest, GetChargePercentReturnsCorrectValue) {
    charge.currentCharge = 0.5F;
    EXPECT_FLOAT_EQ(charge.getChargePercent(), 0.5F);
}

TEST_F(ChargeComponentTest, GetChargePercentAtMax) {
    charge.currentCharge = charge.maxCharge;
    EXPECT_FLOAT_EQ(charge.getChargePercent(), 1.0F);
}

TEST_F(ChargeComponentTest, GetChargePercentAtZero) {
    charge.currentCharge = 0.0F;
    EXPECT_FLOAT_EQ(charge.getChargePercent(), 0.0F);
}

// =============================================================================
// ProjectileComponent Tests
// =============================================================================

class ProjectileComponentTest : public ::testing::Test {
   protected:
    ProjectileComponent proj;
};

TEST_F(ProjectileComponentTest, DefaultValues) {
    EXPECT_EQ(proj.damage, 25);
    EXPECT_EQ(proj.ownerNetworkId, 0U);
    EXPECT_EQ(proj.owner, ProjectileOwner::Player);
    EXPECT_EQ(proj.type, ProjectileType::BasicBullet);
    EXPECT_FALSE(proj.piercing);
    EXPECT_EQ(proj.maxHits, 1);
    EXPECT_EQ(proj.currentHits, 0);
}

TEST_F(ProjectileComponentTest, ConstructWithParameters) {
    ProjectileComponent custom(50, 42U, ProjectileOwner::Enemy,
                               ProjectileType::HeavyBullet);
    EXPECT_EQ(custom.damage, 50);
    EXPECT_EQ(custom.ownerNetworkId, 42U);
    EXPECT_EQ(custom.owner, ProjectileOwner::Enemy);
    EXPECT_EQ(custom.type, ProjectileType::HeavyBullet);
}

TEST_F(ProjectileComponentTest, RegisterHitNonPiercing) {
    proj.piercing = false;
    bool shouldDestroy = proj.registerHit();
    EXPECT_TRUE(shouldDestroy);
    EXPECT_EQ(proj.currentHits, 1);
}

TEST_F(ProjectileComponentTest, RegisterHitPiercingBelowMax) {
    proj.piercing = true;
    proj.maxHits = 3;
    bool shouldDestroy = proj.registerHit();
    EXPECT_FALSE(shouldDestroy);
    EXPECT_EQ(proj.currentHits, 1);
}

TEST_F(ProjectileComponentTest, RegisterHitPiercingAtMax) {
    proj.piercing = true;
    proj.maxHits = 2;
    proj.currentHits = 1;
    bool shouldDestroy = proj.registerHit();
    EXPECT_TRUE(shouldDestroy);
    EXPECT_EQ(proj.currentHits, 2);
}

TEST_F(ProjectileComponentTest, RegisterHitPiercingAboveMax) {
    proj.piercing = true;
    proj.maxHits = 2;
    proj.currentHits = 2;
    bool shouldDestroy = proj.registerHit();
    EXPECT_TRUE(shouldDestroy);
}

TEST_F(ProjectileComponentTest, CanHitPlayerProjectileHitsEnemy) {
    proj.owner = ProjectileOwner::Player;
    EXPECT_TRUE(proj.canHit(false));   // Can hit enemy
    EXPECT_FALSE(proj.canHit(true));   // Cannot hit player
}

TEST_F(ProjectileComponentTest, CanHitEnemyProjectileHitsPlayer) {
    proj.owner = ProjectileOwner::Enemy;
    EXPECT_TRUE(proj.canHit(true));    // Can hit player
    EXPECT_FALSE(proj.canHit(false));  // Cannot hit enemy
}

TEST_F(ProjectileComponentTest, CanHitNeutralHitsEveryone) {
    proj.owner = ProjectileOwner::Neutral;
    EXPECT_TRUE(proj.canHit(true));   // Can hit player
    EXPECT_TRUE(proj.canHit(false));  // Can hit enemy
}

TEST_F(ProjectileComponentTest, ProjectileTypeValues) {
    EXPECT_EQ(static_cast<uint8_t>(ProjectileType::BasicBullet), 0);
    EXPECT_EQ(static_cast<uint8_t>(ProjectileType::ChargedShot), 1);
    EXPECT_EQ(static_cast<uint8_t>(ProjectileType::Missile), 2);
    EXPECT_EQ(static_cast<uint8_t>(ProjectileType::LaserBeam), 3);
    EXPECT_EQ(static_cast<uint8_t>(ProjectileType::SpreadShot), 4);
    EXPECT_EQ(static_cast<uint8_t>(ProjectileType::EnemyBullet), 50);
    EXPECT_EQ(static_cast<uint8_t>(ProjectileType::HeavyBullet), 51);
    EXPECT_EQ(static_cast<uint8_t>(ProjectileType::BossBullet), 52);
}

TEST_F(ProjectileComponentTest, ProjectileOwnerValues) {
    EXPECT_EQ(static_cast<uint8_t>(ProjectileOwner::Player), 0);
    EXPECT_EQ(static_cast<uint8_t>(ProjectileOwner::Enemy), 1);
    EXPECT_EQ(static_cast<uint8_t>(ProjectileOwner::Neutral), 2);
}

// =============================================================================
// PlayerProjectileTag and EnemyProjectileTag Tests
// =============================================================================

TEST(ProjectileTagsTest, PlayerProjectileTagExists) {
    PlayerProjectileTag tag;
    (void)tag;
    SUCCEED();
}

TEST(ProjectileTagsTest, EnemyProjectileTagExists) {
    EnemyProjectileTag tag;
    (void)tag;
    SUCCEED();
}
