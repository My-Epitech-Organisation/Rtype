/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_cooldown_component - Unit tests for cooldown and charge components
*/

#include <gtest/gtest.h>

#include "games/rtype/shared/Components/CooldownComponent.hpp"

using namespace rtype::games::rtype::shared;

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

TEST_F(ShootCooldownComponentTest, ConstructorWithCooldown) {
    ShootCooldownComponent customCooldown(0.5F);
    EXPECT_FLOAT_EQ(customCooldown.cooldownTime, 0.5F);
    EXPECT_FLOAT_EQ(customCooldown.currentCooldown, 0.0F);
}

TEST_F(ShootCooldownComponentTest, CanShootWhenCooldownZero) {
    cooldown.currentCooldown = 0.0F;
    EXPECT_TRUE(cooldown.canShoot());
}

TEST_F(ShootCooldownComponentTest, CanShootWhenCooldownNegative) {
    cooldown.currentCooldown = -0.1F;
    EXPECT_TRUE(cooldown.canShoot());
}

TEST_F(ShootCooldownComponentTest, CannotShootWhenOnCooldown) {
    cooldown.currentCooldown = 0.1F;
    EXPECT_FALSE(cooldown.canShoot());
}

TEST_F(ShootCooldownComponentTest, TriggerCooldown) {
    cooldown.cooldownTime = 0.5F;
    cooldown.triggerCooldown();
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.5F);
}

TEST_F(ShootCooldownComponentTest, UpdateReducesCooldown) {
    cooldown.currentCooldown = 0.5F;
    cooldown.update(0.1F);
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.4F);
}

TEST_F(ShootCooldownComponentTest, UpdateClampsToZero) {
    cooldown.currentCooldown = 0.1F;
    cooldown.update(0.5F);  // More than remaining cooldown
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.0F);
}

TEST_F(ShootCooldownComponentTest, UpdateDoesNothingWhenCooldownZero) {
    cooldown.currentCooldown = 0.0F;
    cooldown.update(0.1F);
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.0F);
}

TEST_F(ShootCooldownComponentTest, Reset) {
    cooldown.currentCooldown = 0.5F;
    cooldown.reset();
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.0F);
}

TEST_F(ShootCooldownComponentTest, SetCooldownTime) {
    cooldown.setCooldownTime(1.0F);
    EXPECT_FLOAT_EQ(cooldown.cooldownTime, 1.0F);
}

TEST_F(ShootCooldownComponentTest, SetWeaponSlot) {
    cooldown.setWeaponSlot(3);
    EXPECT_EQ(cooldown.currentWeaponSlot, 3);
}

TEST_F(ShootCooldownComponentTest, FullCycleTest) {
    // Start with no cooldown - can shoot
    EXPECT_TRUE(cooldown.canShoot());

    // Trigger cooldown
    cooldown.triggerCooldown();
    EXPECT_FALSE(cooldown.canShoot());

    // Update partially
    cooldown.update(0.1F);
    EXPECT_FALSE(cooldown.canShoot());

    // Update to completion
    cooldown.update(0.2F);
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
    EXPECT_FALSE(charge.wasCharging);
    EXPECT_FLOAT_EQ(charge.minChargeThreshold, 0.0F);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::None);
}

TEST_F(ChargeComponentTest, ConstructorWithRate) {
    ChargeComponent customCharge(1.0F);
    EXPECT_FLOAT_EQ(customCharge.chargeRate, 1.0F);
}

TEST_F(ChargeComponentTest, StartCharging) {
    charge.startCharging();
    EXPECT_TRUE(charge.isCharging);
    EXPECT_TRUE(charge.wasCharging);
}

TEST_F(ChargeComponentTest, ReleaseReturnsLevel) {
    charge.currentLevel = ChargeLevel::Level2;
    charge.currentCharge = 0.7F;
    charge.isCharging = true;
    charge.wasCharging = true;

    ChargeLevel released = charge.release();

    EXPECT_EQ(released, ChargeLevel::Level2);
    EXPECT_FALSE(charge.isCharging);
    EXPECT_FALSE(charge.wasCharging);
    EXPECT_FLOAT_EQ(charge.currentCharge, 0.0F);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::None);
}

