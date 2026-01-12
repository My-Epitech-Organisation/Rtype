/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Additional targeted branch coverage tests
*/

#include <gtest/gtest.h>

#include <fstream>
#include <filesystem>
#include <memory>

#include "core/Registry/Registry.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"
#include "games/rtype/shared/Components/LifetimeComponent.hpp"
#include "games/rtype/shared/Components/PowerUpTypeComponent.hpp"
#include "games/rtype/shared/Config/Parser/RTypeConfigParser.hpp"
#include "games/rtype/shared/Config/GameConfig/RTypeGameConfig.hpp"

using namespace ECS;
using namespace rtype::games::rtype::shared;
using namespace rtype::game::config;

// Test ShootCooldownComponent branches
TEST(ShootCooldownComponentBranchTest, Initialization) {
    ShootCooldownComponent cooldown(1.5f);
    EXPECT_FLOAT_EQ(cooldown.cooldownTime, 1.5f);
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.0f);
    EXPECT_TRUE(cooldown.canShoot());
}

TEST(ShootCooldownComponentBranchTest, TriggerCooldown) {
    ShootCooldownComponent cooldown(2.0f);
    cooldown.triggerCooldown();
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 2.0f);
    EXPECT_FALSE(cooldown.canShoot());
}

TEST(ShootCooldownComponentBranchTest, UpdateCooldown) {
    ShootCooldownComponent cooldown(1.0f);
    cooldown.triggerCooldown();
    
    cooldown.update(0.5f);
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.5f);
    EXPECT_FALSE(cooldown.canShoot());
    
    cooldown.update(0.6f);
    EXPECT_LE(cooldown.currentCooldown, 0.0f);
    EXPECT_TRUE(cooldown.canShoot());
}

TEST(ShootCooldownComponentBranchTest, Reset) {
    ShootCooldownComponent cooldown(1.0f);
    cooldown.triggerCooldown();
    cooldown.reset();
    EXPECT_FLOAT_EQ(cooldown.currentCooldown, 0.0f);
    EXPECT_TRUE(cooldown.canShoot());
}

TEST(ShootCooldownComponentBranchTest, EdgeCases) {
    ShootCooldownComponent cooldown(0.0f);
    EXPECT_TRUE(cooldown.canShoot());
    
    cooldown.update(-1.0f);
    EXPECT_TRUE(cooldown.canShoot());
}

// Test PowerUpVariant branches
TEST(PowerUpTypeComponentBranchTest, SpeedBoost) {
    PowerUpTypeComponent powerup;
    powerup.variant = PowerUpVariant::SpeedBoost;
    EXPECT_EQ(powerup.variant, PowerUpVariant::SpeedBoost);
    EXPECT_EQ(PowerUpTypeComponent::variantToString(powerup.variant), "speed_boost");
}

TEST(PowerUpTypeComponentBranchTest, Shield) {
    PowerUpTypeComponent powerup;
    powerup.variant = PowerUpVariant::Shield;
    EXPECT_EQ(powerup.variant, PowerUpVariant::Shield);
    EXPECT_EQ(PowerUpTypeComponent::variantToString(powerup.variant), "shield");
}

TEST(PowerUpTypeComponentBranchTest, RapidFire) {
    PowerUpTypeComponent powerup;
    powerup.variant = PowerUpVariant::RapidFire;
    EXPECT_EQ(powerup.variant, PowerUpVariant::RapidFire);
    EXPECT_EQ(PowerUpTypeComponent::variantToString(powerup.variant), "rapid_fire");
}

TEST(PowerUpTypeComponentBranchTest, DoubleDamage) {
    PowerUpTypeComponent powerup;
    powerup.variant = PowerUpVariant::DoubleDamage;
    EXPECT_EQ(powerup.variant, PowerUpVariant::DoubleDamage);
    EXPECT_EQ(PowerUpTypeComponent::variantToString(powerup.variant), "double_damage");
}

TEST(PowerUpTypeComponentBranchTest, HealthBoost) {
    PowerUpTypeComponent powerup;
    powerup.variant = PowerUpVariant::HealthBoost;
    EXPECT_EQ(powerup.variant, PowerUpVariant::HealthBoost);
    EXPECT_EQ(PowerUpTypeComponent::variantToString(powerup.variant), "health_small");
}

TEST(PowerUpTypeComponentBranchTest, WeaponUpgrade) {
    PowerUpTypeComponent powerup;
    powerup.variant = PowerUpVariant::WeaponUpgrade;
    EXPECT_EQ(powerup.variant, PowerUpVariant::WeaponUpgrade);
    EXPECT_EQ(PowerUpTypeComponent::variantToString(powerup.variant), "weapon_upgrade");
}

