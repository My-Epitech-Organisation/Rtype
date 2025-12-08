/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_ServerNetworkSystem - Unit tests for ServerNetworkSystem
*/

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

#include "../../src/server/network/ServerNetworkSystem.hpp"
#include "../../src/server/network/NetworkServer.hpp"
#include "../../lib/rtype_ecs/src/core/Registry/Registry.hpp"

using namespace rtype::server;
using namespace ECS;

// ============================================================================
// TEST FIXTURE
// ============================================================================

class ServerNetworkSystemTest : public ::testing::Test {
   protected:
    void SetUp() override {
        registry_ = std::make_shared<Registry>();
        NetworkServer::Config config;
        config.clientTimeout = std::chrono::milliseconds(5000);
        server_ = std::make_shared<NetworkServer>(config);
        system_ = std::make_unique<ServerNetworkSystem>(registry_, server_);
    }

    void TearDown() override {
        system_.reset();
        server_->stop();
        server_.reset();
        registry_.reset();
    }

    std::shared_ptr<Registry> registry_;
    std::shared_ptr<NetworkServer> server_;
    std::unique_ptr<ServerNetworkSystem> system_;
};

// ============================================================================
// CONSTRUCTOR TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, Constructor_ValidParameters) {
    auto registry = std::make_shared<Registry>();
    auto server = std::make_shared<NetworkServer>();
    
    EXPECT_NO_THROW({
        ServerNetworkSystem system(registry, server);
    });
}

// ============================================================================
// REGISTER ENTITY TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, RegisterNetworkedEntity_SingleEntity) {
    ECS::Entity entity = registry_->spawnEntity();
    std::uint32_t networkId = 1;
    float x = 100.0f;
    float y = 200.0f;

    EXPECT_NO_THROW({
        system_->registerNetworkedEntity(entity, networkId,
            ServerNetworkSystem::EntityType::Player, x, y);
    });

    auto foundEntity = system_->findEntityByNetworkId(networkId);
    EXPECT_TRUE(foundEntity.has_value());
    EXPECT_EQ(foundEntity->id, entity.id);

    auto foundNetworkId = system_->getNetworkId(entity);
    EXPECT_TRUE(foundNetworkId.has_value());
    EXPECT_EQ(*foundNetworkId, networkId);
}

TEST_F(ServerNetworkSystemTest, RegisterNetworkedEntity_MultipleEntities) {
    ECS::Entity entity1 = registry_->spawnEntity();
    ECS::Entity entity2 = registry_->spawnEntity();
    ECS::Entity entity3 = registry_->spawnEntity();

    system_->registerNetworkedEntity(entity1, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    system_->registerNetworkedEntity(entity2, 2,
        ServerNetworkSystem::EntityType::Bydos, 100.0f, 100.0f);
    system_->registerNetworkedEntity(entity3, 3,
        ServerNetworkSystem::EntityType::Missile, 200.0f, 200.0f);

    EXPECT_TRUE(system_->findEntityByNetworkId(1).has_value());
    EXPECT_TRUE(system_->findEntityByNetworkId(2).has_value());
    EXPECT_TRUE(system_->findEntityByNetworkId(3).has_value());
}

TEST_F(ServerNetworkSystemTest, RegisterNetworkedEntity_DifferentTypes) {
    ECS::Entity player = registry_->spawnEntity();
    ECS::Entity enemy = registry_->spawnEntity();
    ECS::Entity projectile = registry_->spawnEntity();

    system_->registerNetworkedEntity(player, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    system_->registerNetworkedEntity(enemy, 2,
        ServerNetworkSystem::EntityType::Bydos, 50.0f, 50.0f);
    system_->registerNetworkedEntity(projectile, 3,
        ServerNetworkSystem::EntityType::Missile, 75.0f, 75.0f);

    EXPECT_EQ(system_->getNetworkId(player), 1u);
    EXPECT_EQ(system_->getNetworkId(enemy), 2u);
    EXPECT_EQ(system_->getNetworkId(projectile), 3u);
}

// ============================================================================
// UNREGISTER ENTITY TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, UnregisterNetworkedEntity_ValidEntity) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);

    EXPECT_TRUE(system_->findEntityByNetworkId(1).has_value());

    system_->unregisterNetworkedEntity(entity);

    EXPECT_FALSE(system_->findEntityByNetworkId(1).has_value());
    EXPECT_FALSE(system_->getNetworkId(entity).has_value());
}

TEST_F(ServerNetworkSystemTest, UnregisterNetworkedEntity_NotRegistered) {
    ECS::Entity entity = registry_->spawnEntity();

    // Should not throw when unregistering non-registered entity
    EXPECT_NO_THROW({
        system_->unregisterNetworkedEntity(entity);
    });
}

TEST_F(ServerNetworkSystemTest, UnregisterNetworkedEntityById_ValidId) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 42,
        ServerNetworkSystem::EntityType::Bydos, 100.0f, 100.0f);

    EXPECT_TRUE(system_->findEntityByNetworkId(42).has_value());

    system_->unregisterNetworkedEntityById(42);

    EXPECT_FALSE(system_->findEntityByNetworkId(42).has_value());
}

