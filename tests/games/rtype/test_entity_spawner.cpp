/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** RTypeEntitySpawner - Comprehensive tests for entity spawning
*/

#include <gtest/gtest.h>

#include <memory>

#include "games/rtype/server/RTypeEntitySpawner.hpp"
#include "games/rtype/shared/Components/BoundingBoxComponent.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/PlayerIdComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "games/rtype/shared/Components/WeaponComponent.hpp"
#include "server/network/ServerNetworkSystem.hpp"

using namespace rtype::games::rtype::server;
using namespace rtype::games::rtype::shared;
using namespace rtype::server;

// Mock ServerNetworkSystem for testing
class MockServerNetworkSystem : public ServerNetworkSystem {
   public:
    MockServerNetworkSystem(std::shared_ptr<ECS::Registry> registry,
                            std::shared_ptr<rtype::server::NetworkServer> server)
        : ServerNetworkSystem(registry, server) {}

    std::optional<ECS::Entity> lastRegisteredEntity;
    std::uint32_t lastRegisteredNetworkId = 0;
    bool unregisterCalled = false;
};

class RTypeEntitySpawnerTest : public ::testing::Test {
   protected:
    std::shared_ptr<ECS::Registry> registry;
    std::shared_ptr<MockServerNetworkSystem> networkSystem;
    std::unique_ptr<RTypeEntitySpawner> spawner;

    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();
        // Create a mock network server (nullptr is acceptable for these tests)
        networkSystem = std::make_shared<MockServerNetworkSystem>(registry, nullptr);
        
        // Create spawner without game engine and config for basic tests
        spawner = std::make_unique<RTypeEntitySpawner>(
            registry, networkSystem, std::nullopt, std::nullopt);
    }

    void TearDown() override {
        spawner.reset();
        networkSystem.reset();
        registry.reset();
    }
};

// ============================================================================
// Player Spawning Tests
// ============================================================================

TEST_F(RTypeEntitySpawnerTest, SpawnPlayerSuccess) {
    PlayerSpawnConfig config{};
    config.userId = 1001;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);

    EXPECT_TRUE(result.success);
    EXPECT_FALSE(result.entity.isNull());
    EXPECT_EQ(result.networkId, 1001U);
    EXPECT_GT(result.health, 0);
    EXPECT_GT(result.maxHealth, 0);
}

TEST_F(RTypeEntitySpawnerTest, SpawnPlayerHasRequiredComponents) {
    PlayerSpawnConfig config{};
    config.userId = 1002;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    // Check all required components are present
    EXPECT_TRUE(registry->hasComponent<Position>(result.entity));
    EXPECT_TRUE(registry->hasComponent<TransformComponent>(result.entity));
    EXPECT_TRUE(registry->hasComponent<VelocityComponent>(result.entity));
    EXPECT_TRUE(registry->hasComponent<ShootCooldownComponent>(result.entity));
    EXPECT_TRUE(registry->hasComponent<WeaponComponent>(result.entity));
    EXPECT_TRUE(registry->hasComponent<BoundingBoxComponent>(result.entity));
    EXPECT_TRUE(registry->hasComponent<PlayerTag>(result.entity));
    EXPECT_TRUE(registry->hasComponent<HealthComponent>(result.entity));
    EXPECT_TRUE(registry->hasComponent<NetworkIdComponent>(result.entity));
    EXPECT_TRUE(registry->hasComponent<PlayerIdComponent>(result.entity));
}

TEST_F(RTypeEntitySpawnerTest, SpawnPlayerCorrectPosition) {
    PlayerSpawnConfig config{};
    config.userId = 1003;
    config.playerIndex = 2;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    const auto& pos = registry->getComponent<Position>(result.entity);
    EXPECT_EQ(pos.x, 100.0F);  // kSpawnBaseX
    EXPECT_EQ(pos.y, 150.0F + 2.0F * 100.0F);  // kSpawnBaseY + playerIndex * kSpawnYOffset
}

