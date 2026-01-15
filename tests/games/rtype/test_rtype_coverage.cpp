/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_coverage_boost - Additional tests targeting low coverage areas
** Focuses on: ServerNetworkSystem, PlayerInputHandler, Collision branches
*/

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "games/rtype/shared/Components/BossComponent.hpp"
#include "games/rtype/shared/Components/BossPatternComponent.hpp"
#include "games/rtype/shared/Components/BoundingBoxComponent.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"
#include "games/rtype/shared/Components/DamageOnContactComponent.hpp"
#include "games/rtype/shared/Components/EnemyTypeComponent.hpp"
#include "games/rtype/shared/Components/ForcePodComponent.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/LaserBeamComponent.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/PowerUpTypeComponent.hpp"
#include "games/rtype/shared/Components/ProjectileComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "games/rtype/shared/Components/WeakPointComponent.hpp"
#include "games/rtype/shared/Components/WeaponComponent.hpp"
#include "games/rtype/shared/Systems/Collision/AABB.hpp"
#include "games/rtype/shared/Systems/Collision/Rect.hpp"

using namespace rtype::games::rtype::shared;

// =============================================================================
// EnemyTypeComponent String Conversion Tests (branch coverage)
// =============================================================================

TEST(EnemyTypeComponentTest, StringToVariantConversion) {
    EXPECT_EQ(EnemyTypeComponent::stringToVariant("basic"),
              EnemyVariant::Basic);
    EXPECT_EQ(EnemyTypeComponent::stringToVariant("shooter"),
              EnemyVariant::Shooter);
    EXPECT_EQ(EnemyTypeComponent::stringToVariant("chaser"),
              EnemyVariant::Chaser);
    EXPECT_EQ(EnemyTypeComponent::stringToVariant("wave"), EnemyVariant::Wave);
    EXPECT_EQ(EnemyTypeComponent::stringToVariant("patrol"),
              EnemyVariant::Patrol);
    EXPECT_EQ(EnemyTypeComponent::stringToVariant("heavy"), EnemyVariant::Heavy);
    EXPECT_EQ(EnemyTypeComponent::stringToVariant("boss"), EnemyVariant::Boss);
    EXPECT_EQ(EnemyTypeComponent::stringToVariant("boss_1"), EnemyVariant::Boss);
    EXPECT_EQ(EnemyTypeComponent::stringToVariant("unknown"),
              EnemyVariant::Unknown);
    EXPECT_EQ(EnemyTypeComponent::stringToVariant(""),
              EnemyVariant::Unknown);
}

TEST(EnemyTypeComponentTest, VariantToStringConversion) {
    EXPECT_EQ(EnemyTypeComponent::variantToString(EnemyVariant::Basic), "basic");
    EXPECT_EQ(EnemyTypeComponent::variantToString(EnemyVariant::Shooter),
              "shooter");
    EXPECT_EQ(EnemyTypeComponent::variantToString(EnemyVariant::Chaser),
              "chaser");
    EXPECT_EQ(EnemyTypeComponent::variantToString(EnemyVariant::Wave), "wave");
    EXPECT_EQ(EnemyTypeComponent::variantToString(EnemyVariant::Patrol),
              "patrol");
    EXPECT_EQ(EnemyTypeComponent::variantToString(EnemyVariant::Heavy), "heavy");
    EXPECT_EQ(EnemyTypeComponent::variantToString(EnemyVariant::Boss), "boss_1");
    EXPECT_EQ(EnemyTypeComponent::variantToString(EnemyVariant::Unknown),
              "basic");  // Default
}

TEST(EnemyTypeComponentTest, DefaultValues) {
    EnemyTypeComponent enemy;
    EXPECT_EQ(enemy.variant, EnemyVariant::Basic);
    EXPECT_TRUE(enemy.configId.empty());
}

TEST(EnemyTypeComponentTest, CustomConstructor) {
    EnemyTypeComponent enemy(EnemyVariant::Shooter, "shooter_1");
    EXPECT_EQ(enemy.variant, EnemyVariant::Shooter);
    EXPECT_EQ(enemy.configId, "shooter_1");
}