TEST_F(ChargeComponentTest, UpdateWhileNotCharging) {
    charge.isCharging = false;
    charge.currentCharge = 0.5F;
    charge.update(0.1F);
    EXPECT_FLOAT_EQ(charge.currentCharge, 0.5F);  // Should not change
}

TEST_F(ChargeComponentTest, UpdateIncreasesCharge) {
    charge.isCharging = true;
    charge.chargeRate = 1.0F;
    charge.currentCharge = 0.0F;

    charge.update(0.2F);

    EXPECT_FLOAT_EQ(charge.currentCharge, 0.2F);
}

TEST_F(ChargeComponentTest, UpdateClampsToMax) {
    charge.isCharging = true;
    charge.chargeRate = 2.0F;
    charge.currentCharge = 0.9F;
    charge.maxCharge = 1.0F;

    charge.update(1.0F);  // Would exceed max

    EXPECT_FLOAT_EQ(charge.currentCharge, 1.0F);
}

TEST_F(ChargeComponentTest, UpdateSetsLevel1) {
    charge.isCharging = true;
    charge.chargeRate = 1.0F;
    charge.currentCharge = 0.0F;

    charge.update(0.35F);  // Just above Level1 threshold (0.3)

    EXPECT_EQ(charge.currentLevel, ChargeLevel::Level1);
}

TEST_F(ChargeComponentTest, UpdateSetsLevel2) {
    charge.isCharging = true;
    charge.chargeRate = 1.0F;
    charge.currentCharge = 0.0F;

    charge.update(0.65F);  // Just above Level2 threshold (0.6)

    EXPECT_EQ(charge.currentLevel, ChargeLevel::Level2);
}

TEST_F(ChargeComponentTest, UpdateSetsLevel3) {
    charge.isCharging = true;
    charge.chargeRate = 1.0F;
    charge.currentCharge = 0.0F;

    charge.update(0.95F);  // Just above Level3 threshold (0.9)

    EXPECT_EQ(charge.currentLevel, ChargeLevel::Level3);
}

TEST_F(ChargeComponentTest, UpdateBelowLevel1) {
    charge.isCharging = true;
    charge.chargeRate = 1.0F;
    charge.currentCharge = 0.0F;

    charge.update(0.2F);  // Below Level1 threshold

    EXPECT_EQ(charge.currentLevel, ChargeLevel::None);
}

TEST_F(ChargeComponentTest, IsPoweredShotBelowThreshold) {
    charge.currentCharge = 0.0F;
    charge.minChargeThreshold = 0.3F;
    EXPECT_FALSE(charge.isPoweredShot());
}

TEST_F(ChargeComponentTest, IsPoweredShotAtThreshold) {
    charge.currentCharge = 0.3F;
    charge.minChargeThreshold = 0.3F;
    EXPECT_TRUE(charge.isPoweredShot());
}

TEST_F(ChargeComponentTest, IsPoweredShotAboveThreshold) {
    charge.currentCharge = 0.5F;
    charge.minChargeThreshold = 0.3F;
    EXPECT_TRUE(charge.isPoweredShot());
}

TEST_F(ChargeComponentTest, GetChargePercent) {
    charge.currentCharge = 0.5F;
    charge.maxCharge = 1.0F;
    EXPECT_FLOAT_EQ(charge.getChargePercent(), 0.5F);
}

TEST_F(ChargeComponentTest, GetChargePercentCustomMax) {
    charge.currentCharge = 0.5F;
    charge.maxCharge = 2.0F;
    EXPECT_FLOAT_EQ(charge.getChargePercent(), 0.25F);
}

