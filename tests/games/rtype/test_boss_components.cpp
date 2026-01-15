/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_boss_components - Unit tests for boss-related components
** Covers: BossComponent, WeakPointComponent, BossPatternComponent,
**         WeaponComponent, DamageOnContactComponent
*/

#include <gtest/gtest.h>

#include "games/rtype/shared/Components/BossComponent.hpp"
#include "games/rtype/shared/Components/BossPatternComponent.hpp"
#include "games/rtype/shared/Components/DamageOnContactComponent.hpp"
#include "games/rtype/shared/Components/WeakPointComponent.hpp"
#include "games/rtype/shared/Components/WeaponComponent.hpp"

using namespace rtype::games::rtype::shared;

// =============================================================================
// BossPhase Tests
// =============================================================================

class BossPhaseTest : public ::testing::Test {
   protected:
    BossPhase phase;
};

TEST_F(BossPhaseTest, DefaultValues) {
    EXPECT_FLOAT_EQ(phase.healthThreshold, 1.0F);
    EXPECT_EQ(phase.primaryPattern, BossAttackPattern::None);
    EXPECT_EQ(phase.secondaryPattern, BossAttackPattern::None);
    EXPECT_FLOAT_EQ(phase.speedMultiplier, 1.0F);
    EXPECT_FLOAT_EQ(phase.attackSpeedMultiplier, 1.0F);
    EXPECT_FLOAT_EQ(phase.damageMultiplier, 1.0F);
    EXPECT_TRUE(phase.phaseName.empty());
    EXPECT_EQ(phase.colorR, 255);
    EXPECT_EQ(phase.colorG, 255);
    EXPECT_EQ(phase.colorB, 255);
}

TEST_F(BossPhaseTest, ShouldActivateAtThreshold) {
    phase.healthThreshold = 0.75F;
    EXPECT_TRUE(phase.shouldActivate(0.75F));
    EXPECT_TRUE(phase.shouldActivate(0.50F));
    EXPECT_TRUE(phase.shouldActivate(0.25F));
    EXPECT_TRUE(phase.shouldActivate(0.0F));
}

TEST_F(BossPhaseTest, ShouldNotActivateAboveThreshold) {
    phase.healthThreshold = 0.75F;
    EXPECT_FALSE(phase.shouldActivate(0.76F));
    EXPECT_FALSE(phase.shouldActivate(1.0F));
    EXPECT_FALSE(phase.shouldActivate(0.90F));
}

TEST_F(BossPhaseTest, EdgeCaseExactThreshold) {
    phase.healthThreshold = 0.50F;
    EXPECT_TRUE(phase.shouldActivate(0.50F));
}

TEST_F(BossPhaseTest, CustomPhaseConfiguration) {
    phase.healthThreshold = 0.25F;
    phase.primaryPattern = BossAttackPattern::CircularShot;
    phase.secondaryPattern = BossAttackPattern::LaserSweep;
    phase.speedMultiplier = 2.0F;
    phase.attackSpeedMultiplier = 1.5F;
    phase.damageMultiplier = 2.0F;
    phase.phaseName = "Enraged";
    phase.colorR = 255;
    phase.colorG = 0;
    phase.colorB = 0;

    EXPECT_FLOAT_EQ(phase.healthThreshold, 0.25F);
    EXPECT_EQ(phase.primaryPattern, BossAttackPattern::CircularShot);
    EXPECT_EQ(phase.secondaryPattern, BossAttackPattern::LaserSweep);
    EXPECT_FLOAT_EQ(phase.speedMultiplier, 2.0F);
    EXPECT_FLOAT_EQ(phase.attackSpeedMultiplier, 1.5F);
    EXPECT_FLOAT_EQ(phase.damageMultiplier, 2.0F);
    EXPECT_EQ(phase.phaseName, "Enraged");
}

// =============================================================================
// BossComponent Tests
// =============================================================================

class BossComponentTest : public ::testing::Test {
   protected:
    BossComponent boss;

    void SetUp() override {
        // Set up a boss with 3 phases
        BossPhase phase1;
        phase1.healthThreshold = 1.0F;
        phase1.phaseName = "Phase1";

        BossPhase phase2;
        phase2.healthThreshold = 0.66F;
        phase2.phaseName = "Phase2";

        BossPhase phase3;
        phase3.healthThreshold = 0.33F;
        phase3.phaseName = "Phase3";

        boss.phases.push_back(phase1);
        boss.phases.push_back(phase2);
        boss.phases.push_back(phase3);
    }
};

TEST_F(BossComponentTest, DefaultValues) {
    BossComponent defaultBoss;
    EXPECT_EQ(defaultBoss.bossType, BossType::Generic);
    EXPECT_TRUE(defaultBoss.bossId.empty());
    EXPECT_TRUE(defaultBoss.phases.empty());
    EXPECT_EQ(defaultBoss.currentPhaseIndex, 0);
    EXPECT_FALSE(defaultBoss.phaseTransitionActive);
    EXPECT_FLOAT_EQ(defaultBoss.phaseTransitionTimer, 0.0F);
    EXPECT_FLOAT_EQ(defaultBoss.phaseTransitionDuration, 1.0F);
    EXPECT_FLOAT_EQ(defaultBoss.invulnerabilityTimer, 0.0F);
    EXPECT_EQ(defaultBoss.scoreValue, 5000);
    EXPECT_FALSE(defaultBoss.defeated);
    EXPECT_TRUE(defaultBoss.levelCompleteTrigger);
}

TEST_F(BossComponentTest, GetCurrentPhaseValid) {
    const BossPhase* currentPhase = boss.getCurrentPhase();
    ASSERT_NE(currentPhase, nullptr);
    EXPECT_EQ(currentPhase->phaseName, "Phase1");
}