// =============================================================================
// ProjectileComponent Tests (branch coverage)
// =============================================================================

TEST(ProjectileComponentTest, DefaultValues) {
    ProjectileComponent proj;
    EXPECT_EQ(proj.damage, 25);
    EXPECT_EQ(proj.ownerNetworkId, 0);
    EXPECT_EQ(proj.owner, ProjectileOwner::Player);
    EXPECT_EQ(proj.type, ProjectileType::BasicBullet);
    EXPECT_FALSE(proj.piercing);
    EXPECT_EQ(proj.maxHits, 1);
    EXPECT_EQ(proj.currentHits, 0);
}

TEST(ProjectileComponentTest, CustomConstructor) {
    ProjectileComponent proj(50, 1234, ProjectileOwner::Enemy,
                             ProjectileType::HeavyBullet);
    EXPECT_EQ(proj.damage, 50);
    EXPECT_EQ(proj.ownerNetworkId, 1234);
    EXPECT_EQ(proj.owner, ProjectileOwner::Enemy);
    EXPECT_EQ(proj.type, ProjectileType::HeavyBullet);
}

TEST(ProjectileComponentTest, RegisterHitNonPiercing) {
    ProjectileComponent proj;
    proj.piercing = false;

    EXPECT_TRUE(proj.registerHit());  // Should be destroyed
    EXPECT_EQ(proj.currentHits, 1);
}

TEST(ProjectileComponentTest, RegisterHitPiercing) {
    ProjectileComponent proj;
    proj.piercing = true;
    proj.maxHits = 3;

    EXPECT_FALSE(proj.registerHit());  // Not destroyed yet
    EXPECT_EQ(proj.currentHits, 1);

    EXPECT_FALSE(proj.registerHit());
    EXPECT_EQ(proj.currentHits, 2);

    EXPECT_TRUE(proj.registerHit());  // Max hits reached
    EXPECT_EQ(proj.currentHits, 3);
}

TEST(ProjectileComponentTest, CanHitPlayerProjectile) {
    ProjectileComponent proj;
    proj.owner = ProjectileOwner::Player;

    EXPECT_FALSE(proj.canHit(true));   // Cannot hit player
    EXPECT_TRUE(proj.canHit(false));   // Can hit enemy
}

TEST(ProjectileComponentTest, CanHitEnemyProjectile) {
    ProjectileComponent proj;
    proj.owner = ProjectileOwner::Enemy;

    EXPECT_TRUE(proj.canHit(true));    // Can hit player
    EXPECT_FALSE(proj.canHit(false));  // Cannot hit enemy
}

TEST(ProjectileComponentTest, CanHitNeutralProjectile) {
    ProjectileComponent proj;
    proj.owner = ProjectileOwner::Neutral;

    EXPECT_TRUE(proj.canHit(true));   // Can hit player
    EXPECT_TRUE(proj.canHit(false));  // Can hit enemy
}

// =============================================================================
// LaserBeamComponent Tests
// =============================================================================

class LaserBeamComponentTest : public ::testing::Test {
   protected:
    LaserBeamComponent laser;
};

TEST_F(LaserBeamComponentTest, DefaultValues) {
    EXPECT_EQ(laser.state, LaserBeamState::Inactive);
    EXPECT_EQ(laser.ownerNetworkId, 0);
    EXPECT_FLOAT_EQ(laser.activeTime, 0.0F);
    EXPECT_FLOAT_EQ(laser.maxDuration, 3.0F);
    EXPECT_FLOAT_EQ(laser.cooldownTime, 0.0F);
    EXPECT_FLOAT_EQ(laser.cooldownDuration, 2.0F);
    EXPECT_FLOAT_EQ(laser.pulsePhase, 0.0F);
    EXPECT_FLOAT_EQ(laser.pulseSpeed, 8.0F);
}

TEST_F(LaserBeamComponentTest, CanFireWhenInactive) {
    laser.state = LaserBeamState::Inactive;
    EXPECT_TRUE(laser.canFire());

    laser.state = LaserBeamState::Active;
    EXPECT_FALSE(laser.canFire());

    laser.state = LaserBeamState::Cooldown;
    EXPECT_FALSE(laser.canFire());
}