TEST_F(ServerNetworkSystemTest, UnregisterNetworkedEntityById_InvalidId) {
    // Should not throw for invalid ID
    EXPECT_NO_THROW({
        system_->unregisterNetworkedEntityById(999);
    });
}

// ============================================================================
// PLAYER ENTITY TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, SetPlayerEntity_ValidUser) {
    ECS::Entity entity = registry_->spawnEntity();
    std::uint32_t userId = 100;

    system_->setPlayerEntity(userId, entity);

    auto playerEntity = system_->getPlayerEntity(userId);
    EXPECT_TRUE(playerEntity.has_value());
    EXPECT_EQ(playerEntity->id, entity.id);
}

TEST_F(ServerNetworkSystemTest, GetPlayerEntity_NotRegistered) {
    auto playerEntity = system_->getPlayerEntity(999);
    EXPECT_FALSE(playerEntity.has_value());
}

TEST_F(ServerNetworkSystemTest, SetPlayerEntity_MultipleUsers) {
    ECS::Entity entity1 = registry_->spawnEntity();
    ECS::Entity entity2 = registry_->spawnEntity();

    system_->setPlayerEntity(1, entity1);
    system_->setPlayerEntity(2, entity2);

    EXPECT_EQ(system_->getPlayerEntity(1)->id, entity1.id);
    EXPECT_EQ(system_->getPlayerEntity(2)->id, entity2.id);
}

TEST_F(ServerNetworkSystemTest, SetPlayerEntity_OverwriteExisting) {
    ECS::Entity entity1 = registry_->spawnEntity();
    ECS::Entity entity2 = registry_->spawnEntity();

    system_->setPlayerEntity(1, entity1);
    EXPECT_EQ(system_->getPlayerEntity(1)->id, entity1.id);

    system_->setPlayerEntity(1, entity2);
    EXPECT_EQ(system_->getPlayerEntity(1)->id, entity2.id);
}

// ============================================================================
// INPUT HANDLER TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, SetInputHandler_ValidHandler) {
    bool handlerCalled = false;
    std::uint32_t receivedUserId = 0;
    std::uint8_t receivedInput = 0;

    system_->setInputHandler([&](std::uint32_t userId, std::uint8_t inputMask,
                                 std::optional<ECS::Entity> entity) {
        handlerCalled = true;
        receivedUserId = userId;
        receivedInput = inputMask;
    });

    // Handler should be set (cannot directly test without triggering input)
    EXPECT_NO_THROW({
        system_->setInputHandler(nullptr);  // Clear handler
    });
}

// ============================================================================
// CALLBACK TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, OnClientConnected_SetCallback) {
    bool callbackSet = false;

    EXPECT_NO_THROW({
        system_->onClientConnected([&](std::uint32_t userId) {
            callbackSet = true;
        });
    });
}

