/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Unit test to exercise NetworkServer message paths to increase coverage.
*/

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>

#include "../../src/server/network/NetworkServer.hpp"
#include "../../src/client/network/NetworkClient.hpp"

using namespace rtype;
using namespace rtype::server;
using namespace rtype::client;

TEST(NetworkServerMessages, DeliverVariousMessages) {
    NetworkServer::Config serverConfig;
    serverConfig.clientTimeout = std::chrono::milliseconds(500);

    NetworkServer server(serverConfig);
    std::atomic_bool serverConnected{false};
    std::uint32_t userId{0};
    server.onClientConnected([&](std::uint32_t id) {
        serverConnected = true;
        userId = id;
    });

    ASSERT_TRUE(server.start(0));
    const uint16_t serverPort = server.port();

    // Start client
    NetworkClient::Config clientConfig;
    NetworkClient client(clientConfig);
    std::atomic_bool clientConnected{false};
    std::atomic_bool spawnReceived{false};
    std::atomic_bool moveReceived{false};
    std::atomic_bool destroyReceived{false};
    std::atomic_bool gameStateReceived{false};

    client.onConnected([&](std::uint32_t id) { (void)id; clientConnected = true; });
    client.onEntitySpawn([&](EntitySpawnEvent ev){ (void)ev; spawnReceived = true; });
    client.onEntityMove([&](EntityMoveEvent ev){ (void)ev; moveReceived = true; });
    client.onEntityDestroy([&](std::uint32_t ev){ (void)ev; destroyReceived = true; });
    client.onGameStateChange([&](GameStateEvent ev){ (void)ev; gameStateReceived = true; });

    ASSERT_TRUE(client.connect("127.0.0.1", serverPort));

    // Wait until connected
    auto start = std::chrono::steady_clock::now();
    while (!clientConnected && std::chrono::steady_clock::now() - start < std::chrono::seconds(2)) {
        client.poll();
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ASSERT_TRUE(clientConnected);
    ASSERT_TRUE(serverConnected);

    // Spawn entity (reliable)
    constexpr std::uint32_t entityId = 5001;
    server.spawnEntity(entityId, network::EntityType::Bydos, 0, 10.0f, 20.0f);

    // Move entity (unreliable)
    server.moveEntity(entityId, 15.0f, 25.0f, 1.0f, 1.0f);

    // Update game state
    server.updateGameState(network::GameState::Running);

    // Send user list where only our user exists
    server.sendUserList(userId, {userId});

    // Destroy entity
    server.destroyEntity(entityId);

    // Wait for messages to be processed and handled by client
    start = std::chrono::steady_clock::now();
    const auto deadline = start + std::chrono::seconds(2);
    while ((!(spawnReceived && moveReceived && destroyReceived && gameStateReceived)) && std::chrono::steady_clock::now() < deadline) {
        client.poll();
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    EXPECT_TRUE(spawnReceived);
    EXPECT_TRUE(moveReceived);
    EXPECT_TRUE(destroyReceived);
    EXPECT_TRUE(gameStateReceived);

    // Cleanup
    client.disconnect();
    server.stop();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