TEST_F(LaserBeamComponentTest, IsActiveCheck) {
    laser.state = LaserBeamState::Inactive;
    EXPECT_FALSE(laser.isActive());

    laser.state = LaserBeamState::Active;
    EXPECT_TRUE(laser.isActive());

    laser.state = LaserBeamState::Cooldown;
    EXPECT_FALSE(laser.isActive());
}

TEST_F(LaserBeamComponentTest, IsCoolingDownCheck) {
    laser.state = LaserBeamState::Inactive;
    EXPECT_FALSE(laser.isCoolingDown());

    laser.state = LaserBeamState::Active;
    EXPECT_FALSE(laser.isCoolingDown());

    laser.state = LaserBeamState::Cooldown;
    EXPECT_TRUE(laser.isCoolingDown());
}

TEST_F(LaserBeamComponentTest, GetCooldownProgressWhenNotCooling) {
    laser.state = LaserBeamState::Inactive;
    EXPECT_FLOAT_EQ(laser.getCooldownProgress(), 1.0F);

    laser.state = LaserBeamState::Active;
    EXPECT_FLOAT_EQ(laser.getCooldownProgress(), 1.0F);
}

TEST_F(LaserBeamComponentTest, GetCooldownProgressDuringCooldown) {
    laser.state = LaserBeamState::Cooldown;
    laser.cooldownDuration = 2.0F;
    laser.cooldownTime = 1.0F;  // Half way through

    EXPECT_FLOAT_EQ(laser.getCooldownProgress(), 0.5F);
}

TEST_F(LaserBeamComponentTest, GetCooldownProgressZeroDuration) {
    laser.state = LaserBeamState::Cooldown;
    laser.cooldownDuration = 0.0F;

    EXPECT_FLOAT_EQ(laser.getCooldownProgress(), 1.0F);
}

TEST_F(LaserBeamComponentTest, GetDurationProgressWhenNotActive) {
    laser.state = LaserBeamState::Inactive;
    EXPECT_FLOAT_EQ(laser.getDurationProgress(), 0.0F);

    laser.state = LaserBeamState::Cooldown;
    EXPECT_FLOAT_EQ(laser.getDurationProgress(), 0.0F);
}

TEST_F(LaserBeamComponentTest, GetDurationProgressWhenActive) {
    laser.state = LaserBeamState::Active;
    laser.maxDuration = 3.0F;
    laser.activeTime = 1.5F;  // Half way through

    EXPECT_FLOAT_EQ(laser.getDurationProgress(), 0.5F);
}

TEST_F(LaserBeamComponentTest, GetDurationProgressZeroMax) {
    laser.state = LaserBeamState::Active;
    laser.maxDuration = 0.0F;

    EXPECT_FLOAT_EQ(laser.getDurationProgress(), 0.0F);
}

// =============================================================================
// ForcePodComponent Tests
// =============================================================================

class ForcePodComponentTest : public ::testing::Test {
   protected:
    ForcePodComponent pod;
};

TEST_F(ForcePodComponentTest, DefaultValues) {
    EXPECT_EQ(pod.state, ForcePodState::Attached);
    EXPECT_FLOAT_EQ(pod.offsetX, 0.0F);
    EXPECT_FLOAT_EQ(pod.offsetY, 0.0F);
    EXPECT_EQ(pod.ownerNetworkId, 0);
}

TEST_F(ForcePodComponentTest, StateChanges) {
    pod.state = ForcePodState::Attached;
    EXPECT_EQ(pod.state, ForcePodState::Attached);

    pod.state = ForcePodState::Detached;
    EXPECT_EQ(pod.state, ForcePodState::Detached);

    pod.state = ForcePodState::Returning;
    EXPECT_EQ(pod.state, ForcePodState::Returning);
}

TEST_F(ForcePodComponentTest, CustomOffset) {
    pod.offsetX = 50.0F;
    pod.offsetY = -25.0F;
    EXPECT_FLOAT_EQ(pod.offsetX, 50.0F);
    EXPECT_FLOAT_EQ(pod.offsetY, -25.0F);
}