TEST_F(ServerNetworkSystemTest, OnClientDisconnected_SetCallback) {
    bool callbackSet = false;

    EXPECT_NO_THROW({
        system_->onClientDisconnected([&](std::uint32_t userId) {
            callbackSet = true;
        });
    });
}

// ============================================================================
// UPDATE ENTITY POSITION TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, UpdateEntityPosition_ValidEntity) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);

    EXPECT_NO_THROW({
        system_->updateEntityPosition(1, 150.0f, 250.0f, 10.0f, 5.0f);
    });
}

TEST_F(ServerNetworkSystemTest, UpdateEntityPosition_InvalidNetworkId) {
    // Should not throw for invalid network ID
    EXPECT_NO_THROW({
        system_->updateEntityPosition(999, 100.0f, 200.0f, 0.0f, 0.0f);
    });
}

TEST_F(ServerNetworkSystemTest, UpdateEntityPosition_MultipleUpdates) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);

    system_->updateEntityPosition(1, 100.0f, 100.0f, 1.0f, 1.0f);
    system_->updateEntityPosition(1, 200.0f, 200.0f, 2.0f, 2.0f);
    system_->updateEntityPosition(1, 300.0f, 300.0f, 3.0f, 3.0f);

    // Should not throw
    EXPECT_NO_THROW({
        system_->broadcastEntityUpdates();
    });
}

// ============================================================================
// CORRECT PLAYER POSITION TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, CorrectPlayerPosition_ValidUser) {
    EXPECT_NO_THROW({
        system_->correctPlayerPosition(1, 500.0f, 400.0f);
    });
}

// ============================================================================
// BROADCAST TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, BroadcastEntityUpdates_NoEntities) {
    EXPECT_NO_THROW({
        system_->broadcastEntityUpdates();
    });
}

TEST_F(ServerNetworkSystemTest, BroadcastEntityUpdates_WithDirtyEntities) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);

    system_->updateEntityPosition(1, 100.0f, 200.0f, 5.0f, 10.0f);

    EXPECT_NO_THROW({
        system_->broadcastEntityUpdates();
    });

    // Second broadcast should not re-broadcast clean entities
    EXPECT_NO_THROW({
        system_->broadcastEntityUpdates();
    });
}

TEST_F(ServerNetworkSystemTest, BroadcastGameStart) {
    EXPECT_NO_THROW({
        system_->broadcastGameStart();
    });
}

// ============================================================================
// UPDATE TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, Update_NoEntities) {
    EXPECT_NO_THROW({
        system_->update();
    });
}

TEST_F(ServerNetworkSystemTest, Update_WithEntities) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);

    EXPECT_NO_THROW({
        system_->update();
    });
}

TEST_F(ServerNetworkSystemTest, Update_RemovesDeadEntities) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);

    EXPECT_TRUE(system_->findEntityByNetworkId(1).has_value());

    registry_->killEntity(entity);

    system_->update();

    EXPECT_FALSE(system_->findEntityByNetworkId(1).has_value());
}

// ============================================================================
// GET NETWORK ID TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, GetNetworkId_RegisteredEntity) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 42,
        ServerNetworkSystem::EntityType::Bydos, 0.0f, 0.0f);

    auto networkId = system_->getNetworkId(entity);
    EXPECT_TRUE(networkId.has_value());
    EXPECT_EQ(*networkId, 42u);
}

TEST_F(ServerNetworkSystemTest, GetNetworkId_UnregisteredEntity) {
    ECS::Entity entity = registry_->spawnEntity();
    auto networkId = system_->getNetworkId(entity);
    EXPECT_FALSE(networkId.has_value());
}

// ============================================================================
// FIND ENTITY BY NETWORK ID TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, FindEntityByNetworkId_ValidId) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 123,
        ServerNetworkSystem::EntityType::Missile, 50.0f, 50.0f);

    auto found = system_->findEntityByNetworkId(123);
    EXPECT_TRUE(found.has_value());
    EXPECT_EQ(found->id, entity.id);
}