TEST_F(BossComponentTest, GetCurrentPhaseEmpty) {
    BossComponent emptyBoss;
    EXPECT_EQ(emptyBoss.getCurrentPhase(), nullptr);
}

TEST_F(BossComponentTest, GetCurrentPhaseMutableValid) {
    BossPhase* currentPhase = boss.getCurrentPhase();
    ASSERT_NE(currentPhase, nullptr);
    currentPhase->phaseName = "Modified";
    EXPECT_EQ(boss.phases[0].phaseName, "Modified");
}

TEST_F(BossComponentTest, GetCurrentPhaseMutableEmpty) {
    BossComponent emptyBoss;
    EXPECT_EQ(emptyBoss.getCurrentPhase(), nullptr);
}

TEST_F(BossComponentTest, GetCurrentPhaseOutOfRange) {
    boss.currentPhaseIndex = 100;
    EXPECT_EQ(boss.getCurrentPhase(), nullptr);
}

TEST_F(BossComponentTest, CheckPhaseTransitionNoChange) {
    // Health at full - should stay at phase 0
    auto result = boss.checkPhaseTransition(1.0F);
    EXPECT_FALSE(result.has_value());
}

TEST_F(BossComponentTest, CheckPhaseTransitionToPhase2) {
    // Health below 0.66 threshold
    auto result = boss.checkPhaseTransition(0.60F);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1);
}

TEST_F(BossComponentTest, CheckPhaseTransitionToPhase3) {
    // Health below 0.33 threshold, should jump to phase 3
    auto result = boss.checkPhaseTransition(0.30F);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1);  // First transition from phase 0
}

TEST_F(BossComponentTest, CheckPhaseTransitionAlreadyInLastPhase) {
    boss.currentPhaseIndex = 2;  // Already in last phase
    auto result = boss.checkPhaseTransition(0.10F);
    EXPECT_FALSE(result.has_value());
}

TEST_F(BossComponentTest, TransitionToPhase) {
    boss.transitionToPhase(1);
    EXPECT_EQ(boss.currentPhaseIndex, 1);
    EXPECT_TRUE(boss.phaseTransitionActive);
    EXPECT_FLOAT_EQ(boss.phaseTransitionTimer, 0.0F);
}

TEST_F(BossComponentTest, TransitionToInvalidPhase) {
    std::size_t originalIndex = boss.currentPhaseIndex;
    boss.transitionToPhase(100);  // Invalid index
    EXPECT_EQ(boss.currentPhaseIndex, originalIndex);  // Should not change
}

TEST_F(BossComponentTest, HasPhases) {
    EXPECT_TRUE(boss.hasPhases());

    BossComponent emptyBoss;
    EXPECT_FALSE(emptyBoss.hasPhases());
}

TEST_F(BossComponentTest, GetPhaseCount) {
    EXPECT_EQ(boss.getPhaseCount(), 3);

    BossComponent emptyBoss;
    EXPECT_EQ(emptyBoss.getPhaseCount(), 0);
}

TEST_F(BossComponentTest, IsInvulnerableWithTimer) {
    boss.invulnerabilityTimer = 1.0F;
    EXPECT_TRUE(boss.isInvulnerable());
}

TEST_F(BossComponentTest, IsInvulnerableDuringTransition) {
    boss.phaseTransitionActive = true;
    EXPECT_TRUE(boss.isInvulnerable());
}

TEST_F(BossComponentTest, IsNotInvulnerable) {
    boss.invulnerabilityTimer = 0.0F;
    boss.phaseTransitionActive = false;
    EXPECT_FALSE(boss.isInvulnerable());
}

TEST_F(BossComponentTest, RecordPositionEmpty) {
    boss.recordPosition(100.0F, 200.0F);
    ASSERT_EQ(boss.positionHistory.size(), 1);
    EXPECT_FLOAT_EQ(boss.positionHistory.front().first, 100.0F);
    EXPECT_FLOAT_EQ(boss.positionHistory.front().second, 200.0F);
}

TEST_F(BossComponentTest, RecordPositionTooClose) {
    boss.recordPosition(100.0F, 200.0F);
    // Record position too close - should be ignored
    boss.recordPosition(101.0F, 201.0F);
    EXPECT_EQ(boss.positionHistory.size(), 1);
}

TEST_F(BossComponentTest, RecordPositionFarEnough) {
    boss.recordPosition(100.0F, 200.0F);
    // Record position far enough
    boss.recordPosition(110.0F, 200.0F);
    EXPECT_EQ(boss.positionHistory.size(), 2);
}

TEST_F(BossComponentTest, RecordPositionMaxHistory) {
    // Fill history beyond max
    for (std::size_t i = 0; i < BossComponent::MAX_POSITION_HISTORY + 10; ++i) {
        boss.recordPosition(static_cast<float>(i * 100), 0.0F);
    }
    EXPECT_EQ(boss.positionHistory.size(), BossComponent::MAX_POSITION_HISTORY);
}

TEST_F(BossComponentTest, GetSegmentPositionIndex0WithHistory) {
    boss.recordPosition(100.0F, 200.0F);
    auto pos = boss.getSegmentPosition(0);
    EXPECT_FLOAT_EQ(pos.first, 100.0F);
    EXPECT_FLOAT_EQ(pos.second, 200.0F);
}

TEST_F(BossComponentTest, GetSegmentPositionIndex0EmptyHistory) {
    boss.baseX = 50.0F;
    boss.baseY = 75.0F;
    auto pos = boss.getSegmentPosition(0);
    EXPECT_FLOAT_EQ(pos.first, 50.0F);
    EXPECT_FLOAT_EQ(pos.second, 75.0F);
}