TEST_F(ForcePodComponentTest, OwnerTracking) {
    pod.ownerNetworkId = 12345;
    EXPECT_EQ(pod.ownerNetworkId, 12345);
}

// =============================================================================
// AABB Collision Tests (branch coverage)
// =============================================================================

TEST(AABBCollisionTest, OverlappingRects) {
    TransformComponent t1{50.0F, 50.0F, 0.0F};
    BoundingBoxComponent b1{20.0F, 20.0F};

    TransformComponent t2{60.0F, 60.0F, 0.0F};
    BoundingBoxComponent b2{20.0F, 20.0F};

    EXPECT_TRUE(collision::overlaps(t1, b1, t2, b2));
}

TEST(AABBCollisionTest, NonOverlappingRects) {
    TransformComponent t1{0.0F, 0.0F, 0.0F};
    BoundingBoxComponent b1{10.0F, 10.0F};

    TransformComponent t2{100.0F, 100.0F, 0.0F};
    BoundingBoxComponent b2{10.0F, 10.0F};

    EXPECT_FALSE(collision::overlaps(t1, b1, t2, b2));
}

TEST(AABBCollisionTest, TouchingEdges) {
    TransformComponent t1{0.0F, 0.0F, 0.0F};
    BoundingBoxComponent b1{10.0F, 10.0F};

    TransformComponent t2{10.0F, 0.0F, 0.0F};
    BoundingBoxComponent b2{10.0F, 10.0F};

    // Note: The overlaps function uses strict < for separation check,
    // so touching edges ARE considered overlapping (not separated)
    EXPECT_TRUE(collision::overlaps(t1, b1, t2, b2));
}

TEST(AABBCollisionTest, OneInsideOther) {
    TransformComponent t1{50.0F, 50.0F, 0.0F};
    BoundingBoxComponent b1{100.0F, 100.0F};

    TransformComponent t2{75.0F, 75.0F, 0.0F};
    BoundingBoxComponent b2{10.0F, 10.0F};

    EXPECT_TRUE(collision::overlaps(t1, b1, t2, b2));
}

TEST(AABBCollisionTest, NegativeCoordinates) {
    TransformComponent t1{-50.0F, -50.0F, 0.0F};
    BoundingBoxComponent b1{20.0F, 20.0F};

    TransformComponent t2{-45.0F, -45.0F, 0.0F};
    BoundingBoxComponent b2{20.0F, 20.0F};

    EXPECT_TRUE(collision::overlaps(t1, b1, t2, b2));
}

// =============================================================================
// Rect Tests (branch coverage)
// =============================================================================

TEST(RectTest, ConstructorDefault) {
    collision::Rect r;
    EXPECT_FLOAT_EQ(r.x, 0.0F);
    EXPECT_FLOAT_EQ(r.y, 0.0F);
    EXPECT_FLOAT_EQ(r.w, 0.0F);
    EXPECT_FLOAT_EQ(r.h, 0.0F);
}

TEST(RectTest, ConstructorWithValues) {
    collision::Rect r(10.0F, 20.0F, 30.0F, 40.0F);
    EXPECT_FLOAT_EQ(r.x, 10.0F);
    EXPECT_FLOAT_EQ(r.y, 20.0F);
    EXPECT_FLOAT_EQ(r.w, 30.0F);
    EXPECT_FLOAT_EQ(r.h, 40.0F);
}

TEST(RectTest, ContainsPoint) {
    collision::Rect r(0.0F, 0.0F, 100.0F, 100.0F);

    EXPECT_TRUE(r.containsPoint(50.0F, 50.0F));
    EXPECT_TRUE(r.containsPoint(0.0F, 0.0F));
    EXPECT_TRUE(r.containsPoint(100.0F, 100.0F));  // Edge is included
    EXPECT_FALSE(r.containsPoint(100.1F, 100.1F));
    EXPECT_FALSE(r.containsPoint(-1.0F, 50.0F));
    EXPECT_FALSE(r.containsPoint(50.0F, -1.0F));
}

