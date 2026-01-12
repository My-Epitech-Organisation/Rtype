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
#include <gtest/gtest.h>

#include "games/rtype/shared/Config/PrefabLoader.hpp"
#include "games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/BoundingBoxComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "core/Registry/Registry.hpp"
#include "core/Prefab.hpp"

using namespace rtype::games::rtype::shared;
using namespace ECS;

TEST(PrefabLoaderTest, DefaultsWhenNoConfigs) {
    auto& registry = EntityConfigRegistry::getInstance();
    registry.clear();

    EXPECT_FLOAT_EQ(PrefabLoader::getPlayerSpeed("no_such_player"), 200.0f);
    EXPECT_FLOAT_EQ(PrefabLoader::getPlayerFireRate("no_such_player"), 5.0f);
    EXPECT_EQ(PrefabLoader::getEnemyScore("no_such_enemy"), 100);
    EXPECT_EQ(PrefabLoader::getProjectileDamage("no_such_projectile"), 10);
}

TEST(PrefabLoaderTest, ValuesFromLoadedConfigs) {
    auto& registry = EntityConfigRegistry::getInstance();
    registry.clear();

    // Load canonical game configs from repo test assets
    ASSERT_TRUE(registry.loadPlayersWithSearch("config/game/players.toml"));
    ASSERT_TRUE(registry.loadEnemiesWithSearch("config/game/enemies.toml"));
    ASSERT_TRUE(registry.loadProjectilesWithSearch("config/game/projectiles.toml"));

    // Known values from config files
    EXPECT_FLOAT_EQ(PrefabLoader::getPlayerSpeed("default_ship"), 250.0f);
    EXPECT_FLOAT_EQ(PrefabLoader::getPlayerFireRate("default_ship"), 8.0f);
    EXPECT_EQ(PrefabLoader::getEnemyScore("basic"), 100);
    EXPECT_EQ(PrefabLoader::getProjectileDamage("basic_bullet"), 25);
}

TEST(PrefabLoaderTest, RegisterAllPrefabsCreatesPrefabs) {
    auto& registryCfg = EntityConfigRegistry::getInstance();
    registryCfg.clear();

    // Ensure configs are loaded so prefabs will be registered
    ASSERT_TRUE(registryCfg.loadPlayersWithSearch("config/game/players.toml"));
    ASSERT_TRUE(registryCfg.loadEnemiesWithSearch("config/game/enemies.toml"));
    ASSERT_TRUE(registryCfg.loadProjectilesWithSearch("config/game/projectiles.toml"));
    ASSERT_TRUE(registryCfg.loadPowerUpsWithSearch("config/game/powerups.toml"));

    // Create an ECS registry and a PrefabManager
    Registry reg;
    PrefabManager prefabs(reg);

    // Register prefabs from configs
    PrefabLoader::registerAllPrefabs(prefabs);

    // Check presence of a few known prefabs
    EXPECT_TRUE(prefabs.hasPrefab("player_default_ship"));
    EXPECT_TRUE(prefabs.hasPrefab("enemy_basic"));
    EXPECT_TRUE(prefabs.hasPrefab("projectile_basic_bullet"));
    EXPECT_TRUE(prefabs.hasPrefab("powerup_health_small"));

    // Instantiate a player prefab and verify components
    auto playerEntity = prefabs.instantiate("player_default_ship");
    EXPECT_TRUE(reg.hasComponent<TransformComponent>(playerEntity));
    EXPECT_TRUE(reg.hasComponent<VelocityComponent>(playerEntity));
    EXPECT_TRUE(reg.hasComponent<HealthComponent>(playerEntity));
    EXPECT_TRUE(reg.hasComponent<BoundingBoxComponent>(playerEntity));
    EXPECT_TRUE(reg.hasComponent<PlayerTag>(playerEntity));

    // Instantiate an enemy prefab and verify components & velocity behavior
    auto enemyEntity = prefabs.instantiate("enemy_basic");
    EXPECT_TRUE(reg.hasComponent<TransformComponent>(enemyEntity));
    EXPECT_TRUE(reg.hasComponent<VelocityComponent>(enemyEntity));
    EXPECT_TRUE(reg.hasComponent<HealthComponent>(enemyEntity));
    EXPECT_TRUE(reg.hasComponent<EnemyTag>(enemyEntity));

    // Velocity for MoveLeft enemy should be <= 0.0f
    auto& eVel = reg.getComponent<VelocityComponent>(enemyEntity);
    EXPECT_LE(eVel.vx, 0.0f);

    // Instantiate a projectile prefab and verify components
    auto projEntity = prefabs.instantiate("projectile_basic_bullet");
    EXPECT_TRUE(reg.hasComponent<TransformComponent>(projEntity));
    EXPECT_TRUE(reg.hasComponent<VelocityComponent>(projEntity));
    EXPECT_TRUE(reg.hasComponent<BoundingBoxComponent>(projEntity));
    EXPECT_TRUE(reg.hasComponent<HealthComponent>(projEntity));
    EXPECT_TRUE(reg.hasComponent<ProjectileTag>(projEntity));

    // Instantiate a powerup prefab and verify components
    auto puEntity = prefabs.instantiate("powerup_health_small");
    EXPECT_TRUE(reg.hasComponent<TransformComponent>(puEntity));
    EXPECT_TRUE(reg.hasComponent<VelocityComponent>(puEntity));
    EXPECT_TRUE(reg.hasComponent<BoundingBoxComponent>(puEntity));
    EXPECT_TRUE(reg.hasComponent<PickupTag>(puEntity));
}