TEST_F(BossComponentTest, GetSegmentPositionValidIndex) {
    // Fill some history
    for (int i = 0; i < 50; ++i) {
        boss.recordPosition(static_cast<float>(i * 10), 0.0F);
    }
    // Get segment at index 1 (historyIndex = 15)
    auto pos = boss.getSegmentPosition(1);
    EXPECT_GT(boss.positionHistory.size(), 15);
}

TEST_F(BossComponentTest, GetSegmentPositionOutOfRange) {
    boss.recordPosition(100.0F, 200.0F);
    // Segment index way out of range
    auto pos = boss.getSegmentPosition(100);
    // Should return last position with extra offset
    EXPECT_NE(pos.first, 0.0F);
}

TEST_F(BossComponentTest, GetSegmentPositionEmptyHistoryFallback) {
    boss.baseX = 500.0F;
    boss.baseY = 300.0F;
    auto pos = boss.getSegmentPosition(2);
    float expectedX = 500.0F - 2.0F * BossComponent::SEGMENT_SPACING;
    EXPECT_FLOAT_EQ(pos.first, expectedX);
    EXPECT_FLOAT_EQ(pos.second, 300.0F);
}

// =============================================================================
// BossType and BossAttackPattern String Conversion Tests
// =============================================================================

TEST(BossEnumConversionTest, BossAttackPatternToString) {
    EXPECT_STREQ(bossAttackPatternToString(BossAttackPattern::None), "None");
    EXPECT_STREQ(bossAttackPatternToString(BossAttackPattern::CircularShot),
                 "CircularShot");
    EXPECT_STREQ(bossAttackPatternToString(BossAttackPattern::SpreadFan),
                 "SpreadFan");
    EXPECT_STREQ(bossAttackPatternToString(BossAttackPattern::LaserSweep),
                 "LaserSweep");
    EXPECT_STREQ(bossAttackPatternToString(BossAttackPattern::MinionSpawn),
                 "MinionSpawn");
    EXPECT_STREQ(bossAttackPatternToString(BossAttackPattern::TailSweep),
                 "TailSweep");
    EXPECT_STREQ(bossAttackPatternToString(BossAttackPattern::ChargeAttack),
                 "ChargeAttack");
    EXPECT_STREQ(bossAttackPatternToString(BossAttackPattern::HomingMissile),
                 "HomingMissile");
    EXPECT_STREQ(bossAttackPatternToString(BossAttackPattern::GroundPound),
                 "GroundPound");
    // Test default case
    EXPECT_STREQ(bossAttackPatternToString(static_cast<BossAttackPattern>(255)),
                 "Unknown");
}

TEST(BossEnumConversionTest, BossTypeToString) {
    EXPECT_STREQ(bossTypeToString(BossType::Generic), "Generic");
    EXPECT_STREQ(bossTypeToString(BossType::Serpent), "Serpent");
    EXPECT_STREQ(bossTypeToString(BossType::Scorpion), "Scorpion");
    EXPECT_STREQ(bossTypeToString(BossType::Battleship), "Battleship");
    EXPECT_STREQ(bossTypeToString(BossType::Hive), "Hive");
    // Test default case
    EXPECT_STREQ(bossTypeToString(static_cast<BossType>(255)), "Unknown");
}

TEST(BossEnumConversionTest, StringToBossAttackPattern) {
    EXPECT_EQ(stringToBossAttackPattern("circular_shot"),
              BossAttackPattern::CircularShot);
    EXPECT_EQ(stringToBossAttackPattern("spread_fan"),
              BossAttackPattern::SpreadFan);
    EXPECT_EQ(stringToBossAttackPattern("laser_sweep"),
              BossAttackPattern::LaserSweep);
    EXPECT_EQ(stringToBossAttackPattern("minion_spawn"),
              BossAttackPattern::MinionSpawn);
    EXPECT_EQ(stringToBossAttackPattern("tail_sweep"),
              BossAttackPattern::TailSweep);
    EXPECT_EQ(stringToBossAttackPattern("charge_attack"),
              BossAttackPattern::ChargeAttack);
    EXPECT_EQ(stringToBossAttackPattern("homing_missile"),
              BossAttackPattern::HomingMissile);
    EXPECT_EQ(stringToBossAttackPattern("ground_pound"),
              BossAttackPattern::GroundPound);
    // Unknown string should return None
    EXPECT_EQ(stringToBossAttackPattern("unknown_pattern"),
              BossAttackPattern::None);
    EXPECT_EQ(stringToBossAttackPattern(""), BossAttackPattern::None);
}

TEST(BossEnumConversionTest, StringToBossType) {
    EXPECT_EQ(stringToBossType("serpent"), BossType::Serpent);
    EXPECT_EQ(stringToBossType("scorpion"), BossType::Scorpion);
    EXPECT_EQ(stringToBossType("battleship"), BossType::Battleship);
    EXPECT_EQ(stringToBossType("hive"), BossType::Hive);
    // Unknown string should return Generic
    EXPECT_EQ(stringToBossType("unknown_boss"), BossType::Generic);
    EXPECT_EQ(stringToBossType(""), BossType::Generic);
}

// =============================================================================
// WeakPointComponent Tests
// =============================================================================

class WeakPointComponentTest : public ::testing::Test {
   protected:
    WeakPointComponent weakPoint;
};