TEST_F(ServerNetworkSystemTest, FindEntityByNetworkId_InvalidId) {
    auto found = system_->findEntityByNetworkId(999);
    EXPECT_FALSE(found.has_value());
}

// ============================================================================
// NEXT NETWORK ID TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, NextNetworkId_Sequential) {
    std::uint32_t id1 = system_->nextNetworkId();
    std::uint32_t id2 = system_->nextNetworkId();
    std::uint32_t id3 = system_->nextNetworkId();

    EXPECT_EQ(id2, id1 + 1);
    EXPECT_EQ(id3, id2 + 1);
}

TEST_F(ServerNetworkSystemTest, NextNetworkId_StartsAtOne) {
    std::uint32_t id = system_->nextNetworkId();
    EXPECT_GE(id, 1u);
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, Integration_FullEntityLifecycle) {
    // Spawn entity
    ECS::Entity entity = registry_->spawnEntity();
    std::uint32_t networkId = system_->nextNetworkId();

    // Register
    system_->registerNetworkedEntity(entity, networkId,
        ServerNetworkSystem::EntityType::Player, 100.0f, 200.0f);

    EXPECT_TRUE(system_->findEntityByNetworkId(networkId).has_value());
    EXPECT_EQ(system_->getNetworkId(entity), networkId);

    // Update position
    system_->updateEntityPosition(networkId, 150.0f, 250.0f, 1.0f, 2.0f);
    system_->broadcastEntityUpdates();

    // Update system
    system_->update();

    // Unregister
    system_->unregisterNetworkedEntity(entity);

    EXPECT_FALSE(system_->findEntityByNetworkId(networkId).has_value());
}

TEST_F(ServerNetworkSystemTest, Integration_MultiplePlayersWithInputs) {
    // Setup input handler
    std::vector<std::pair<std::uint32_t, std::uint8_t>> receivedInputs;
    system_->setInputHandler(
        [&](std::uint32_t userId, std::uint8_t inputMask,
            std::optional<ECS::Entity> entity) {
            receivedInputs.push_back({userId, inputMask});
        });

    // Create players
    ECS::Entity player1 = registry_->spawnEntity();
    ECS::Entity player2 = registry_->spawnEntity();

    system_->registerNetworkedEntity(player1, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    system_->registerNetworkedEntity(player2, 2,
        ServerNetworkSystem::EntityType::Player, 100.0f, 0.0f);

    system_->setPlayerEntity(100, player1);
    system_->setPlayerEntity(101, player2);

    EXPECT_EQ(system_->getPlayerEntity(100)->id, player1.id);
    EXPECT_EQ(system_->getPlayerEntity(101)->id, player2.id);
}

TEST_F(ServerNetworkSystemTest, Integration_EntityCleanupOnRegistryKill) {
    ECS::Entity entity1 = registry_->spawnEntity();
    ECS::Entity entity2 = registry_->spawnEntity();

    system_->registerNetworkedEntity(entity1, 1,
        ServerNetworkSystem::EntityType::Bydos, 0.0f, 0.0f);
    system_->registerNetworkedEntity(entity2, 2,
        ServerNetworkSystem::EntityType::Bydos, 100.0f, 100.0f);

    EXPECT_TRUE(system_->findEntityByNetworkId(1).has_value());
    EXPECT_TRUE(system_->findEntityByNetworkId(2).has_value());

    // Kill one entity
    registry_->killEntity(entity1);

    // Update should clean up dead entity
    system_->update();

    EXPECT_FALSE(system_->findEntityByNetworkId(1).has_value());
    EXPECT_TRUE(system_->findEntityByNetworkId(2).has_value());
}

// ============================================================================
// CALLBACK INTEGRATION TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, CallbackIntegration_ConnectedAndDisconnected) {
    std::vector<std::uint32_t> connectedUsers;
    std::vector<std::uint32_t> disconnectedUsers;

    system_->onClientConnected([&](std::uint32_t userId) {
        connectedUsers.push_back(userId);
    });

    system_->onClientDisconnected([&](std::uint32_t userId) {
        disconnectedUsers.push_back(userId);
    });

    // Callbacks are set - would be triggered by actual network events
    EXPECT_TRUE(connectedUsers.empty());
    EXPECT_TRUE(disconnectedUsers.empty());
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, EdgeCase_RegisterSameNetworkIdTwice) {
    ECS::Entity entity1 = registry_->spawnEntity();
    ECS::Entity entity2 = registry_->spawnEntity();

    system_->registerNetworkedEntity(entity1, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    
    // Registering with same network ID should overwrite
    system_->registerNetworkedEntity(entity2, 1,
        ServerNetworkSystem::EntityType::Bydos, 100.0f, 100.0f);

    auto found = system_->findEntityByNetworkId(1);
    EXPECT_TRUE(found.has_value());
    EXPECT_EQ(found->id, entity2.id);
}

TEST_F(ServerNetworkSystemTest, EdgeCase_UpdateAfterUnregister) {
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);

    system_->unregisterNetworkedEntity(entity);

    // Update position on unregistered entity should not crash
    EXPECT_NO_THROW({
        system_->updateEntityPosition(1, 100.0f, 100.0f, 0.0f, 0.0f);
    });
}

