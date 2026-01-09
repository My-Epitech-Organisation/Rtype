/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_ServerNetworkSystem_grace - Unit tests for ServerNetworkSystem grace/timeout behavior
*/

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include "../../src/server/network/ServerNetworkSystem.hpp"
#include "../../src/server/network/NetworkServer.hpp"
#include "../../lib/ecs/src/core/Registry/Registry.hpp"

using namespace rtype::server;
using namespace ECS;
using namespace rtype::network;

// Note: This test waits for the configured grace period (5s) to expire; keep it minimal.
TEST(ServerNetworkSystemGraceTest, GracePeriodExpires_FinalizeCalled) {
    auto registry = std::make_shared<Registry>();
    NetworkServer::Config config;
    config.clientTimeout = std::chrono::milliseconds(5000);
    auto server = std::make_shared<NetworkServer>(config);
    ServerNetworkSystem system(registry, server);

    // Spawn and register entity and associate with userId
    ECS::Entity entity = registry->spawnEntity();
    system.registerNetworkedEntity(entity, 42, ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    std::uint32_t userId = 1000;
    system.setPlayerEntity(userId, entity);

    bool callbackCalled = false;
    system.onClientDisconnected([&](std::uint32_t uid) {
        if (uid == userId) callbackCalled = true;
    });

    // Trigger disconnect with Timeout (uses grace period)
    system.handleClientDisconnected(userId, DisconnectReason::Timeout);

    // Immediately should NOT have finalized
    EXPECT_FALSE(callbackCalled);

    // Wait longer than grace period, then call update() which triggers processing
    std::this_thread::sleep_for(std::chrono::milliseconds(5200));
    system.update();

    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(system.getPlayerEntity(userId).has_value());
}

TEST(ServerNetworkSystemGraceTest, ImmediateFinalize_InvokesCallback) {
    auto registry = std::make_shared<Registry>();
    NetworkServer::Config config;
    config.clientTimeout = std::chrono::milliseconds(5000);
    auto server = std::make_shared<NetworkServer>(config);
    ServerNetworkSystem system(registry, server);

    ECS::Entity entity = registry->spawnEntity();
    system.registerNetworkedEntity(entity, 84, ServerNetworkSystem::EntityType::Player, 0.0f, 0.0f);
    std::uint32_t userId = 2000;
    system.setPlayerEntity(userId, entity);

    bool callbackCalled = false;
    system.onClientDisconnected([&](std::uint32_t uid) {
        if (uid == userId) callbackCalled = true;
    });

    // Trigger disconnect with a non-grace reason
    system.handleClientDisconnected(userId, DisconnectReason::RemoteRequest);

    // Should have been finalized immediately
    EXPECT_TRUE(callbackCalled);
    EXPECT_FALSE(system.getPlayerEntity(userId).has_value());
}
