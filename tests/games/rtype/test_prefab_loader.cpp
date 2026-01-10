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