TEST_F(RTypeEntitySpawnerTest, SpawnPlayerCorrectHealth) {
    PlayerSpawnConfig config{};
    config.userId = 1004;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    const auto& health = registry->getComponent<HealthComponent>(result.entity);
    EXPECT_EQ(health.current, 100);  // kDefaultPlayerHealth
    EXPECT_EQ(health.max, 100);
}

TEST_F(RTypeEntitySpawnerTest, SpawnPlayerCorrectNetworkId) {
    PlayerSpawnConfig config{};
    config.userId = 1005;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    const auto& netId = registry->getComponent<NetworkIdComponent>(result.entity);
    EXPECT_EQ(netId.networkId, 1005U);
}

TEST_F(RTypeEntitySpawnerTest, SpawnPlayerCorrectPlayerId) {
    PlayerSpawnConfig config{};
    config.userId = 1006;
    config.playerIndex = 3;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    const auto& playerId = registry->getComponent<PlayerIdComponent>(result.entity);
    EXPECT_EQ(playerId.playerId, 4U);  // playerIndex + 1
}

TEST_F(RTypeEntitySpawnerTest, SpawnPlayerWithNullRegistry) {
    auto spawnerWithNullRegistry = std::make_unique<RTypeEntitySpawner>(
        nullptr, networkSystem, std::nullopt, std::nullopt);

    PlayerSpawnConfig config{};
    config.userId = 1007;
    config.playerIndex = 0;

    auto result = spawnerWithNullRegistry->spawnPlayer(config);
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(result.entity.isNull());
}

TEST_F(RTypeEntitySpawnerTest, SpawnMultiplePlayers) {
    for (int i = 0; i < 4; ++i) {
        PlayerSpawnConfig config{};
        config.userId = 2000 + i;
        config.playerIndex = i;

        auto result = spawner->spawnPlayer(config);
        EXPECT_TRUE(result.success);
        EXPECT_FALSE(result.entity.isNull());
    }

    // Verify all players exist
    auto view = registry->view<PlayerTag>();
    int count = 0;
    view.each([&count](ECS::Entity, const PlayerTag&) { ++count; });
    EXPECT_EQ(count, 4);
}

// ============================================================================
// Player Destruction Tests
// ============================================================================

TEST_F(RTypeEntitySpawnerTest, DestroyPlayerSuccess) {
    PlayerSpawnConfig config{};
    config.userId = 3001;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    spawner->destroyPlayer(result.entity);
    EXPECT_FALSE(registry->isAlive(result.entity));
}

TEST_F(RTypeEntitySpawnerTest, DestroyPlayerByUserIdSuccess) {
    PlayerSpawnConfig config{};
    config.userId = 3002;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    bool destroyed = spawner->destroyPlayerByUserId(3002);
    EXPECT_TRUE(destroyed);
}

TEST_F(RTypeEntitySpawnerTest, DestroyPlayerByUserIdNotFound) {
    bool destroyed = spawner->destroyPlayerByUserId(9999);
    EXPECT_FALSE(destroyed);
}

TEST_F(RTypeEntitySpawnerTest, DestroyPlayerWithNullRegistry) {
    auto spawnerWithNullRegistry = std::make_unique<RTypeEntitySpawner>(
        nullptr, networkSystem, std::nullopt, std::nullopt);

    ECS::Entity dummyEntity = registry->spawnEntity();
    spawnerWithNullRegistry->destroyPlayer(dummyEntity);
    // Should not crash
    EXPECT_TRUE(registry->isAlive(dummyEntity));
}

// ============================================================================
// Shooting Tests
// ============================================================================

TEST_F(RTypeEntitySpawnerTest, CanPlayerShootWithCooldownReady) {
    PlayerSpawnConfig config{};
    config.userId = 4001;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    // New players should be able to shoot
    EXPECT_TRUE(spawner->canPlayerShoot(result.entity));
}

TEST_F(RTypeEntitySpawnerTest, CanPlayerShootAfterTriggeringCooldown) {
    PlayerSpawnConfig config{};
    config.userId = 4002;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    spawner->triggerShootCooldown(result.entity);
    // After triggering, should not be able to shoot immediately
    EXPECT_FALSE(spawner->canPlayerShoot(result.entity));
}

