/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_powerup_type_component - Tests for PowerUpTypeComponent conversion
*/

#include <gtest/gtest.h>

#include "games/rtype/shared/Components/PowerUpTypeComponent.hpp"

using namespace rtype::games::rtype::shared;

class PowerUpTypeComponentTest : public ::testing::Test {};

// =============================================================================
// stringToVariant Tests
// =============================================================================

TEST_F(PowerUpTypeComponentTest, StringToVariant_SpeedBoost) {
    auto variant = PowerUpTypeComponent::stringToVariant("speed_boost");
    EXPECT_EQ(variant, PowerUpVariant::SpeedBoost);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_Shield) {
    auto variant = PowerUpTypeComponent::stringToVariant("shield");
    EXPECT_EQ(variant, PowerUpVariant::Shield);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_RapidFire) {
    auto variant = PowerUpTypeComponent::stringToVariant("rapid_fire");
    EXPECT_EQ(variant, PowerUpVariant::RapidFire);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_DoubleDamage) {
    auto variant = PowerUpTypeComponent::stringToVariant("double_damage");
    EXPECT_EQ(variant, PowerUpVariant::DoubleDamage);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_HealthSmall) {
    auto variant = PowerUpTypeComponent::stringToVariant("health_small");
    EXPECT_EQ(variant, PowerUpVariant::HealthBoost);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_HealthLarge) {
    auto variant = PowerUpTypeComponent::stringToVariant("health_large");
    EXPECT_EQ(variant, PowerUpVariant::HealthBoost);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_WeaponUpgrade) {
    auto variant = PowerUpTypeComponent::stringToVariant("weapon_upgrade");
    EXPECT_EQ(variant, PowerUpVariant::WeaponUpgrade);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_ExtraLife) {
    auto variant = PowerUpTypeComponent::stringToVariant("extra_life");
    EXPECT_EQ(variant, PowerUpVariant::ExtraLife);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_ForcePod) {
    auto variant = PowerUpTypeComponent::stringToVariant("force_pod");
    EXPECT_EQ(variant, PowerUpVariant::ForcePod);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_Unknown) {
    auto variant = PowerUpTypeComponent::stringToVariant("unknown_powerup");
    EXPECT_EQ(variant, PowerUpVariant::Unknown);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_EmptyString) {
    auto variant = PowerUpTypeComponent::stringToVariant("");
    EXPECT_EQ(variant, PowerUpVariant::Unknown);
}

TEST_F(PowerUpTypeComponentTest, StringToVariant_CaseSensitive) {
    // Should be case-sensitive
    auto variant = PowerUpTypeComponent::stringToVariant("Speed_Boost");
    EXPECT_EQ(variant, PowerUpVariant::Unknown);

    variant = PowerUpTypeComponent::stringToVariant("SHIELD");
    EXPECT_EQ(variant, PowerUpVariant::Unknown);
}

// =============================================================================
// variantToString Tests
// =============================================================================

TEST_F(PowerUpTypeComponentTest, VariantToString_SpeedBoost) {
    auto str = PowerUpTypeComponent::variantToString(PowerUpVariant::SpeedBoost);
    EXPECT_EQ(str, "speed_boost");
}

TEST_F(PowerUpTypeComponentTest, VariantToString_Shield) {
    auto str = PowerUpTypeComponent::variantToString(PowerUpVariant::Shield);
    EXPECT_EQ(str, "shield");
}

TEST_F(PowerUpTypeComponentTest, VariantToString_RapidFire) {
    auto str = PowerUpTypeComponent::variantToString(PowerUpVariant::RapidFire);
    EXPECT_EQ(str, "rapid_fire");
}

TEST_F(PowerUpTypeComponentTest, VariantToString_DoubleDamage) {
    auto str = PowerUpTypeComponent::variantToString(PowerUpVariant::DoubleDamage);
    EXPECT_EQ(str, "double_damage");
}

TEST_F(PowerUpTypeComponentTest, VariantToString_HealthBoost) {
    auto str = PowerUpTypeComponent::variantToString(PowerUpVariant::HealthBoost);
    EXPECT_EQ(str, "health_small");
}

TEST_F(PowerUpTypeComponentTest, VariantToString_WeaponUpgrade) {
    auto str = PowerUpTypeComponent::variantToString(PowerUpVariant::WeaponUpgrade);
    EXPECT_EQ(str, "weapon_upgrade");
}

TEST_F(PowerUpTypeComponentTest, VariantToString_ExtraLife) {
    auto str = PowerUpTypeComponent::variantToString(PowerUpVariant::ExtraLife);
    EXPECT_EQ(str, "extra_life");
}

TEST_F(PowerUpTypeComponentTest, VariantToString_ForcePod) {
    auto str = PowerUpTypeComponent::variantToString(PowerUpVariant::ForcePod);
    EXPECT_EQ(str, "force_pod");
}

TEST_F(PowerUpTypeComponentTest, VariantToString_Unknown) {
    auto str = PowerUpTypeComponent::variantToString(PowerUpVariant::Unknown);
    EXPECT_EQ(str, "health_small");  // Default fallback
}

// =============================================================================
// Roundtrip Tests
// =============================================================================

TEST_F(PowerUpTypeComponentTest, RoundtripAllKnownVariants) {
    std::vector<PowerUpVariant> variants = {
        PowerUpVariant::SpeedBoost,
        PowerUpVariant::Shield,
        PowerUpVariant::RapidFire,
        PowerUpVariant::DoubleDamage,
        PowerUpVariant::WeaponUpgrade,
        PowerUpVariant::ExtraLife,
        PowerUpVariant::ForcePod,
    };

    for (const auto& variant : variants) {
        auto str = PowerUpTypeComponent::variantToString(variant);
        auto roundtrip = PowerUpTypeComponent::stringToVariant(str);
        EXPECT_EQ(roundtrip, variant) << "Roundtrip failed for variant: " << static_cast<int>(variant);
    }
}

// =============================================================================
// Component Construction Tests
// =============================================================================

TEST_F(PowerUpTypeComponentTest, DefaultConstruction) {
    PowerUpTypeComponent component;
    EXPECT_EQ(component.variant, PowerUpVariant::Unknown);
}

TEST_F(PowerUpTypeComponentTest, ExplicitConstruction) {
    PowerUpTypeComponent component{PowerUpVariant::Shield};
    EXPECT_EQ(component.variant, PowerUpVariant::Shield);
}

// =============================================================================
// Enum Value Tests
// =============================================================================

TEST_F(PowerUpTypeComponentTest, EnumValues) {
    EXPECT_EQ(static_cast<uint8_t>(PowerUpVariant::SpeedBoost), 0);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpVariant::Shield), 1);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpVariant::RapidFire), 2);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpVariant::DoubleDamage), 3);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpVariant::HealthBoost), 4);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpVariant::WeaponUpgrade), 5);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpVariant::ExtraLife), 6);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpVariant::ForcePod), 7);
    EXPECT_EQ(static_cast<uint8_t>(PowerUpVariant::Unknown), 255);
}