TEST_F(WeakPointComponentTest, DefaultValues) {
    EXPECT_EQ(weakPoint.parentBossNetworkId, 0);
    EXPECT_EQ(weakPoint.type, WeakPointType::Generic);
    EXPECT_TRUE(weakPoint.weakPointId.empty());
    EXPECT_FLOAT_EQ(weakPoint.localOffsetX, 0.0F);
    EXPECT_FLOAT_EQ(weakPoint.localOffsetY, 0.0F);
    EXPECT_FLOAT_EQ(weakPoint.localRotation, 0.0F);
    EXPECT_EQ(weakPoint.segmentIndex, -1);
    EXPECT_EQ(weakPoint.bonusScore, 500);
    EXPECT_EQ(weakPoint.damageToParent, 0);
    EXPECT_FLOAT_EQ(weakPoint.damageMultiplier, 1.0F);
    EXPECT_FALSE(weakPoint.destroyed);
    EXPECT_FALSE(weakPoint.critical);
    EXPECT_FALSE(weakPoint.disablesBossAttack);
    EXPECT_TRUE(weakPoint.disabledAttackPattern.empty());
    EXPECT_FALSE(weakPoint.exposesCore);
}

TEST_F(WeakPointComponentTest, IsActiveWhenValid) {
    weakPoint.parentBossNetworkId = 1;
    weakPoint.destroyed = false;
    EXPECT_TRUE(weakPoint.isActive());
}

TEST_F(WeakPointComponentTest, IsNotActiveWhenDestroyed) {
    weakPoint.parentBossNetworkId = 1;
    weakPoint.destroyed = true;
    EXPECT_FALSE(weakPoint.isActive());
}

TEST_F(WeakPointComponentTest, IsNotActiveWhenNoParent) {
    weakPoint.parentBossNetworkId = 0;
    weakPoint.destroyed = false;
    EXPECT_FALSE(weakPoint.isActive());
}

TEST_F(WeakPointComponentTest, Destroy) {
    weakPoint.destroy();
    EXPECT_TRUE(weakPoint.destroyed);
}

TEST_F(WeakPointComponentTest, GetEffectiveDamageMultiplierNormal) {
    weakPoint.damageMultiplier = 1.5F;
    weakPoint.critical = false;
    EXPECT_FLOAT_EQ(weakPoint.getEffectiveDamageMultiplier(), 1.5F);
}

TEST_F(WeakPointComponentTest, GetEffectiveDamageMultiplierCritical) {
    weakPoint.damageMultiplier = 1.5F;
    weakPoint.critical = true;
    EXPECT_FLOAT_EQ(weakPoint.getEffectiveDamageMultiplier(), 3.0F);  // 1.5 * 2
}

TEST(WeakPointTypeConversionTest, WeakPointTypeToString) {
    EXPECT_STREQ(weakPointTypeToString(WeakPointType::Generic), "Generic");
    EXPECT_STREQ(weakPointTypeToString(WeakPointType::Head), "Head");
    EXPECT_STREQ(weakPointTypeToString(WeakPointType::Tail), "Tail");
    EXPECT_STREQ(weakPointTypeToString(WeakPointType::Core), "Core");
    EXPECT_STREQ(weakPointTypeToString(WeakPointType::Arm), "Arm");
    EXPECT_STREQ(weakPointTypeToString(WeakPointType::Cannon), "Cannon");
    EXPECT_STREQ(weakPointTypeToString(WeakPointType::Engine), "Engine");
    EXPECT_STREQ(weakPointTypeToString(WeakPointType::Shield), "Shield");
    // Test default case
    EXPECT_STREQ(weakPointTypeToString(static_cast<WeakPointType>(255)),
                 "Unknown");
}

TEST(WeakPointTypeConversionTest, StringToWeakPointType) {
    EXPECT_EQ(stringToWeakPointType("head"), WeakPointType::Head);
    EXPECT_EQ(stringToWeakPointType("tail"), WeakPointType::Tail);
    EXPECT_EQ(stringToWeakPointType("core"), WeakPointType::Core);
    EXPECT_EQ(stringToWeakPointType("arm"), WeakPointType::Arm);
    EXPECT_EQ(stringToWeakPointType("cannon"), WeakPointType::Cannon);
    EXPECT_EQ(stringToWeakPointType("engine"), WeakPointType::Engine);
    EXPECT_EQ(stringToWeakPointType("shield"), WeakPointType::Shield);
    // Unknown string should return Generic
    EXPECT_EQ(stringToWeakPointType("unknown"), WeakPointType::Generic);
    EXPECT_EQ(stringToWeakPointType(""), WeakPointType::Generic);
}

// =============================================================================
// AttackPatternConfig Tests
// =============================================================================

class AttackPatternConfigTest : public ::testing::Test {
   protected:
    AttackPatternConfig config;
};

TEST_F(AttackPatternConfigTest, DefaultValues) {
    EXPECT_EQ(config.pattern, BossAttackPattern::None);
    EXPECT_FLOAT_EQ(config.duration, 2.0F);
    EXPECT_FLOAT_EQ(config.cooldown, 1.0F);
    EXPECT_FLOAT_EQ(config.projectileSpeed, 400.0F);
    EXPECT_EQ(config.projectileCount, 8);
    EXPECT_EQ(config.damage, 25);
    EXPECT_FLOAT_EQ(config.spreadAngle, 45.0F);
    EXPECT_FLOAT_EQ(config.rotationSpeed, 90.0F);
    EXPECT_TRUE(config.minionType.empty());
    EXPECT_EQ(config.minionCount, 3);
    EXPECT_FLOAT_EQ(config.telegraphDuration, 0.5F);
    EXPECT_FALSE(config.requiresTarget);
}

TEST_F(AttackPatternConfigTest, CreateCircularShotDefault) {
    auto circularShot = AttackPatternConfig::createCircularShot();
    EXPECT_EQ(circularShot.pattern, BossAttackPattern::CircularShot);
    EXPECT_EQ(circularShot.projectileCount, 12);
    EXPECT_FLOAT_EQ(circularShot.projectileSpeed, 350.0F);
    EXPECT_EQ(circularShot.damage, 15);
    EXPECT_FLOAT_EQ(circularShot.duration, 0.5F);
    EXPECT_FLOAT_EQ(circularShot.cooldown, 2.0F);
}