TEST(RectTest, IntersectsRect) {
    collision::Rect r1(0.0F, 0.0F, 100.0F, 100.0F);
    collision::Rect r2(50.0F, 50.0F, 100.0F, 100.0F);
    collision::Rect r3(200.0F, 200.0F, 50.0F, 50.0F);

    EXPECT_TRUE(r1.intersects(r2));
    EXPECT_TRUE(r2.intersects(r1));
    EXPECT_FALSE(r1.intersects(r3));
}

TEST(RectTest, EdgeMethods) {
    collision::Rect r(10.0F, 20.0F, 100.0F, 50.0F);

    EXPECT_FLOAT_EQ(r.left(), 10.0F);
    EXPECT_FLOAT_EQ(r.right(), 110.0F);
    EXPECT_FLOAT_EQ(r.top(), 20.0F);
    EXPECT_FLOAT_EQ(r.bottom(), 70.0F);
    EXPECT_FLOAT_EQ(r.centerX(), 60.0F);
    EXPECT_FLOAT_EQ(r.centerY(), 45.0F);
    EXPECT_FLOAT_EQ(r.area(), 5000.0F);
    EXPECT_TRUE(r.isValid());
}

TEST(RectTest, ContainsOtherRect) {
    collision::Rect outer(0.0F, 0.0F, 100.0F, 100.0F);
    collision::Rect inner(25.0F, 25.0F, 50.0F, 50.0F);
    collision::Rect partial(50.0F, 50.0F, 100.0F, 100.0F);

    EXPECT_TRUE(outer.contains(inner));
    EXPECT_FALSE(outer.contains(partial));
    EXPECT_FALSE(inner.contains(outer));
}

TEST(RectTest, InvalidRect) {
    collision::Rect invalid1(0.0F, 0.0F, 0.0F, 10.0F);
    collision::Rect invalid2(0.0F, 0.0F, 10.0F, 0.0F);
    collision::Rect invalid3(0.0F, 0.0F, -10.0F, 10.0F);

    EXPECT_FALSE(invalid1.isValid());
    EXPECT_FALSE(invalid2.isValid());
    EXPECT_FALSE(invalid3.isValid());
}

// =============================================================================
// VelocityComponent Tests
// =============================================================================

TEST(VelocityComponentTest, DefaultValues) {
    VelocityComponent vel;
    EXPECT_FLOAT_EQ(vel.vx, 0.0F);
    EXPECT_FLOAT_EQ(vel.vy, 0.0F);
}

TEST(VelocityComponentTest, CustomValues) {
    VelocityComponent vel{100.0F, -50.0F};
    EXPECT_FLOAT_EQ(vel.vx, 100.0F);
    EXPECT_FLOAT_EQ(vel.vy, -50.0F);
}

// =============================================================================
// TransformComponent Tests
// =============================================================================

TEST(TransformComponentTest, DefaultValues) {
    TransformComponent t;
    EXPECT_FLOAT_EQ(t.x, 0.0F);
    EXPECT_FLOAT_EQ(t.y, 0.0F);
    EXPECT_FLOAT_EQ(t.rotation, 0.0F);
}

TEST(TransformComponentTest, CustomValues) {
    TransformComponent t{100.0F, 200.0F, 45.0F};
    EXPECT_FLOAT_EQ(t.x, 100.0F);
    EXPECT_FLOAT_EQ(t.y, 200.0F);
    EXPECT_FLOAT_EQ(t.rotation, 45.0F);
}

// =============================================================================
// NetworkIdComponent Tests
// =============================================================================

TEST(NetworkIdComponentTest, InvalidIdConstant) {
    EXPECT_EQ(INVALID_NETWORK_ID, std::numeric_limits<uint32_t>::max());
}

TEST(NetworkIdComponentTest, IsValidCheck) {
    NetworkIdComponent netId;
    netId.networkId = INVALID_NETWORK_ID;
    EXPECT_FALSE(netId.isValid());

    netId.networkId = 0;
    EXPECT_TRUE(netId.isValid());

    netId.networkId = 12345;
    EXPECT_TRUE(netId.isValid());
}

// =============================================================================
// BoundingBoxComponent Tests
// =============================================================================