TEST_F(ServerNetworkSystemTest, EdgeCase_ZeroPosition) {
    ECS::Entity entity = registry_->spawnEntity();
    
    EXPECT_NO_THROW({
        system_->registerNetworkedEntity(entity, 1,
            ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    });

    EXPECT_NO_THROW({
        system_->updateEntityPosition(1, 0.0f, 0.0f, 0.0f, 0.0f);
    });
}

TEST_F(ServerNetworkSystemTest, EdgeCase_NegativePosition) {
    ECS::Entity entity = registry_->spawnEntity();
    
    EXPECT_NO_THROW({
        system_->registerNetworkedEntity(entity, 1,
            ServerNetworkSystem::EntityType::Player, -100.0f, -200.0f);
    });

    EXPECT_NO_THROW({
        system_->updateEntityPosition(1, -50.0f, -75.0f, -1.0f, -2.0f);
    });
}

TEST_F(ServerNetworkSystemTest, EdgeCase_LargeNetworkId) {
    ECS::Entity entity = registry_->spawnEntity();
    std::uint32_t largeId = 0xFFFFFFFF;

    EXPECT_NO_THROW({
        system_->registerNetworkedEntity(entity, largeId,
            ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    });

    EXPECT_TRUE(system_->findEntityByNetworkId(largeId).has_value());
}

// ============================================================================
// INPUT HANDLER INTEGRATION TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, InputHandler_WithPlayerEntity) {
    std::uint32_t receivedUserId = 0;
    std::uint8_t receivedInput = 0;
    bool hasEntity = false;

    system_->setInputHandler(
        [&](std::uint32_t userId, std::uint8_t inputMask,
            std::optional<ECS::Entity> entity) {
            receivedUserId = userId;
            receivedInput = inputMask;
            hasEntity = entity.has_value();
        });

    // Create and register player entity
    ECS::Entity player = registry_->spawnEntity();
    system_->registerNetworkedEntity(player, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    system_->setPlayerEntity(100, player);

    // The handler is set and entity is registered
    EXPECT_TRUE(system_->getPlayerEntity(100).has_value());
}

TEST_F(ServerNetworkSystemTest, InputHandler_NullHandler) {
    // Set handler then clear it
    system_->setInputHandler(
        [](std::uint32_t, std::uint8_t, std::optional<ECS::Entity>) {});
    
    system_->setInputHandler(nullptr);

    // Should not crash when handler is null
    EXPECT_NO_THROW({
        system_->update();
    });
}

// ============================================================================
// BROADCAST WITH CONNECTED SERVER TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, BroadcastWithRunningServer) {
    // Start the server
    EXPECT_TRUE(server_->start(14245));
    
    ECS::Entity entity = registry_->spawnEntity();
    system_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 100.0f, 200.0f);

    // Update and broadcast
    system_->updateEntityPosition(1, 150.0f, 250.0f, 5.0f, 10.0f);
    
    EXPECT_NO_THROW({
        system_->broadcastEntityUpdates();
    });
    
    EXPECT_NO_THROW({
        system_->broadcastGameStart();
    });

    server_->stop();
}