TEST_F(AttackPatternConfigTest, CreateCircularShotCustom) {
    auto circularShot = AttackPatternConfig::createCircularShot(24, 500.0F, 30);
    EXPECT_EQ(circularShot.projectileCount, 24);
    EXPECT_FLOAT_EQ(circularShot.projectileSpeed, 500.0F);
    EXPECT_EQ(circularShot.damage, 30);
}

TEST_F(AttackPatternConfigTest, CreateSpreadFanDefault) {
    auto spreadFan = AttackPatternConfig::createSpreadFan();
    EXPECT_EQ(spreadFan.pattern, BossAttackPattern::SpreadFan);
    EXPECT_EQ(spreadFan.projectileCount, 5);
    EXPECT_FLOAT_EQ(spreadFan.spreadAngle, 60.0F);
    EXPECT_FLOAT_EQ(spreadFan.projectileSpeed, 400.0F);
    EXPECT_EQ(spreadFan.damage, 20);
    EXPECT_FLOAT_EQ(spreadFan.duration, 0.3F);
    EXPECT_FLOAT_EQ(spreadFan.cooldown, 1.5F);
    EXPECT_TRUE(spreadFan.requiresTarget);
}

TEST_F(AttackPatternConfigTest, CreateSpreadFanCustom) {
    auto spreadFan = AttackPatternConfig::createSpreadFan(7, 90.0F, 600.0F);
    EXPECT_EQ(spreadFan.projectileCount, 7);
    EXPECT_FLOAT_EQ(spreadFan.spreadAngle, 90.0F);
    EXPECT_FLOAT_EQ(spreadFan.projectileSpeed, 600.0F);
}

TEST_F(AttackPatternConfigTest, CreateLaserSweepDefault) {
    auto laserSweep = AttackPatternConfig::createLaserSweep();
    EXPECT_EQ(laserSweep.pattern, BossAttackPattern::LaserSweep);
    EXPECT_FLOAT_EQ(laserSweep.duration, 3.0F);
    EXPECT_FLOAT_EQ(laserSweep.spreadAngle, 120.0F);
    EXPECT_EQ(laserSweep.damage, 30);
    EXPECT_FLOAT_EQ(laserSweep.cooldown, 5.0F);
    EXPECT_FLOAT_EQ(laserSweep.telegraphDuration, 1.0F);
    EXPECT_FLOAT_EQ(laserSweep.rotationSpeed, 40.0F);  // 120 / 3
}

TEST_F(AttackPatternConfigTest, CreateLaserSweepCustom) {
    auto laserSweep = AttackPatternConfig::createLaserSweep(5.0F, 180.0F, 50);
    EXPECT_FLOAT_EQ(laserSweep.duration, 5.0F);
    EXPECT_FLOAT_EQ(laserSweep.spreadAngle, 180.0F);
    EXPECT_EQ(laserSweep.damage, 50);
    EXPECT_FLOAT_EQ(laserSweep.rotationSpeed, 36.0F);  // 180 / 5
}

TEST_F(AttackPatternConfigTest, CreateMinionSpawnDefault) {
    auto minionSpawn = AttackPatternConfig::createMinionSpawn();
    EXPECT_EQ(minionSpawn.pattern, BossAttackPattern::MinionSpawn);
    EXPECT_EQ(minionSpawn.minionType, "basic");
    EXPECT_EQ(minionSpawn.minionCount, 4);
    EXPECT_FLOAT_EQ(minionSpawn.duration, 1.0F);
    EXPECT_FLOAT_EQ(minionSpawn.cooldown, 8.0F);
    EXPECT_FLOAT_EQ(minionSpawn.telegraphDuration, 0.8F);
}

TEST_F(AttackPatternConfigTest, CreateMinionSpawnCustom) {
    auto minionSpawn = AttackPatternConfig::createMinionSpawn("elite", 6);
    EXPECT_EQ(minionSpawn.minionType, "elite");
    EXPECT_EQ(minionSpawn.minionCount, 6);
}

TEST_F(AttackPatternConfigTest, CreateTailSweepDefault) {
    auto tailSweep = AttackPatternConfig::createTailSweep();
    EXPECT_EQ(tailSweep.pattern, BossAttackPattern::TailSweep);
    EXPECT_FLOAT_EQ(tailSweep.duration, 2.0F);
    EXPECT_EQ(tailSweep.damage, 40);
    EXPECT_FLOAT_EQ(tailSweep.cooldown, 4.0F);
    EXPECT_FLOAT_EQ(tailSweep.spreadAngle, 180.0F);
    EXPECT_FLOAT_EQ(tailSweep.telegraphDuration, 0.5F);
}

TEST_F(AttackPatternConfigTest, CreateTailSweepCustom) {
    auto tailSweep = AttackPatternConfig::createTailSweep(3.0F, 60);
    EXPECT_FLOAT_EQ(tailSweep.duration, 3.0F);
    EXPECT_EQ(tailSweep.damage, 60);
}

// =============================================================================
// BossPatternComponent Tests
// =============================================================================

class BossPatternComponentTest : public ::testing::Test {
   protected:
    BossPatternComponent patterns;

    void SetUp() override {
        patterns.patternQueue.push_back(
            AttackPatternConfig::createCircularShot());
        patterns.patternQueue.push_back(AttackPatternConfig::createSpreadFan());
    }
};

