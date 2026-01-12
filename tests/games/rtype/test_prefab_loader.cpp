/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_prefab_loader - Tests for PrefabLoader configuration bridge
*/

#include <gtest/gtest.h>

#include <memory>

#include "games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"
#include "games/rtype/shared/Config/PrefabLoader.hpp"
#include "core/Prefab.hpp"
#include "core/Registry/Registry.hpp"

using namespace rtype::games::rtype::shared;

class PrefabLoaderTest : public ::testing::Test {
   protected:
    std::unique_ptr<ECS::Registry> registry;

    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        // Clear any existing configs
        EntityConfigRegistry::getInstance().clear();
    }

    void TearDown() override {
        EntityConfigRegistry::getInstance().clear();
        registry.reset();
    }
};

// =============================================================================
// registerAllPrefabs Tests (with empty config)
// =============================================================================

TEST_F(PrefabLoaderTest, RegisterAllPrefabsWithEmptyConfig) {
    ECS::PrefabManager prefabs(*registry);

    // Should not crash with empty config
    EXPECT_NO_THROW(PrefabLoader::registerAllPrefabs(prefabs));
}

TEST_F(PrefabLoaderTest, RegisterEnemyPrefabsEmpty) {
    ECS::PrefabManager prefabs(*registry);

    EXPECT_NO_THROW(PrefabLoader::registerEnemyPrefabs(prefabs));
}

TEST_F(PrefabLoaderTest, RegisterProjectilePrefabsEmpty) {
    ECS::PrefabManager prefabs(*registry);

    EXPECT_NO_THROW(PrefabLoader::registerProjectilePrefabs(prefabs));
}

TEST_F(PrefabLoaderTest, RegisterPlayerPrefabsEmpty) {
    ECS::PrefabManager prefabs(*registry);

    EXPECT_NO_THROW(PrefabLoader::registerPlayerPrefabs(prefabs));
}

TEST_F(PrefabLoaderTest, RegisterPowerUpPrefabsEmpty) {
    ECS::PrefabManager prefabs(*registry);

    EXPECT_NO_THROW(PrefabLoader::registerPowerUpPrefabs(prefabs));
}

// =============================================================================
// Helper Function Tests (with empty config - default values)
// =============================================================================

TEST_F(PrefabLoaderTest, GetPlayerSpeedNotFound) {
    // Should return default value when player not found
    float speed = PrefabLoader::getPlayerSpeed("nonexistent");
    EXPECT_FLOAT_EQ(speed, 200.0f);  // Default value
}

TEST_F(PrefabLoaderTest, GetPlayerFireRateNotFound) {
    float fireRate = PrefabLoader::getPlayerFireRate("nonexistent");
    EXPECT_FLOAT_EQ(fireRate, 5.0f);  // Default value
}

TEST_F(PrefabLoaderTest, GetEnemyScoreNotFound) {
    int32_t score = PrefabLoader::getEnemyScore("nonexistent");
    EXPECT_EQ(score, 100);  // Default value
}

TEST_F(PrefabLoaderTest, GetProjectileDamageNotFound) {
    int32_t damage = PrefabLoader::getProjectileDamage("nonexistent");
    EXPECT_EQ(damage, 10);  // Default value
}

// =============================================================================
// Helper Function Tests (with empty string)
// =============================================================================

TEST_F(PrefabLoaderTest, GetPlayerSpeedEmptyId) {
    float speed = PrefabLoader::getPlayerSpeed("");
    EXPECT_FLOAT_EQ(speed, 200.0f);  // Default value
}

TEST_F(PrefabLoaderTest, GetPlayerFireRateEmptyId) {
    float fireRate = PrefabLoader::getPlayerFireRate("");
    EXPECT_FLOAT_EQ(fireRate, 5.0f);  // Default value
}

TEST_F(PrefabLoaderTest, GetEnemyScoreEmptyId) {
    int32_t score = PrefabLoader::getEnemyScore("");
    EXPECT_EQ(score, 100);  // Default value
}

TEST_F(PrefabLoaderTest, GetProjectileDamageEmptyId) {
    int32_t damage = PrefabLoader::getProjectileDamage("");
    EXPECT_EQ(damage, 10);  // Default value
}

// =============================================================================
// Multiple calls to register (idempotency)
// =============================================================================

TEST_F(PrefabLoaderTest, RegisterAllPrefabsMultipleTimes) {
    ECS::PrefabManager prefabs(*registry);

    EXPECT_NO_THROW(PrefabLoader::registerAllPrefabs(prefabs));
    EXPECT_NO_THROW(PrefabLoader::registerAllPrefabs(prefabs));
}

TEST_F(PrefabLoaderTest, RegisterEachCategoryMultipleTimes) {
    ECS::PrefabManager prefabs(*registry);

    EXPECT_NO_THROW(PrefabLoader::registerEnemyPrefabs(prefabs));
    EXPECT_NO_THROW(PrefabLoader::registerEnemyPrefabs(prefabs));
    EXPECT_NO_THROW(PrefabLoader::registerProjectilePrefabs(prefabs));
    EXPECT_NO_THROW(PrefabLoader::registerProjectilePrefabs(prefabs));
    EXPECT_NO_THROW(PrefabLoader::registerPlayerPrefabs(prefabs));
    EXPECT_NO_THROW(PrefabLoader::registerPlayerPrefabs(prefabs));
    EXPECT_NO_THROW(PrefabLoader::registerPowerUpPrefabs(prefabs));
    EXPECT_NO_THROW(PrefabLoader::registerPowerUpPrefabs(prefabs));
}
