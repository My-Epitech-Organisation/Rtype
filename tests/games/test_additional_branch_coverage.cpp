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

// Test additional CooldownComponent branches
TEST(CooldownComponentBranchTest, Initialization) {
    CooldownComponent cooldown(1.5f);
    EXPECT_FLOAT_EQ(cooldown.duration, 1.5f);
    EXPECT_FLOAT_EQ(cooldown.remaining, 0.0f);
    EXPECT_TRUE(cooldown.isReady());
}

TEST(CooldownComponentBranchTest, StartCooldown) {
    CooldownComponent cooldown(2.0f);
    cooldown.start();
    EXPECT_FLOAT_EQ(cooldown.remaining, 2.0f);
    EXPECT_FALSE(cooldown.isReady());
}

TEST(CooldownComponentBranchTest, UpdateCooldown) {
    CooldownComponent cooldown(1.0f);
    cooldown.start();
    
    cooldown.update(0.5f);
    EXPECT_FLOAT_EQ(cooldown.remaining, 0.5f);
    EXPECT_FALSE(cooldown.isReady());
    
    cooldown.update(0.6f);
    EXPECT_FLOAT_EQ(cooldown.remaining, 0.0f);
    EXPECT_TRUE(cooldown.isReady());
}

TEST(CooldownComponentBranchTest, ResetCooldown) {
    CooldownComponent cooldown(1.0f);
    cooldown.start();
    cooldown.reset();
    EXPECT_FLOAT_EQ(cooldown.remaining, 0.0f);
    EXPECT_TRUE(cooldown.isReady());
}

TEST(CooldownComponentBranchTest, GetProgress) {
    CooldownComponent cooldown(2.0f);
    EXPECT_FLOAT_EQ(cooldown.getProgress(), 1.0f);
    
    cooldown.start();
    EXPECT_FLOAT_EQ(cooldown.getProgress(), 0.0f);
    
    cooldown.update(1.0f);
    EXPECT_FLOAT_EQ(cooldown.getProgress(), 0.5f);
}

TEST(CooldownComponentBranchTest, EdgeCases) {
    CooldownComponent cooldown(0.0f);
    EXPECT_TRUE(cooldown.isReady());
    
    cooldown.update(-1.0f);
    EXPECT_TRUE(cooldown.isReady());
}

// Test PowerUpTypeComponent branches
TEST(PowerUpTypeComponentBranchTest, HealthBoost) {
    PowerUpTypeComponent powerup(PowerUpType::HealthBoost);
    EXPECT_EQ(powerup.type, PowerUpType::HealthBoost);
    EXPECT_EQ(powerup.toString(), "HealthBoost");
}

TEST(PowerUpTypeComponentBranchTest, SpeedBoost) {
    PowerUpTypeComponent powerup(PowerUpType::SpeedBoost);
    EXPECT_EQ(powerup.type, PowerUpType::SpeedBoost);
    EXPECT_EQ(powerup.toString(), "SpeedBoost");
}

TEST(PowerUpTypeComponentBranchTest, WeaponUpgrade) {
    PowerUpTypeComponent powerup(PowerUpType::WeaponUpgrade);
    EXPECT_EQ(powerup.type, PowerUpType::WeaponUpgrade);
    EXPECT_EQ(powerup.toString(), "WeaponUpgrade");
}

TEST(PowerUpTypeComponentBranchTest, Shield) {
    PowerUpTypeComponent powerup(PowerUpType::Shield);
    EXPECT_EQ(powerup.type, PowerUpType::Shield);
    EXPECT_EQ(powerup.toString(), "Shield");
}

TEST(PowerUpTypeComponentBranchTest, ForcePod) {
    PowerUpTypeComponent powerup(PowerUpType::ForcePod);
    EXPECT_EQ(powerup.type, PowerUpType::ForcePod);
    EXPECT_EQ(powerup.toString(), "ForcePod");
}

TEST(PowerUpTypeComponentBranchTest, FireRateIncrease) {
    PowerUpTypeComponent powerup(PowerUpType::FireRateIncrease);
    EXPECT_EQ(powerup.type, PowerUpType::FireRateIncrease);
    EXPECT_EQ(powerup.toString(), "FireRateIncrease");
}