TEST(PowerUpTypeComponentBranchTest, ExtraLife) {
    PowerUpTypeComponent powerup;
    powerup.variant = PowerUpVariant::ExtraLife;
    EXPECT_EQ(powerup.variant, PowerUpVariant::ExtraLife);
    EXPECT_EQ(PowerUpTypeComponent::variantToString(powerup.variant), "extra_life");
}

TEST(PowerUpTypeComponentBranchTest, ForcePod) {
    PowerUpTypeComponent powerup;
    powerup.variant = PowerUpVariant::ForcePod;
    EXPECT_EQ(powerup.variant, PowerUpVariant::ForcePod);
    EXPECT_EQ(PowerUpTypeComponent::variantToString(powerup.variant), "force_pod");
}

TEST(PowerUpTypeComponentBranchTest, StringToVariant) {
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("speed_boost"), PowerUpVariant::SpeedBoost);
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("shield"), PowerUpVariant::Shield);
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("rapid_fire"), PowerUpVariant::RapidFire);
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("double_damage"), PowerUpVariant::DoubleDamage);
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("health_small"), PowerUpVariant::HealthBoost);
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("health_large"), PowerUpVariant::HealthBoost);
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("weapon_upgrade"), PowerUpVariant::WeaponUpgrade);
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("extra_life"), PowerUpVariant::ExtraLife);
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("force_pod"), PowerUpVariant::ForcePod);
    EXPECT_EQ(PowerUpTypeComponent::stringToVariant("unknown"), PowerUpVariant::Unknown);
}

// Test RTypeConfigParser branches
TEST(RTypeConfigParserBranchTest, LoadNonExistentFile) {
    RTypeConfigParser parser;
    auto result = parser.loadFromFile("/nonexistent/path/to/config.toml");
    EXPECT_FALSE(result.has_value());
}

TEST(RTypeConfigParserBranchTest, LoadFromString) {
    RTypeConfigParser parser;
    std::string validConfig = R"(
[video]
width = 1280
height = 720

[network]
server_address = "127.0.0.1"
server_port = 8080
)";
    
    auto result = parser.loadFromString(validConfig);
    EXPECT_TRUE(result.has_value());
    if (result.has_value()) {
        EXPECT_EQ(result->video.width, 1280);
    }
}

TEST(RTypeConfigParserBranchTest, LoadFromStringInvalid) {
    RTypeConfigParser parser;
    std::string invalidConfig = "this is not valid toml { [ }";
    
    auto result = parser.loadFromString(invalidConfig);
    EXPECT_FALSE(result.has_value());
}

TEST(RTypeConfigParserBranchTest, SaveToFile) {
    RTypeGameConfig config;
    config.video.width = 1920;
    config.video.height = 1080;
    
    RTypeConfigParser parser;
    auto tempPath = std::filesystem::temp_directory_path() / "test_config.toml";
    
    bool saved = parser.saveToFile(config, tempPath);
    EXPECT_TRUE(saved);
    
    if (saved) {
        auto loaded = parser.loadFromFile(tempPath);
        EXPECT_TRUE(loaded.has_value());
        if (loaded.has_value()) {
            EXPECT_EQ(loaded->video.width, 1920);
        }
        std::filesystem::remove(tempPath);
    }
}

TEST(RTypeConfigParserBranchTest, SerializeToString) {
    RTypeGameConfig config;
    config.video.width = 1920;
    config.network.serverPort = 4242;
    
    RTypeConfigParser parser;
    std::string serialized = parser.serializeToString(config);
    
    EXPECT_FALSE(serialized.empty());
    EXPECT_NE(serialized.find("1920"), std::string::npos);
}

// Test LifetimeComponent
TEST(LifetimeComponentBranchTest, BasicLifetime) {
    LifetimeComponent lifetime(2.0f);
    
    EXPECT_FLOAT_EQ(lifetime.remainingTime, 2.0f);
    EXPECT_GT(lifetime.remainingTime, 0.0f);
    
    lifetime.remainingTime -= 1.0f;
    EXPECT_FLOAT_EQ(lifetime.remainingTime, 1.0f);
    
    lifetime.remainingTime -= 2.0f;
    EXPECT_LT(lifetime.remainingTime, 0.0f);
}

TEST(LifetimeComponentBranchTest, DefaultLifetime) {
    LifetimeComponent lifetime;
    EXPECT_FLOAT_EQ(lifetime.remainingTime, 5.0f);
}