TEST_F(BossPatternComponentTest, DefaultValues) {
    BossPatternComponent defaultPatterns;
    EXPECT_TRUE(defaultPatterns.phasePatterns.empty());
    EXPECT_TRUE(defaultPatterns.patternQueue.empty());
    EXPECT_EQ(defaultPatterns.state, PatternExecutionState::Idle);
    EXPECT_FLOAT_EQ(defaultPatterns.stateTimer, 0.0F);
    EXPECT_FLOAT_EQ(defaultPatterns.globalCooldown, 0.0F);
    EXPECT_FLOAT_EQ(defaultPatterns.patternProgress, 0.0F);
    EXPECT_FLOAT_EQ(defaultPatterns.targetX, 0.0F);
    EXPECT_FLOAT_EQ(defaultPatterns.targetY, 0.0F);
    EXPECT_TRUE(defaultPatterns.cyclical);
    EXPECT_TRUE(defaultPatterns.enabled);
    EXPECT_FLOAT_EQ(defaultPatterns.telegraphAngle, 0.0F);
    EXPECT_EQ(defaultPatterns.projectilesFired, 0);
}

TEST_F(BossPatternComponentTest, IsExecutingIdle) {
    patterns.state = PatternExecutionState::Idle;
    EXPECT_FALSE(patterns.isExecuting());
}

TEST_F(BossPatternComponentTest, IsExecutingTelegraph) {
    patterns.state = PatternExecutionState::Telegraph;
    EXPECT_TRUE(patterns.isExecuting());
}

TEST_F(BossPatternComponentTest, IsExecutingExecuting) {
    patterns.state = PatternExecutionState::Executing;
    EXPECT_TRUE(patterns.isExecuting());
}

TEST_F(BossPatternComponentTest, IsExecutingCooldown) {
    patterns.state = PatternExecutionState::Cooldown;
    EXPECT_FALSE(patterns.isExecuting());
}

TEST_F(BossPatternComponentTest, CanStartPatternWhenReady) {
    patterns.enabled = true;
    patterns.state = PatternExecutionState::Idle;
    patterns.globalCooldown = 0.0F;
    EXPECT_TRUE(patterns.canStartPattern());
}

TEST_F(BossPatternComponentTest, CannotStartPatternWhenDisabled) {
    patterns.enabled = false;
    patterns.state = PatternExecutionState::Idle;
    patterns.globalCooldown = 0.0F;
    EXPECT_FALSE(patterns.canStartPattern());
}

TEST_F(BossPatternComponentTest, CannotStartPatternWhenExecuting) {
    patterns.enabled = true;
    patterns.state = PatternExecutionState::Executing;
    patterns.globalCooldown = 0.0F;
    EXPECT_FALSE(patterns.canStartPattern());
}

TEST_F(BossPatternComponentTest, CannotStartPatternWhenOnCooldown) {
    patterns.enabled = true;
    patterns.state = PatternExecutionState::Idle;
    patterns.globalCooldown = 1.0F;
    EXPECT_FALSE(patterns.canStartPattern());
}

TEST_F(BossPatternComponentTest, CannotStartPatternWhenQueueEmpty) {
    patterns.patternQueue.clear();
    patterns.enabled = true;
    patterns.state = PatternExecutionState::Idle;
    patterns.globalCooldown = 0.0F;
    EXPECT_FALSE(patterns.canStartPattern());
}

TEST_F(BossPatternComponentTest, StartNextPatternWithTelegraph) {
    patterns.startNextPattern();

    EXPECT_EQ(patterns.state, PatternExecutionState::Telegraph);
    EXPECT_EQ(patterns.currentPattern.pattern, BossAttackPattern::CircularShot);
    EXPECT_FLOAT_EQ(patterns.patternProgress, 0.0F);
    EXPECT_EQ(patterns.projectilesFired, 0);
    // Pattern should be moved to back of queue (cyclical)
    EXPECT_EQ(patterns.patternQueue.size(), 2);
}

TEST_F(BossPatternComponentTest, StartNextPatternWithoutTelegraph) {
    // Create pattern without telegraph
    AttackPatternConfig noTelegraph;
    noTelegraph.pattern = BossAttackPattern::CircularShot;
    noTelegraph.telegraphDuration = 0.0F;
    noTelegraph.duration = 1.0F;

    patterns.patternQueue.clear();
    patterns.patternQueue.push_back(noTelegraph);

    patterns.startNextPattern();

    EXPECT_EQ(patterns.state, PatternExecutionState::Executing);
    EXPECT_FLOAT_EQ(patterns.stateTimer, 1.0F);
}

TEST_F(BossPatternComponentTest, StartNextPatternNonCyclical) {
    patterns.cyclical = false;
    std::size_t initialSize = patterns.patternQueue.size();

    patterns.startNextPattern();

    EXPECT_EQ(patterns.patternQueue.size(), initialSize - 1);
}

TEST_F(BossPatternComponentTest, StartNextPatternEmptyQueue) {
    patterns.patternQueue.clear();
    patterns.startNextPattern();  // Should not crash
    EXPECT_EQ(patterns.state, PatternExecutionState::Idle);
}

TEST_F(BossPatternComponentTest, StartExecution) {
    patterns.currentPattern = AttackPatternConfig::createCircularShot();
    patterns.state = PatternExecutionState::Telegraph;

    patterns.startExecution();

    EXPECT_EQ(patterns.state, PatternExecutionState::Executing);
    EXPECT_FLOAT_EQ(patterns.stateTimer, patterns.currentPattern.duration);
}

TEST_F(BossPatternComponentTest, CompletePattern) {
    patterns.currentPattern = AttackPatternConfig::createCircularShot();

    patterns.completePattern();

    EXPECT_EQ(patterns.state, PatternExecutionState::Cooldown);
    EXPECT_FLOAT_EQ(patterns.stateTimer, patterns.currentPattern.cooldown);
    EXPECT_FLOAT_EQ(patterns.globalCooldown,
                    patterns.currentPattern.cooldown * 0.5F);
}