TEST(BoundingBoxComponentTest, DefaultValues) {
    BoundingBoxComponent box;
    // Default values are 32.0F as defined in the component
    EXPECT_FLOAT_EQ(box.width, 32.0F);
    EXPECT_FLOAT_EQ(box.height, 32.0F);
}

TEST(BoundingBoxComponentTest, CustomValues) {
    BoundingBoxComponent box{50.0F, 30.0F};
    EXPECT_FLOAT_EQ(box.width, 50.0F);
    EXPECT_FLOAT_EQ(box.height, 30.0F);
}

// =============================================================================
// HealthComponent Branch Coverage
// =============================================================================

TEST(HealthComponentBranchTest, TakeDamageMultipleTimes) {
    HealthComponent health;
    health.max = 100;
    health.current = 100;

    // Multiple damage applications
    health.takeDamage(20);
    EXPECT_EQ(health.current, 80);

    health.takeDamage(30);
    EXPECT_EQ(health.current, 50);

    health.takeDamage(60);  // More than remaining
    EXPECT_EQ(health.current, 0);
}

TEST(HealthComponentBranchTest, HealMultipleTimes) {
    HealthComponent health;
    health.max = 100;
    health.current = 10;

    health.heal(20);
    EXPECT_EQ(health.current, 30);

    health.heal(50);
    EXPECT_EQ(health.current, 80);

    health.heal(50);  // More than needed to cap
    EXPECT_EQ(health.current, 100);
}

TEST(HealthComponentBranchTest, TakeDamageNegativeValue) {
    HealthComponent health;
    health.max = 100;
    health.current = 100;

    health.takeDamage(-10);  // Negative damage acts as healing
    // The implementation does current -= damage, so negative damage increases health
    EXPECT_EQ(health.current, 110);
}

// =============================================================================
// WeaponComponent Additional Tests
// =============================================================================

TEST(WeaponComponentBranchTest, WeaponSwitchingBoundary) {
    WeaponComponent weapon;
    weapon.unlockedSlots = 3;
    weapon.currentSlot = 0;

    // Switch forward to end
    weapon.nextWeapon();
    weapon.nextWeapon();
    EXPECT_EQ(weapon.currentSlot, 2);

    // Wrap around
    weapon.nextWeapon();
    EXPECT_EQ(weapon.currentSlot, 0);

    // Switch backward
    weapon.previousWeapon();
    EXPECT_EQ(weapon.currentSlot, 2);
}

TEST(WeaponComponentBranchTest, SelectWeaponBoundary) {
    WeaponComponent weapon;
    weapon.unlockedSlots = 3;

    weapon.selectWeapon(0);
    EXPECT_EQ(weapon.currentSlot, 0);

    weapon.selectWeapon(2);
    EXPECT_EQ(weapon.currentSlot, 2);

    weapon.selectWeapon(3);  // Invalid - beyond unlocked
    EXPECT_EQ(weapon.currentSlot, 2);  // Should not change
}

// =============================================================================
// DamageOnContactComponent Additional Tests
// =============================================================================

TEST(DamageOnContactBranchTest, DPSCalculations) {
    DamageOnContactComponent damage;
    damage.isDPS = true;
    damage.damagePerSecond = 60.0F;

    // 60 DPS * 1 second = 60 damage
    EXPECT_EQ(damage.calculateDamage(1.0F), 60);

    // 60 DPS * 0.5 seconds = 30 damage
    EXPECT_EQ(damage.calculateDamage(0.5F), 30);

    // Very small delta - minimum 1 damage
    EXPECT_GE(damage.calculateDamage(0.001F), 1);
}

TEST(DamageOnContactBranchTest, ActiveTimeTracking) {
    DamageOnContactComponent damage;
    damage.isDPS = true;
    damage.startupDelay = 1.0F;
    damage.activeTime = 0.0F;

    EXPECT_FALSE(damage.isActive());

    damage.activeTime = 0.5F;
    EXPECT_FALSE(damage.isActive());

    damage.activeTime = 1.0F;
    EXPECT_TRUE(damage.isActive());

    damage.activeTime = 2.0F;
    EXPECT_TRUE(damage.isActive());
}