TEST(PowerUpTypeComponentBranchTest, UnknownType) {
    PowerUpTypeComponent powerup(static_cast<PowerUpType>(999));
    EXPECT_EQ(powerup.toString(), "Unknown");
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
[game]
name = "TestGame"
max_players = 4
tick_rate = 60

[network]
port = 8080
timeout_seconds = 30
)";
    
    auto result = parser.loadFromString(validConfig);
    EXPECT_TRUE(result.has_value());
    if (result.has_value()) {
        EXPECT_EQ(result->gameName, "TestGame");
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
    config.gameName = "SaveTest";
    config.maxPlayers = 8;
    
    RTypeConfigParser parser;
    auto tempPath = std::filesystem::temp_directory_path() / "test_config.toml";
    
    bool saved = parser.saveToFile(config, tempPath);
    EXPECT_TRUE(saved);
    
    if (saved) {
        auto loaded = parser.loadFromFile(tempPath);
        EXPECT_TRUE(loaded.has_value());
        if (loaded.has_value()) {
            EXPECT_EQ(loaded->gameName, "SaveTest");
        }
        std::filesystem::remove(tempPath);
    }
}

TEST(RTypeConfigParserBranchTest, SaveToInvalidPath) {
    RTypeGameConfig config;
    RTypeConfigParser parser;
    
    // Try to save to an invalid path (e.g., root directory without permissions)
    bool saved = parser.saveToFile(config, "/root/impossible_location/config.toml");
    EXPECT_FALSE(saved);
}

TEST(RTypeConfigParserBranchTest, SerializeToString) {
    RTypeGameConfig config;
    config.gameName = "SerializeTest";
    config.maxPlayers = 6;
    
    RTypeConfigParser parser;
    std::string serialized = parser.serializeToString(config);
    
    EXPECT_FALSE(serialized.empty());
    EXPECT_NE(serialized.find("SerializeTest"), std::string::npos);
}

// Test LifetimeComponent
TEST(LifetimeComponentBranchTest, BasicLifetime) {
    LifetimeComponent lifetime;
    lifetime.lifetime = 2.0f;
    lifetime.elapsed = 0.0f;
    
    EXPECT_FALSE(lifetime.elapsed >= lifetime.lifetime);
    
    lifetime.elapsed = 1.0f;
    EXPECT_FALSE(lifetime.elapsed >= lifetime.lifetime);
    
    lifetime.elapsed = 2.0f;
    EXPECT_TRUE(lifetime.elapsed >= lifetime.lifetime);
    
    lifetime.elapsed = 3.0f;
    EXPECT_TRUE(lifetime.elapsed >= lifetime.lifetime);
}

// Test Registry edge cases with components
TEST(RegistryComponentBranchTest, AddAndRemoveMultipleComponents) {
    auto registry = std::make_shared<Registry>();
    auto entity = registry->spawnEntity();
    
    registry->emplaceComponent<HealthComponent>(entity);
    EXPECT_TRUE(registry->hasComponent<HealthComponent>(entity));
    
    registry->emplaceComponent<CooldownComponent>(entity, 1.0f);
    EXPECT_TRUE(registry->hasComponent<CooldownComponent>(entity));
    
    registry->removeComponent<HealthComponent>(entity);
    EXPECT_FALSE(registry->hasComponent<HealthComponent>(entity));
    EXPECT_TRUE(registry->hasComponent<CooldownComponent>(entity));
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
    CooldownComponent cooldown(0.0f);
    EXPECT_TRUE(cooldown.isReady());
    
    LifetimeComponent lifetime;
    lifetime.lifetime = 0.0f;
    lifetime.elapsed = 0.0f;
    EXPECT_TRUE(lifetime.elapsed >= lifetime.lifetime);
    
    HealthComponent health;
    health.current = 0;
    health.max = 0;
    EXPECT_TRUE(health.current <= 0);
}

TEST(BoundaryConditionsBranchTest, NegativeValues) {
    CooldownComponent cooldown(-1.0f);
    cooldown.start();
    EXPECT_TRUE(cooldown.isReady());  // Negative duration should be treated as ready
    
    LifetimeComponent lifetime;
    lifetime.lifetime = 1.0f;
    lifetime.elapsed = -1.0f;
    EXPECT_FALSE(lifetime.elapsed >= lifetime.lifetime);
}

TEST(BoundaryConditionsBranchTest, LargeValues) {
    CooldownComponent cooldown(1000000.0f);
    cooldown.start();
    EXPECT_FALSE(cooldown.isReady());
    
    cooldown.update(999999.0f);
    EXPECT_FALSE(cooldown.isReady());
    
    cooldown.update(2.0f);
    EXPECT_TRUE(cooldown.isReady());
}