TEST_F(ServerNetworkSystemTest, CorrectPlayerPositionWithRunningServer) {
    EXPECT_TRUE(server_->start(14246));
    
    EXPECT_NO_THROW({
        system_->correctPlayerPosition(1, 500.0f, 400.0f);
    });

    server_->stop();
}

// ============================================================================
// MULTIPLE ENTITY LIFECYCLE TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, MultipleEntities_RegisterUpdateUnregister) {
    std::vector<ECS::Entity> entities;
    
    // Register multiple entities
    for (int i = 0; i < 10; ++i) {
        ECS::Entity entity = registry_->spawnEntity();
        entities.push_back(entity);
        system_->registerNetworkedEntity(entity, static_cast<std::uint32_t>(i + 1),
            ServerNetworkSystem::EntityType::Bydos,
            static_cast<float>(i * 100), static_cast<float>(i * 50));
    }

    // Update all positions
    for (int i = 0; i < 10; ++i) {
        system_->updateEntityPosition(static_cast<std::uint32_t>(i + 1),
            static_cast<float>(i * 100 + 50), static_cast<float>(i * 50 + 25),
            1.0f, 2.0f);
    }

    // Broadcast updates
    system_->broadcastEntityUpdates();

    // Verify all entities registered
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(system_->findEntityByNetworkId(static_cast<std::uint32_t>(i + 1)).has_value());
    }

    // Unregister half of them
    for (int i = 0; i < 5; ++i) {
        system_->unregisterNetworkedEntityById(static_cast<std::uint32_t>(i + 1));
    }

    // Verify state
    for (int i = 0; i < 5; ++i) {
        EXPECT_FALSE(system_->findEntityByNetworkId(static_cast<std::uint32_t>(i + 1)).has_value());
    }
    for (int i = 5; i < 10; ++i) {
        EXPECT_TRUE(system_->findEntityByNetworkId(static_cast<std::uint32_t>(i + 1)).has_value());
    }
}

// ============================================================================
// ENTITY TYPE TESTS
// ============================================================================

TEST_F(ServerNetworkSystemTest, AllEntityTypes) {
    ECS::Entity player = registry_->spawnEntity();
    ECS::Entity bydos = registry_->spawnEntity();
    ECS::Entity missile = registry_->spawnEntity();

    system_->registerNetworkedEntity(player, 1,
        ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    system_->registerNetworkedEntity(bydos, 2,
        ServerNetworkSystem::EntityType::Bydos, 100.0f, 100.0f);
    system_->registerNetworkedEntity(missile, 3,
        ServerNetworkSystem::EntityType::Missile, 200.0f, 200.0f);

    EXPECT_TRUE(system_->findEntityByNetworkId(1).has_value());
    EXPECT_TRUE(system_->findEntityByNetworkId(2).has_value());
    EXPECT_TRUE(system_->findEntityByNetworkId(3).has_value());
}

// ============================================================================
// NEXT NETWORK ID STRESS TEST
// ============================================================================

TEST_F(ServerNetworkSystemTest, NextNetworkId_ManyIds) {
    std::vector<std::uint32_t> ids;
    
    for (int i = 0; i < 100; ++i) {
        ids.push_back(system_->nextNetworkId());
    }

    // All IDs should be unique
    std::sort(ids.begin(), ids.end());
    auto last = std::unique(ids.begin(), ids.end());
    EXPECT_EQ(last, ids.end());  // No duplicates
}