TEST_F(ChargeComponentTest, GetDamageForLevelNone) {
    EXPECT_EQ(ChargeComponent::getDamageForLevel(ChargeLevel::None), 0);
}

TEST_F(ChargeComponentTest, GetDamageForLevel1) {
    EXPECT_EQ(ChargeComponent::getDamageForLevel(ChargeLevel::Level1),
              ChargeComponent::kLevel1Damage);
}

TEST_F(ChargeComponentTest, GetDamageForLevel2) {
    EXPECT_EQ(ChargeComponent::getDamageForLevel(ChargeLevel::Level2),
              ChargeComponent::kLevel2Damage);
}

TEST_F(ChargeComponentTest, GetDamageForLevel3) {
    EXPECT_EQ(ChargeComponent::getDamageForLevel(ChargeLevel::Level3),
              ChargeComponent::kLevel3Damage);
}

TEST_F(ChargeComponentTest, GetPierceCountForLevelNone) {
    EXPECT_EQ(ChargeComponent::getPierceCountForLevel(ChargeLevel::None), 0);
}

TEST_F(ChargeComponentTest, GetPierceCountForLevel1) {
    EXPECT_EQ(ChargeComponent::getPierceCountForLevel(ChargeLevel::Level1),
              ChargeComponent::kLevel1Pierce);
}

TEST_F(ChargeComponentTest, GetPierceCountForLevel2) {
    EXPECT_EQ(ChargeComponent::getPierceCountForLevel(ChargeLevel::Level2),
              ChargeComponent::kLevel2Pierce);
}

TEST_F(ChargeComponentTest, GetPierceCountForLevel3) {
    EXPECT_EQ(ChargeComponent::getPierceCountForLevel(ChargeLevel::Level3),
              ChargeComponent::kLevel3Pierce);
}

TEST_F(ChargeComponentTest, FullChargeCycle) {
    // Start charging
    charge.chargeRate = 1.0F;
    charge.startCharging();
    EXPECT_TRUE(charge.isCharging);

    // Update to Level1
    charge.update(0.35F);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::Level1);

    // Continue to Level2
    charge.update(0.30F);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::Level2);

    // Continue to Level3
    charge.update(0.35F);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::Level3);

    // Release
    ChargeLevel released = charge.release();
    EXPECT_EQ(released, ChargeLevel::Level3);
    EXPECT_EQ(charge.currentLevel, ChargeLevel::None);
    EXPECT_FALSE(charge.isCharging);
}

TEST_F(ChargeComponentTest, ChargeDoesNotExceedMaxWhileCharging) {
    charge.isCharging = true;
    charge.chargeRate = 10.0F;
    charge.maxCharge = 1.0F;
    charge.currentCharge = 0.95F;

    charge.update(1.0F);  // Would exceed max

    EXPECT_FLOAT_EQ(charge.currentCharge, 1.0F);
}

// =============================================================================
// ChargeLevel Constants Tests
// =============================================================================

TEST(ChargeLevelConstantsTest, ThresholdValues) {
    EXPECT_FLOAT_EQ(ChargeComponent::kLevel1Threshold, 0.3F);
    EXPECT_FLOAT_EQ(ChargeComponent::kLevel2Threshold, 0.6F);
    EXPECT_FLOAT_EQ(ChargeComponent::kLevel3Threshold, 0.9F);
}

TEST(ChargeLevelConstantsTest, DamageValues) {
    EXPECT_EQ(ChargeComponent::kLevel1Damage, 20);
    EXPECT_EQ(ChargeComponent::kLevel2Damage, 40);
    EXPECT_EQ(ChargeComponent::kLevel3Damage, 80);
}

TEST(ChargeLevelConstantsTest, PierceValues) {
    EXPECT_EQ(ChargeComponent::kLevel1Pierce, 1);
    EXPECT_EQ(ChargeComponent::kLevel2Pierce, 2);
    EXPECT_EQ(ChargeComponent::kLevel3Pierce, 4);
}