TEST_F(RTypeEntitySpawnerTest, CanPlayerShootWithoutCooldownComponent) {
    ECS::Entity entity = registry->spawnEntity();
    EXPECT_FALSE(spawner->canPlayerShoot(entity));
}

TEST_F(RTypeEntitySpawnerTest, CanPlayerShootWithNullRegistry) {
    auto spawnerWithNullRegistry = std::make_unique<RTypeEntitySpawner>(
        nullptr, networkSystem, std::nullopt, std::nullopt);

    ECS::Entity entity = registry->spawnEntity();
    EXPECT_FALSE(spawnerWithNullRegistry->canPlayerShoot(entity));
}

TEST_F(RTypeEntitySpawnerTest, TriggerShootCooldownSuccess) {
    PlayerSpawnConfig config{};
    config.userId = 4003;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    spawner->triggerShootCooldown(result.entity);
    
    auto& cooldown = registry->getComponent<ShootCooldownComponent>(result.entity);
    EXPECT_GT(cooldown.currentCooldown, 0.0F);
}

TEST_F(RTypeEntitySpawnerTest, HandlePlayerShootWithoutGameEngine) {
    PlayerSpawnConfig config{};
    config.userId = 4004;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    std::uint32_t projectileId = spawner->handlePlayerShoot(result.entity, result.networkId);
    EXPECT_EQ(projectileId, 0U);  // Should return 0 without game engine
}

// ============================================================================
// Entity Query Tests
// ============================================================================

TEST_F(RTypeEntitySpawnerTest, GetPlayerEntitySuccess) {
    PlayerSpawnConfig config{};
    config.userId = 5001;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    auto entityOpt = spawner->getPlayerEntity(5001);
    EXPECT_TRUE(entityOpt.has_value());
    EXPECT_EQ(*entityOpt, result.entity);
}

TEST_F(RTypeEntitySpawnerTest, GetPlayerEntityNotFound) {
    auto entityOpt = spawner->getPlayerEntity(9999);
    EXPECT_FALSE(entityOpt.has_value());
}

TEST_F(RTypeEntitySpawnerTest, GetEntityNetworkIdSuccess) {
    PlayerSpawnConfig config{};
    config.userId = 5002;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    auto networkIdOpt = spawner->getEntityNetworkId(result.entity);
    EXPECT_TRUE(networkIdOpt.has_value());
    EXPECT_EQ(*networkIdOpt, 5002U);
}

TEST_F(RTypeEntitySpawnerTest, GetEntityNetworkIdWithoutComponent) {
    ECS::Entity entity = registry->spawnEntity();
    auto networkIdOpt = spawner->getEntityNetworkId(entity);
    EXPECT_FALSE(networkIdOpt.has_value());
}

TEST_F(RTypeEntitySpawnerTest, GetEntityNetworkIdWithNullRegistry) {
    auto spawnerWithNullRegistry = std::make_unique<RTypeEntitySpawner>(
        nullptr, networkSystem, std::nullopt, std::nullopt);

    ECS::Entity entity = registry->spawnEntity();
    auto networkIdOpt = spawnerWithNullRegistry->getEntityNetworkId(entity);
    EXPECT_FALSE(networkIdOpt.has_value());
}

TEST_F(RTypeEntitySpawnerTest, GetEntityPositionSuccess) {
    PlayerSpawnConfig config{};
    config.userId = 5003;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    auto posOpt = spawner->getEntityPosition(result.entity);
    EXPECT_TRUE(posOpt.has_value());
    EXPECT_EQ(posOpt->x, 100.0F);
    EXPECT_EQ(posOpt->y, 150.0F);
}

TEST_F(RTypeEntitySpawnerTest, GetEntityPositionWithoutComponent) {
    ECS::Entity entity = registry->spawnEntity();
    auto posOpt = spawner->getEntityPosition(entity);
    EXPECT_FALSE(posOpt.has_value());
}