// =============================================================================
// BossComponent Position History Branch Coverage
// =============================================================================

TEST(BossComponentBranchTest, PositionHistoryEdgeCases) {
    BossComponent boss;

    // Empty history - segment 0 returns base
    boss.baseX = 100.0F;
    boss.baseY = 200.0F;
    auto pos = boss.getSegmentPosition(0);
    EXPECT_FLOAT_EQ(pos.first, 100.0F);
    EXPECT_FLOAT_EQ(pos.second, 200.0F);

    // Add history
    boss.recordPosition(150.0F, 250.0F);
    pos = boss.getSegmentPosition(0);
    EXPECT_FLOAT_EQ(pos.first, 150.0F);
    EXPECT_FLOAT_EQ(pos.second, 250.0F);

    // Segment beyond history
    pos = boss.getSegmentPosition(10);
    // Should return extrapolated position
}

TEST(BossComponentBranchTest, PhaseTransitionBranches) {
    BossComponent boss;

    BossPhase phase1;
    phase1.healthThreshold = 1.0F;
    phase1.phaseName = "Phase1";

    BossPhase phase2;
    phase2.healthThreshold = 0.5F;
    phase2.phaseName = "Phase2";

    boss.phases.push_back(phase1);
    boss.phases.push_back(phase2);

    // At full health - no transition
    EXPECT_FALSE(boss.checkPhaseTransition(1.0F).has_value());

    // Below threshold - should transition
    auto result = boss.checkPhaseTransition(0.4F);
    EXPECT_TRUE(result.has_value());
}

// =============================================================================
// BossPatternComponent Branch Coverage
// =============================================================================

TEST(BossPatternComponentBranchTest, PatternQueueOperations) {
    BossPatternComponent patterns;

    // Empty queue
    EXPECT_FALSE(patterns.canStartPattern());

    // Add patterns
    patterns.patternQueue.push_back(
        AttackPatternConfig::createCircularShot());
    patterns.enabled = true;
    patterns.state = PatternExecutionState::Idle;
    patterns.globalCooldown = 0.0F;

    EXPECT_TRUE(patterns.canStartPattern());

    // Start pattern
    patterns.startNextPattern();
    EXPECT_FALSE(patterns.canStartPattern());  // Now executing
}

TEST(BossPatternComponentBranchTest, StateTransitions) {
    BossPatternComponent patterns;

    // Default state
    EXPECT_EQ(patterns.state, PatternExecutionState::Idle);
    EXPECT_FALSE(patterns.isExecuting());

    // Telegraph state
    patterns.state = PatternExecutionState::Telegraph;
    EXPECT_TRUE(patterns.isExecuting());

    // Executing state
    patterns.state = PatternExecutionState::Executing;
    EXPECT_TRUE(patterns.isExecuting());

    // Cooldown state
    patterns.state = PatternExecutionState::Cooldown;
    EXPECT_FALSE(patterns.isExecuting());
}

// =============================================================================
// ChargeComponent Branch Coverage
// =============================================================================

TEST(ChargeComponentBranchTest, ChargeLevelTransitions) {
    ChargeComponent charge;
    charge.chargeRate = 1.0F;
    charge.isCharging = true;
    charge.currentCharge = 0.0F;

    // Below Level1
    charge.update(0.2F);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::None);

    // To Level1
    charge.update(0.15F);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::Level1);

    // To Level2
    charge.update(0.35F);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::Level2);

    // To Level3
    charge.update(0.35F);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::Level3);
}

TEST(ChargeComponentBranchTest, ReleaseResetsState) {
    ChargeComponent charge;
    charge.currentCharge = 0.8F;
    charge.currentLevel = ChargeLevel::Level2;
    charge.isCharging = true;
    charge.wasCharging = true;

    auto released = charge.release();

    EXPECT_EQ(released, ChargeLevel::Level2);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::None);
    EXPECT_FLOAT_EQ(charge.currentCharge, 0.0F);
    EXPECT_FALSE(charge.isCharging);
    EXPECT_FALSE(charge.wasCharging);
}