TEST_F(BossPatternComponentTest, ResetToIdle) {
    patterns.state = PatternExecutionState::Cooldown;
    patterns.stateTimer = 5.0F;
    patterns.patternProgress = 0.5F;
    patterns.projectilesFired = 10;

    patterns.resetToIdle();

    EXPECT_EQ(patterns.state, PatternExecutionState::Idle);
    EXPECT_FLOAT_EQ(patterns.stateTimer, 0.0F);
    EXPECT_FLOAT_EQ(patterns.patternProgress, 0.0F);
    EXPECT_EQ(patterns.projectilesFired, 0);
}

TEST_F(BossPatternComponentTest, SetPhasePatterns) {
    std::vector<AttackPatternConfig> newPatterns = {
        AttackPatternConfig::createLaserSweep(),
        AttackPatternConfig::createMinionSpawn()};

    patterns.setPhasePatterns(newPatterns);

    EXPECT_EQ(patterns.phasePatterns.size(), 2);
    EXPECT_EQ(patterns.patternQueue.size(), 2);
    EXPECT_EQ(patterns.patternQueue.front().pattern,
              BossAttackPattern::LaserSweep);
}

TEST_F(BossPatternComponentTest, Clear) {
    patterns.state = PatternExecutionState::Executing;
    patterns.stateTimer = 5.0F;
    patterns.globalCooldown = 2.0F;
    patterns.patternProgress = 0.5F;

    patterns.clear();

    EXPECT_TRUE(patterns.phasePatterns.empty());
    EXPECT_TRUE(patterns.patternQueue.empty());
    EXPECT_EQ(patterns.state, PatternExecutionState::Idle);
    EXPECT_FLOAT_EQ(patterns.stateTimer, 0.0F);
    EXPECT_FLOAT_EQ(patterns.globalCooldown, 0.0F);
    EXPECT_FLOAT_EQ(patterns.patternProgress, 0.0F);
}

// =============================================================================
// WeaponComponent Tests
// =============================================================================

class WeaponComponentTest : public ::testing::Test {
   protected:
    WeaponComponent weapon;
};

TEST_F(WeaponComponentTest, DefaultValues) {
    EXPECT_EQ(weapon.currentSlot, 0);
    EXPECT_EQ(weapon.unlockedSlots, 1);
}

TEST_F(WeaponComponentTest, GetCurrentWeapon) {
    const WeaponConfig& current = weapon.getCurrentWeapon();
    EXPECT_EQ(current.projectileType, ProjectileType::BasicBullet);
}

TEST_F(WeaponComponentTest, NextWeaponSingleSlot) {
    weapon.unlockedSlots = 1;
    weapon.nextWeapon();
    EXPECT_EQ(weapon.currentSlot, 0);  // Should wrap around
}

TEST_F(WeaponComponentTest, NextWeaponMultipleSlots) {
    weapon.unlockedSlots = 3;
    weapon.currentSlot = 0;

    weapon.nextWeapon();
    EXPECT_EQ(weapon.currentSlot, 1);

    weapon.nextWeapon();
    EXPECT_EQ(weapon.currentSlot, 2);

    weapon.nextWeapon();
    EXPECT_EQ(weapon.currentSlot, 0);  // Wrap around
}

TEST_F(WeaponComponentTest, PreviousWeaponSingleSlot) {
    weapon.unlockedSlots = 1;
    weapon.previousWeapon();
    EXPECT_EQ(weapon.currentSlot, 0);  // Should wrap around
}

TEST_F(WeaponComponentTest, PreviousWeaponMultipleSlots) {
    weapon.unlockedSlots = 3;
    weapon.currentSlot = 2;

    weapon.previousWeapon();
    EXPECT_EQ(weapon.currentSlot, 1);

    weapon.previousWeapon();
    EXPECT_EQ(weapon.currentSlot, 0);

    weapon.previousWeapon();
    EXPECT_EQ(weapon.currentSlot, 2);  // Wrap around
}

TEST_F(WeaponComponentTest, SelectWeaponValid) {
    weapon.unlockedSlots = 4;
    weapon.selectWeapon(2);
    EXPECT_EQ(weapon.currentSlot, 2);
}

TEST_F(WeaponComponentTest, SelectWeaponInvalid) {
    weapon.unlockedSlots = 2;
    weapon.currentSlot = 0;
    weapon.selectWeapon(5);  // Invalid
    EXPECT_EQ(weapon.currentSlot, 0);  // Should not change
}

TEST_F(WeaponComponentTest, UnlockSlot) {
    weapon.unlockedSlots = 1;
    weapon.unlockSlot();
    EXPECT_EQ(weapon.unlockedSlots, 2);

    weapon.unlockSlot();
    EXPECT_EQ(weapon.unlockedSlots, 3);
}

TEST_F(WeaponComponentTest, UnlockSlotMaxLimit) {
    weapon.unlockedSlots = MAX_WEAPON_SLOTS;
    weapon.unlockSlot();
    EXPECT_EQ(weapon.unlockedSlots, MAX_WEAPON_SLOTS);  // Should not exceed
}

TEST(WeaponPresetsTest, BasicBulletPreset) {
    const auto& preset = WeaponPresets::BasicBullet;
    EXPECT_EQ(preset.projectileType, ProjectileType::BasicBullet);
    EXPECT_EQ(preset.damage, 25);
    EXPECT_FLOAT_EQ(preset.speed, 500.0F);
    EXPECT_FLOAT_EQ(preset.cooldown, 0.2F);
    EXPECT_FALSE(preset.piercing);
}

