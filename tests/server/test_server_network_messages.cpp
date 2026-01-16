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

TEST(NetworkServerMessages, ChatRelay) {
    NetworkServer::Config serverConfig;
    serverConfig.clientTimeout = std::chrono::seconds(5);

    NetworkServer server(serverConfig);
    
    // Wire up chat relay (simulates ServerNetworkSystem logic)
    server.onClientChat([&](std::uint32_t senderId, const std::string& msg) {
        server.broadcastChat(senderId, msg);
    });

    ASSERT_TRUE(server.start(0));
    const uint16_t port = server.port();

    // Client 1 (Sender)
    NetworkClient c1;
    std::atomic<bool> c1Connected{false};
    c1.onConnected([&](std::uint32_t){ c1Connected = true; });
    ASSERT_TRUE(c1.connect("127.0.0.1", port));

    // Client 2 (Receiver)
    NetworkClient c2;
    std::atomic<bool> c2Connected{false};
    std::atomic<bool> c2Received{false};
    std::string receivedMsg;
    std::uint32_t receivedSender = 0;

    c2.onConnected([&](std::uint32_t){ c2Connected = true; });
    c2.onChatReceived([&](std::uint32_t sender, std::string msg) {
        c2Received = true;
        receivedSender = sender;
        receivedMsg = msg;
    });

    ASSERT_TRUE(c2.connect("127.0.0.1", port));

    // Wait for connections
    auto start = std::chrono::steady_clock::now();
    auto deadline = start + std::chrono::seconds(2);
    while ((!c1Connected || !c2Connected) && std::chrono::steady_clock::now() < deadline) {
        c1.poll();
        c2.poll();
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ASSERT_TRUE(c1Connected) << "Client 1 failed to connect";
    ASSERT_TRUE(c2Connected) << "Client 2 failed to connect";
    
    // Join lobby
    std::atomic<bool> c1Joined{false};
    std::atomic<bool> c2Joined{false};

    c1.onJoinLobbyResponse([&](bool success, uint8_t, const std::string&){ if(success) c1Joined = true; });
    c2.onJoinLobbyResponse([&](bool success, uint8_t, const std::string&){ if(success) c2Joined = true; });

    c1.sendJoinLobby("TEST01");
    c2.sendJoinLobby("TEST01");

    start = std::chrono::steady_clock::now();
    deadline = start + std::chrono::seconds(2);
    while ((!c1Joined || !c2Joined) && std::chrono::steady_clock::now() < deadline) {
        c1.poll();
        c2.poll();
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ASSERT_TRUE(c1Joined) << "Client 1 failed to join lobby";
    ASSERT_TRUE(c2Joined) << "Client 2 failed to join lobby";

    // Send chat from C1
    c1.sendChat("Hello C2");

    // Wait for delivery
    start = std::chrono::steady_clock::now();
    deadline = start + std::chrono::seconds(2);
    while (!c2Received && std::chrono::steady_clock::now() < deadline) {
        c1.poll();
        c2.poll();
        server.poll();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    EXPECT_TRUE(c2Received);
    EXPECT_EQ(receivedMsg, "Hello C2");
    
    // Note: Can't easily check sender ID without knowing what ID server assigned to C1.
    // But since server.broadcastChat(senderId, msg) uses senderId from onClientChat,
    // and onClientChat gives correct ID, it should match.
    // We could capture C1's ID in its onConnected callback.
    
    c1.disconnect();
    c2.disconnect();
    server.stop();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