// Test Registry edge cases with components
TEST(RegistryComponentBranchTest, AddAndRemoveMultipleComponents) {
    auto registry = std::make_shared<Registry>();
    auto entity = registry->spawnEntity();
    
    registry->emplaceComponent<HealthComponent>(entity);
    EXPECT_TRUE(registry->hasComponent<HealthComponent>(entity));
    
    registry->emplaceComponent<LifetimeComponent>(entity, 2.0f);
    EXPECT_TRUE(registry->hasComponent<LifetimeComponent>(entity));
    
    registry->removeComponent<HealthComponent>(entity);
    EXPECT_FALSE(registry->hasComponent<HealthComponent>(entity));
    EXPECT_TRUE(registry->hasComponent<LifetimeComponent>(entity));
}

TEST(RegistryComponentBranchTest, GetComponentVariations) {
    auto registry = std::make_shared<Registry>();
    auto entity = registry->spawnEntity();
    
    auto& health = registry->emplaceComponent<HealthComponent>(entity);
    health.current = 50;
    health.max = 100;
    
    const auto& retrieved = registry->getComponent<HealthComponent>(entity);
    EXPECT_EQ(retrieved.current, 50);
    EXPECT_EQ(retrieved.max, 100);
    
    // Modify through reference
    auto& mutableHealth = registry->getComponent<HealthComponent>(entity);
    mutableHealth.current = 75;
    
    EXPECT_EQ(registry->getComponent<HealthComponent>(entity).current, 75);
}

// Test boundary conditions in various components
TEST(BoundaryConditionsBranchTest, ZeroValues) {
    ShootCooldownComponent cooldown(0.0f);
    EXPECT_TRUE(cooldown.canShoot());
    
    LifetimeComponent lifetime(0.0f);
    EXPECT_LE(lifetime.remainingTime, 0.0f);
    
    HealthComponent health;
    health.current = 0;
    health.max = 0;
    EXPECT_TRUE(health.current <= 0);
}

TEST(BoundaryConditionsBranchTest, NegativeValues) {
    ShootCooldownComponent cooldown(1.0f);
    cooldown.currentCooldown = -1.0f;
    EXPECT_TRUE(cooldown.canShoot());
    
    LifetimeComponent lifetime(-1.0f);
    EXPECT_LT(lifetime.remainingTime, 0.0f);
}

TEST(BoundaryConditionsBranchTest, LargeValues) {
    ShootCooldownComponent cooldown(1000000.0f);
    cooldown.triggerCooldown();
    EXPECT_FALSE(cooldown.canShoot());
    
    cooldown.update(999999.0f);
    EXPECT_FALSE(cooldown.canShoot());
    
    cooldown.update(2.0f);
    EXPECT_TRUE(cooldown.canShoot());
}

// Test VideoConfig
TEST(VideoConfigBranchTest, DefaultValues) {
    VideoConfig config;
    EXPECT_EQ(config.width, 1280);
    EXPECT_EQ(config.height, 720);
    EXPECT_FALSE(config.fullscreen);
    EXPECT_TRUE(config.vsync);
}

TEST(VideoConfigBranchTest, CustomValues) {
    VideoConfig config;
    config.width = 1920;
    config.height = 1080;
    config.fullscreen = true;
    
    EXPECT_EQ(config.width, 1920);
    EXPECT_EQ(config.height, 1080);
    EXPECT_TRUE(config.fullscreen);
}

// Test AudioConfig
TEST(AudioConfigBranchTest, DefaultValues) {
    AudioConfig config;
    EXPECT_FLOAT_EQ(config.masterVolume, 1.0f);
    EXPECT_FLOAT_EQ(config.musicVolume, 0.8f);
    EXPECT_FALSE(config.muted);
}

TEST(AudioConfigBranchTest, VolumeAdjustments) {
    AudioConfig config;
    config.masterVolume = 0.5f;
    config.sfxVolume = 0.7f;
    config.muted = true;
    
    EXPECT_FLOAT_EQ(config.masterVolume, 0.5f);
    EXPECT_FLOAT_EQ(config.sfxVolume, 0.7f);
    EXPECT_TRUE(config.muted);
}

// Test NetworkConfig
TEST(NetworkConfigBranchTest, DefaultValues) {
    NetworkConfig config;
    EXPECT_EQ(config.serverAddress, "127.0.0.1");
    EXPECT_EQ(config.serverPort, 4000);
    EXPECT_EQ(config.clientPort, 0);
}

TEST(NetworkConfigBranchTest, CustomNetwork) {
    NetworkConfig config;
    config.serverAddress = "192.168.1.100";
    config.serverPort = 8080;
    config.tickrate = 120;
    
    EXPECT_EQ(config.serverAddress, "192.168.1.100");
    EXPECT_EQ(config.serverPort, 8080);
    EXPECT_EQ(config.tickrate, 120);
}
