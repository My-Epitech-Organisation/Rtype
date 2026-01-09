/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_player_spawner - Unit tests for PlayerSpawner
*/

#include <gtest/gtest.h>

#include <memory>
#include <optional>

#include "core/Registry/Registry.hpp"
#include "network/ServerNetworkSystem.hpp"
#include "network/NetworkServer.hpp"
#include "serverApp/player/playerSpawner/PlayerSpawner.hpp"
#include "games/rtype/shared/Components/BoundingBoxComponent.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"
#include "games/rtype/shared/Components/HealthComponent.hpp"
#include "games/rtype/shared/Components/NetworkIdComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/Tags.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"
#include "games/rtype/shared/Components/WeaponComponent.hpp"
#include "games/rtype/shared/Config/EntityConfig/EntityConfig.hpp"

using namespace rtype::server;
using namespace ECS;

// ============================================================================
// TEST FIXTURE
// ============================================================================

class PlayerSpawnerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Load entity configurations required by PlayerSpawner
        auto& entityConfigRegistry =
            rtype::games::rtype::shared::EntityConfigRegistry::getInstance();
        entityConfigRegistry.loadPlayersWithSearch("config/game/players.toml");

        registry_ = std::make_shared<Registry>();
        NetworkServer::Config config;
        config.clientTimeout = std::chrono::milliseconds(5000);
        server_ = std::make_shared<NetworkServer>(config);
        networkSystem_ = std::make_unique<ServerNetworkSystem>(registry_, server_);
    }

    void TearDown() override {
        networkSystem_.reset();
        server_->stop();
        server_.reset();
        registry_.reset();
    }

    std::shared_ptr<Registry> registry_;
    std::shared_ptr<NetworkServer> server_;
    std::unique_ptr<ServerNetworkSystem> networkSystem_;
};

// ============================================================================
// CONSTRUCTOR TESTS
// ============================================================================

TEST_F(PlayerSpawnerTest, Constructor_DefaultConfig) {
    EXPECT_NO_THROW({
        PlayerSpawner spawner(registry_, networkSystem_.get());
    });
}

TEST_F(PlayerSpawnerTest, Constructor_CustomConfig) {
    SpawnConfig config;
    config.baseX = 200.0f;
    config.baseY = 300.0f;
    config.yOffset = 50.0f;
    config.playerWidth = 40.0f;
    config.playerHeight = 20.0f;
    config.playerLives = 5;
    config.shootCooldown = 0.5f;

    EXPECT_NO_THROW({
        PlayerSpawner spawner(registry_, networkSystem_.get(), config);
    });
}

TEST_F(PlayerSpawnerTest, Constructor_NullNetworkSystem) {
    EXPECT_NO_THROW({
        PlayerSpawner spawner(registry_, nullptr);
    });
}

// ============================================================================
// SPAWN PLAYER TESTS
// ============================================================================

TEST_F(PlayerSpawnerTest, SpawnPlayer_FirstPlayer) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result = spawner.spawnPlayer(1, 0);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.networkId, 1u);
    EXPECT_FLOAT_EQ(result.x, 100.0f);  // Default baseX
    EXPECT_FLOAT_EQ(result.y, 150.0f);  // Default baseY
}

TEST_F(PlayerSpawnerTest, SpawnPlayer_SecondPlayer) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result = spawner.spawnPlayer(2, 1);

    EXPECT_TRUE(result.success);
    EXPECT_EQ(result.networkId, 2u);
    EXPECT_FLOAT_EQ(result.y, 250.0f);  // baseY + yOffset
}

TEST_F(PlayerSpawnerTest, SpawnPlayer_MultiplePlayersWithDifferentIndices) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result0 = spawner.spawnPlayer(1, 0);
    auto result1 = spawner.spawnPlayer(2, 1);
    auto result2 = spawner.spawnPlayer(3, 2);
    auto result3 = spawner.spawnPlayer(4, 3);

    EXPECT_TRUE(result0.success);
    EXPECT_TRUE(result1.success);
    EXPECT_TRUE(result2.success);
    EXPECT_TRUE(result3.success);

    // Check Y positions increase
    EXPECT_LT(result0.y, result1.y);
    EXPECT_LT(result1.y, result2.y);
    EXPECT_LT(result2.y, result3.y);
}

TEST_F(PlayerSpawnerTest, SpawnPlayer_HasAllComponents) {
    using Transform = rtype::games::rtype::shared::TransformComponent;
    using TransformComponent = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;
    using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;
    using Weapon = rtype::games::rtype::shared::WeaponComponent;
    using BoundingBox = rtype::games::rtype::shared::BoundingBoxComponent;
    using PlayerTag = rtype::games::rtype::shared::PlayerTag;
    using Health = rtype::games::rtype::shared::HealthComponent;
    using NetworkIdComponent = rtype::games::rtype::shared::NetworkIdComponent;

    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result = spawner.spawnPlayer(1, 0);

    EXPECT_TRUE(result.success);

    ECS::Entity entity = result.entity;

    EXPECT_TRUE(registry_->hasComponent<Transform>(entity));
    EXPECT_TRUE(registry_->hasComponent<TransformComponent>(entity));
    EXPECT_TRUE(registry_->hasComponent<Velocity>(entity));
    EXPECT_TRUE(registry_->hasComponent<ShootCooldown>(entity));
    EXPECT_TRUE(registry_->hasComponent<Weapon>(entity));
    EXPECT_TRUE(registry_->hasComponent<BoundingBox>(entity));
    EXPECT_TRUE(registry_->hasComponent<PlayerTag>(entity));
    EXPECT_TRUE(registry_->hasComponent<Health>(entity));
    EXPECT_TRUE(registry_->hasComponent<NetworkIdComponent>(entity));
}