TEST_F(RTypeEntitySpawnerTest, GetEntityPositionWithNullRegistry) {
    auto spawnerWithNullRegistry = std::make_unique<RTypeEntitySpawner>(
        nullptr, networkSystem, std::nullopt, std::nullopt);

    ECS::Entity entity = registry->spawnEntity();
    auto posOpt = spawnerWithNullRegistry->getEntityPosition(entity);
    EXPECT_FALSE(posOpt.has_value());
}

// ============================================================================
// Player Movement Tests
// ============================================================================

TEST_F(RTypeEntitySpawnerTest, UpdatePlayerVelocitySuccess) {
    PlayerSpawnConfig config{};
    config.userId = 6001;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    spawner->updatePlayerVelocity(result.entity, 100.0F, 50.0F);

    const auto& vel = registry->getComponent<VelocityComponent>(result.entity);
    EXPECT_EQ(vel.vx, 100.0F);
    EXPECT_EQ(vel.vy, 50.0F);
}

TEST_F(RTypeEntitySpawnerTest, UpdatePlayerVelocityWithoutComponent) {
    ECS::Entity entity = registry->spawnEntity();
    spawner->updatePlayerVelocity(entity, 100.0F, 50.0F);
    // Should not crash
    EXPECT_TRUE(registry->isAlive(entity));
}

TEST_F(RTypeEntitySpawnerTest, UpdateAllPlayersMovement) {
    // Spawn multiple players
    for (int i = 0; i < 3; ++i) {
        PlayerSpawnConfig config{};
        config.userId = 6100 + i;
        config.playerIndex = i;
        auto result = spawner->spawnPlayer(config);
        (void)result;  // Suppress unused warning
    }

    int callbackCount = 0;
    spawner->updateAllPlayersMovement(0.016F, [&callbackCount](std::uint32_t, float, float, float, float) {
        ++callbackCount;
    });

    EXPECT_EQ(callbackCount, 3);  // Called for each player
}

// ============================================================================
// Configuration Tests
// ============================================================================

TEST_F(RTypeEntitySpawnerTest, GetPlayerSpeedDefaultValue) {
    float speed = spawner->getPlayerSpeed();
    EXPECT_EQ(speed, 250.0F);  // kDefaultPlayerSpeed
}

TEST_F(RTypeEntitySpawnerTest, GetWorldBounds) {
    auto bounds = spawner->getWorldBounds();
    EXPECT_EQ(bounds.minX, 0.0F);
    EXPECT_EQ(bounds.maxX, 1920.0F - 64.0F);
    EXPECT_EQ(bounds.minY, 0.0F);
    EXPECT_EQ(bounds.maxY, 1080.0F - 64.0F);
}

TEST_F(RTypeEntitySpawnerTest, GetGameId) {
    std::string gameId = spawner->getGameId();
    EXPECT_EQ(gameId, "rtype");
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(RTypeEntitySpawnerTest, SpawnPlayerWithHighPlayerIndex) {
    PlayerSpawnConfig config{};
    config.userId = 7001;
    config.playerIndex = 100;

    auto result = spawner->spawnPlayer(config);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.y, 150.0F + 100.0F * 100.0F);  // Very high Y position
}

TEST_F(RTypeEntitySpawnerTest, SpawnPlayerWithZeroUserId) {
    PlayerSpawnConfig config{};
    config.userId = 0;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.networkId, 0U);
}

TEST_F(RTypeEntitySpawnerTest, DestroyPlayerTwice) {
    PlayerSpawnConfig config{};
    config.userId = 7002;
    config.playerIndex = 0;

    auto result = spawner->spawnPlayer(config);
    ASSERT_TRUE(result.success);

    spawner->destroyPlayer(result.entity);
    spawner->destroyPlayer(result.entity);  // Should not crash
    EXPECT_FALSE(registry->isAlive(result.entity));
}

TEST_F(RTypeEntitySpawnerTest, GetPlayerEntityWithNullNetworkSystem) {
    auto spawnerWithNullNetwork = std::make_unique<RTypeEntitySpawner>(
        registry, nullptr, std::nullopt, std::nullopt);

    auto entityOpt = spawnerWithNullNetwork->getPlayerEntity(1000);
    EXPECT_FALSE(entityOpt.has_value());
}