TEST(WeaponPresetsTest, ChargedShotPreset) {
    const auto& preset = WeaponPresets::ChargedShot;
    EXPECT_EQ(preset.projectileType, ProjectileType::ChargedShot);
    EXPECT_EQ(preset.damage, 100);
    EXPECT_TRUE(preset.piercing);
    EXPECT_EQ(preset.maxHits, 3);
}

TEST(WeaponPresetsTest, MissilePreset) {
    const auto& preset = WeaponPresets::Missile;
    EXPECT_EQ(preset.projectileType, ProjectileType::Missile);
    EXPECT_EQ(preset.damage, 75);
    EXPECT_FALSE(preset.piercing);
}

TEST(WeaponPresetsTest, LaserBeamPreset) {
    const auto& preset = WeaponPresets::LaserBeam;
    EXPECT_EQ(preset.projectileType, ProjectileType::LaserBeam);
    EXPECT_TRUE(preset.piercing);
    EXPECT_EQ(preset.maxHits, 10);
}

TEST(WeaponPresetsTest, SpreadShotPreset) {
    const auto& preset = WeaponPresets::SpreadShot;
    EXPECT_EQ(preset.projectileType, ProjectileType::SpreadShot);
    EXPECT_EQ(preset.projectileCount, 5);
    EXPECT_FLOAT_EQ(preset.spreadAngle, 30.0F);
}

TEST(WeaponPresetsTest, EnemyBulletPreset) {
    const auto& preset = WeaponPresets::EnemyBullet;
    EXPECT_EQ(preset.projectileType, ProjectileType::EnemyBullet);
    EXPECT_EQ(preset.damage, 15);
}

TEST(WeaponPresetsTest, HeavyBulletPreset) {
    const auto& preset = WeaponPresets::HeavyBullet;
    EXPECT_EQ(preset.projectileType, ProjectileType::HeavyBullet);
    EXPECT_EQ(preset.damage, 30);
}

TEST(WeaponPresetsTest, ContinuousLaserPreset) {
    const auto& preset = WeaponPresets::ContinuousLaser;
    EXPECT_EQ(preset.projectileType, ProjectileType::ContinuousLaser);
    EXPECT_TRUE(preset.piercing);
    EXPECT_EQ(preset.maxHits, 999);
    EXPECT_FLOAT_EQ(preset.speed, 0.0F);  // Beam doesn't move
}

// =============================================================================
// DamageOnContactComponent Tests
// =============================================================================

class DamageOnContactComponentTest : public ::testing::Test {
   protected:
    DamageOnContactComponent damage;
};

TEST_F(DamageOnContactComponentTest, DefaultValues) {
    EXPECT_EQ(damage.damage, 10);
    EXPECT_FLOAT_EQ(damage.damagePerSecond, 0.0F);
    EXPECT_FALSE(damage.isDPS);
    EXPECT_FALSE(damage.destroySelf);
    EXPECT_EQ(damage.ownerNetworkId, 0);
    EXPECT_FLOAT_EQ(damage.startupDelay, 0.0F);
    EXPECT_FLOAT_EQ(damage.activeTime, 0.0F);
}

TEST_F(DamageOnContactComponentTest, IsActiveInstantDamage) {
    damage.isDPS = false;
    EXPECT_TRUE(damage.isActive());
}

TEST_F(DamageOnContactComponentTest, IsActiveDPSNoDelay) {
    damage.isDPS = true;
    damage.startupDelay = 0.0F;
    damage.activeTime = 0.0F;
    EXPECT_TRUE(damage.isActive());
}

TEST_F(DamageOnContactComponentTest, IsActiveDPSPastDelay) {
    damage.isDPS = true;
    damage.startupDelay = 0.5F;
    damage.activeTime = 1.0F;
    EXPECT_TRUE(damage.isActive());
}

TEST_F(DamageOnContactComponentTest, IsNotActiveDPSBeforeDelay) {
    damage.isDPS = true;
    damage.startupDelay = 0.5F;
    damage.activeTime = 0.2F;
    EXPECT_FALSE(damage.isActive());
}

TEST_F(DamageOnContactComponentTest, CalculateDamageInstant) {
    damage.isDPS = false;
    damage.damage = 50;
    EXPECT_EQ(damage.calculateDamage(0.016F), 50);
}

TEST_F(DamageOnContactComponentTest, CalculateDamageDPS) {
    damage.isDPS = true;
    damage.damagePerSecond = 100.0F;
    // 100 DPS * 0.1 seconds = 10 damage
    EXPECT_EQ(damage.calculateDamage(0.1F), 10);
}

TEST_F(DamageOnContactComponentTest, CalculateDamageDPSMinimum) {
    damage.isDPS = true;
    damage.damagePerSecond = 1.0F;
    // Very small delta should still return at least 1
    EXPECT_GE(damage.calculateDamage(0.001F), 1);
}

TEST_F(DamageOnContactComponentTest, CalculateDamageDPSZero) {
    damage.isDPS = true;
    damage.damagePerSecond = 0.0F;
    // Zero DPS should return minimum 1
    EXPECT_EQ(damage.calculateDamage(0.016F), 1);
}

// =============================================================================
// Tag Components Tests
// =============================================================================

TEST(TagComponentsTest, BossTagExists) {
    BossTag tag;
    (void)tag;  // Just ensure it compiles
}

TEST(TagComponentsTest, WeakPointTagExists) {
    WeakPointTag tag;
    (void)tag;  // Just ensure it compiles
}

// =============================================================================
// MAX_WEAPON_SLOTS Constant Test
// =============================================================================

TEST(WeaponConstantsTest, MaxWeaponSlots) {
    EXPECT_EQ(MAX_WEAPON_SLOTS, 5);
}