TEST_F(PlayerSpawnerTest, SpawnPlayer_HealthSetCorrectly) {
    using Health = rtype::games::rtype::shared::HealthComponent;

    SpawnConfig config;
    config.playerLives = 7;

    PlayerSpawner spawner(registry_, networkSystem_.get(), config);

    auto result = spawner.spawnPlayer(1, 0);
    auto& health = registry_->getComponent<Health>(result.entity);

    // Health comes from player config, should be positive and at max
    EXPECT_GT(health.current, 0);
    EXPECT_EQ(health.current, health.max);
}

TEST_F(PlayerSpawnerTest, SpawnPlayer_WithoutNetworkSystem) {
    PlayerSpawner spawner(registry_, nullptr);

    auto result = spawner.spawnPlayer(1, 0);

    EXPECT_TRUE(result.success);
}

TEST_F(PlayerSpawnerTest, SpawnPlayer_CustomConfig) {
    SpawnConfig config;
    config.baseX = 500.0f;
    config.baseY = 100.0f;
    config.yOffset = 75.0f;

    PlayerSpawner spawner(registry_, networkSystem_.get(), config);

    auto result = spawner.spawnPlayer(1, 2);

    EXPECT_TRUE(result.success);
    EXPECT_FLOAT_EQ(result.x, 500.0f);
    EXPECT_FLOAT_EQ(result.y, 250.0f);  // 100 + 2 * 75
}

// ============================================================================
// DESTROY PLAYER TESTS
// ============================================================================

TEST_F(PlayerSpawnerTest, DestroyPlayer_ValidPlayer) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result = spawner.spawnPlayer(1, 0);
    EXPECT_TRUE(result.success);

    bool destroyed = spawner.destroyPlayer(1);
    EXPECT_TRUE(destroyed);
}

TEST_F(PlayerSpawnerTest, DestroyPlayer_InvalidUserId) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    bool destroyed = spawner.destroyPlayer(999);
    EXPECT_FALSE(destroyed);
}

TEST_F(PlayerSpawnerTest, DestroyPlayer_WithoutNetworkSystem) {
    PlayerSpawner spawner(registry_, nullptr);

    bool destroyed = spawner.destroyPlayer(1);
    EXPECT_FALSE(destroyed);
}

TEST_F(PlayerSpawnerTest, DestroyPlayer_AlreadyDestroyed) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result = spawner.spawnPlayer(1, 0);
    EXPECT_TRUE(result.success);

    bool destroyed1 = spawner.destroyPlayer(1);
    EXPECT_TRUE(destroyed1);

    // Second destroy might succeed or fail depending on implementation
    // Just verify it doesn't crash
    EXPECT_NO_THROW({
        spawner.destroyPlayer(1);
    });
}

// ============================================================================
// GET PLAYER ENTITY TESTS
// ============================================================================

TEST_F(PlayerSpawnerTest, GetPlayerEntity_ValidPlayer) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result = spawner.spawnPlayer(1, 0);
    EXPECT_TRUE(result.success);

    auto entity = spawner.getPlayerEntity(1);
    EXPECT_TRUE(entity.has_value());
    EXPECT_EQ(entity->id, result.entity.id);
}

TEST_F(PlayerSpawnerTest, GetPlayerEntity_InvalidUserId) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto entity = spawner.getPlayerEntity(999);
    EXPECT_FALSE(entity.has_value());
}

TEST_F(PlayerSpawnerTest, GetPlayerEntity_WithoutNetworkSystem) {
    PlayerSpawner spawner(registry_, nullptr);

    auto entity = spawner.getPlayerEntity(1);
    EXPECT_FALSE(entity.has_value());
}

TEST_F(PlayerSpawnerTest, GetPlayerEntity_AfterDestroy) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result = spawner.spawnPlayer(1, 0);
    EXPECT_TRUE(result.success);

    spawner.destroyPlayer(1);

    // The behavior depends on whether network system clears the mapping
    // Just verify it doesn't crash
    EXPECT_NO_THROW({
        auto entity = spawner.getPlayerEntity(1);
        (void)entity;
    });
}

// ============================================================================
// NETWORK REGISTRATION TESTS
// ============================================================================

TEST_F(PlayerSpawnerTest, NetworkRegistration_EntityIsRegistered) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result = spawner.spawnPlayer(1, 0);
    EXPECT_TRUE(result.success);

    auto networkId = networkSystem_->getNetworkId(result.entity);
    EXPECT_TRUE(networkId.has_value());
    EXPECT_EQ(*networkId, 1u);
}

TEST_F(PlayerSpawnerTest, NetworkRegistration_CanFindByNetworkId) {
    PlayerSpawner spawner(registry_, networkSystem_.get());

    auto result = spawner.spawnPlayer(42, 0);
    EXPECT_TRUE(result.success);

    auto entity = networkSystem_->findEntityByNetworkId(42);
    EXPECT_TRUE(entity.has_value());
}
