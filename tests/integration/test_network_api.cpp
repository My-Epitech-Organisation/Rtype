/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_network_api - Integration tests for NetworkClient and NetworkServer
*/

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "../../src/client/network/NetworkClient.hpp"
#include "../../src/server/network/NetworkServer.hpp"
#include "protocol/Payloads.hpp"

using namespace rtype;
using namespace std::chrono_literals;

// ============================================================================
// Test Fixture
// ============================================================================

class NetworkApiTest : public ::testing::Test {
   protected:
    void SetUp() override {
        server_ = std::make_unique<server::NetworkServer>();
        client_ = std::make_unique<client::NetworkClient>();
    }

    void TearDown() override {
        if (client_) {
            client_->disconnect();
            // Poll to process disconnect
            for (int i = 0; i < 10; ++i) {
                client_->poll();
                std::this_thread::sleep_for(10ms);
            }
        }
        if (server_) {
            server_->stop();
        }
        client_.reset();
        server_.reset();
    }

    // Helper to poll both client and server for a duration
    void pollBoth(std::chrono::milliseconds duration) {
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < duration) {
            if (server_)
                server_->poll();
            if (client_)
                client_->poll();
            std::this_thread::sleep_for(1ms);
        }
    }

    // Wait for an atomic flag while continuously pumping both ends
    bool waitFor(std::atomic<bool>& flag, std::chrono::milliseconds timeout,
                 std::chrono::milliseconds pumpStep = 5ms) {
        auto deadline = std::chrono::steady_clock::now() + timeout;
        while (!flag.load(std::memory_order_acquire) &&
               std::chrono::steady_clock::now() < deadline) {
            pollBoth(pumpStep);
        }
        return flag.load(std::memory_order_acquire);
    }

    static constexpr std::uint16_t TEST_PORT = 14242;

    std::unique_ptr<server::NetworkServer> server_;
    std::unique_ptr<client::NetworkClient> client_;
};

// ============================================================================
// Server Lifecycle Tests
// ============================================================================

TEST_F(NetworkApiTest, ServerStartStop) {
    EXPECT_FALSE(server_->isRunning());
    EXPECT_EQ(server_->port(), 0);

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(server_->isRunning());
    EXPECT_EQ(server_->port(), TEST_PORT);

    server_->stop();
    EXPECT_FALSE(server_->isRunning());
}

TEST_F(NetworkApiTest, ServerDoubleStart) {
    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_FALSE(server_->start(TEST_PORT));  // Already running
    server_->stop();
}

TEST_F(NetworkApiTest, ServerClientCount) {
    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_EQ(server_->clientCount(), 0);
}

// ============================================================================
// Client Lifecycle Tests
// ============================================================================

TEST_F(NetworkApiTest, ClientInitialState) {
    EXPECT_FALSE(client_->isConnected());
    EXPECT_FALSE(client_->userId().has_value());
}

TEST_F(NetworkApiTest, ClientConnectWithoutServer) {
    // Attempting to connect when no server is running
    // Should start connect attempt but not complete
    bool result = client_->connect("127.0.0.1", TEST_PORT);
    EXPECT_TRUE(result);  // Connect attempt started
    EXPECT_FALSE(client_->isConnected());  // Not connected yet
}

// ============================================================================
// Connection Tests
// ============================================================================

TEST_F(NetworkApiTest, ClientServerConnection) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> serverGotClient{false};
    std::atomic<std::uint32_t> assignedUserId{0};
    std::atomic<std::uint32_t> serverSeenUserId{0};

    server_->onClientConnected([&](std::uint32_t userId) {
        serverSeenUserId = userId;
        serverGotClient = true;
    });

    client_->onConnected([&](std::uint32_t userId) {
        assignedUserId = userId;
        clientConnected = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Poll until connected or timeout
    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(clientConnected.load());
    EXPECT_TRUE(serverGotClient.load());
    EXPECT_TRUE(client_->isConnected());
    EXPECT_EQ(assignedUserId.load(), serverSeenUserId.load());
    EXPECT_GT(assignedUserId.load(), 0u);
    EXPECT_EQ(server_->clientCount(), 1);
}

TEST_F(NetworkApiTest, ClientGracefulDisconnect) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> clientDisconnected{false};
    std::atomic<bool> serverGotDisconnect{false};
    std::uint32_t disconnectedUserId = 0;

    server_->onClientConnected([&](std::uint32_t userId) {
        (void)userId;
    });

    server_->onClientDisconnected(
        [&](std::uint32_t userId, rtype::network::DisconnectReason reason) {
            (void)reason;
            disconnectedUserId = userId;
            serverGotDisconnect = true;
        });

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    client_->onDisconnected([&](client::NetworkClient::DisconnectReason reason) {
        (void)reason;
        clientDisconnected = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Wait for connection
    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    // Disconnect
    client_->disconnect();

    // Wait for disconnect to be processed
    for (int i = 0; i < 100 && !serverGotDisconnect; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(serverGotDisconnect.load());
    EXPECT_EQ(server_->clientCount(), 0);
}

// ============================================================================
// Input Tests
// ============================================================================

TEST_F(NetworkApiTest, ClientSendInput) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> inputReceived{false};
    std::uint8_t receivedInput = 0;
    std::uint32_t inputUserId = 0;

    server_->onClientConnected([&](std::uint32_t userId) {
        (void)userId;
    });

    server_->onClientInput([&](std::uint32_t userId, std::uint8_t input) {
        inputUserId = userId;
        receivedInput = input;
        inputReceived = true;
    });

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Wait for connection
    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    // Send input
    std::uint8_t testInput =
        network::InputMask::kUp | network::InputMask::kShoot;
    EXPECT_TRUE(client_->sendInput(testInput));

    // Wait for input to be received
    for (int i = 0; i < 100 && !inputReceived; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(inputReceived.load());
    EXPECT_EQ(receivedInput, testInput);
    EXPECT_GT(inputUserId, 0u);
}

TEST_F(NetworkApiTest, ClientSendInputWhileDisconnected) {
    EXPECT_FALSE(client_->sendInput(network::InputMask::kUp));
}

// ============================================================================
// Entity Broadcast Tests
// ============================================================================

TEST_F(NetworkApiTest, ServerBroadcastEntitySpawn) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> spawnReceived{false};
    client::EntitySpawnEvent receivedEvent{};

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    client_->onEntitySpawn([&](client::EntitySpawnEvent event) {
        receivedEvent = event;
        spawnReceived = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Wait for connection
    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    // Spawn entity
    server_->spawnEntity(42, network::EntityType::Player, 0, 100.0f, 200.0f);

    // Wait for spawn to be received
    for (int i = 0; i < 100 && !spawnReceived; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(spawnReceived.load());
    EXPECT_EQ(receivedEvent.entityId, 42u);
    EXPECT_EQ(receivedEvent.type, network::EntityType::Player);
    EXPECT_FLOAT_EQ(receivedEvent.x, 100.0f);
    EXPECT_FLOAT_EQ(receivedEvent.y, 200.0f);
}

TEST_F(NetworkApiTest, ServerBroadcastEntityMove) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> moveReceived{false};
    client::EntityMoveEvent receivedEvent{};

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    client_->onEntityMove([&](client::EntityMoveEvent event) {
        receivedEvent = event;
        moveReceived = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Wait for connection
    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    // Move entity
    server_->moveEntity(123, 50.0f, 75.0f, 10.0f, -5.0f);

    // Wait for move to be received
    for (int i = 0; i < 100 && !moveReceived; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(moveReceived.load());
    EXPECT_EQ(receivedEvent.entityId, 123u);
    EXPECT_FLOAT_EQ(receivedEvent.x, 50.0f);
    EXPECT_FLOAT_EQ(receivedEvent.y, 75.0f);
    EXPECT_FLOAT_EQ(receivedEvent.vx, 10.0f);
    EXPECT_FLOAT_EQ(receivedEvent.vy, -5.0f);
}

TEST_F(NetworkApiTest, ServerBroadcastEntityDestroy) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> destroyReceived{false};
    std::uint32_t destroyedEntityId = 0;

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    client_->onEntityDestroy([&](std::uint32_t entityId) {
        destroyedEntityId = entityId;
        destroyReceived = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Wait for connection
    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    // Destroy entity
    server_->destroyEntity(999);

    // Wait for destroy to be received
    for (int i = 0; i < 100 && !destroyReceived; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(destroyReceived.load());
    EXPECT_EQ(destroyedEntityId, 999u);
}

TEST_F(NetworkApiTest, ServerPositionCorrection) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> correctionReceived{false};
    float correctedX = 0;
    float correctedY = 0;
    std::uint32_t clientUserId = 0;

    client_->onConnected([&](std::uint32_t userId) {
        clientUserId = userId;
        clientConnected = true;
    });

    client_->onPositionCorrection([&](float x, float y) {
        correctedX = x;
        correctedY = y;
        correctionReceived = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Wait for connection
    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    // Send position correction to the connected client
    server_->correctPosition(clientUserId, 150.0f, 250.0f);

    // Wait for correction to be received
    for (int i = 0; i < 100 && !correctionReceived; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(correctionReceived.load());
    EXPECT_FLOAT_EQ(correctedX, 150.0f);
    EXPECT_FLOAT_EQ(correctedY, 250.0f);
}

// ============================================================================
// Multiple Clients Test
// ============================================================================

TEST_F(NetworkApiTest, MultipleClientsConnect) {
    std::atomic<int> connectedClients{0};
    std::atomic<int> serverSeenClients{0};

    server_->onClientConnected([&](std::uint32_t userId) {
        (void)userId;
        ++serverSeenClients;
    });

    auto client2 = std::make_unique<client::NetworkClient>();
    auto client3 = std::make_unique<client::NetworkClient>();

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        ++connectedClients;
    });
    client2->onConnected([&](std::uint32_t userId) {
        (void)userId;
        ++connectedClients;
    });
    client3->onConnected([&](std::uint32_t userId) {
        (void)userId;
        ++connectedClients;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));
    EXPECT_TRUE(client2->connect("127.0.0.1", TEST_PORT));
    EXPECT_TRUE(client3->connect("127.0.0.1", TEST_PORT));

    // Poll all
    for (int i = 0; i < 200 && connectedClients < 3; ++i) {
        server_->poll();
        client_->poll();
        client2->poll();
        client3->poll();
        std::this_thread::sleep_for(5ms);
    }

    EXPECT_EQ(connectedClients.load(), 3);
    EXPECT_EQ(serverSeenClients.load(), 3);
    EXPECT_EQ(server_->clientCount(), 3);

    // Clean up
    client2->disconnect();
    client3->disconnect();
    for (int i = 0; i < 20; ++i) {
        server_->poll();
        client2->poll();
        client3->poll();
        std::this_thread::sleep_for(5ms);
    }
}

// ============================================================================
// Get Connected Clients Test
// ============================================================================

TEST_F(NetworkApiTest, GetConnectedClients) {
    std::atomic<bool> clientConnected{false};

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Wait for connection
    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    auto clients = server_->getConnectedClients();
    EXPECT_EQ(clients.size(), 1);
    EXPECT_GT(clients[0], 0u);
}

// ============================================================================
// Game State Test
// ============================================================================

TEST_F(NetworkApiTest, ServerUpdateGameState) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> stateReceived{false};
    client::GameStateEvent receivedEvent{};

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    client_->onGameStateChange([&](client::GameStateEvent event) {
        receivedEvent = event;
        stateReceived = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Wait for connection
    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    // Update game state
    server_->updateGameState(network::GameState::Running);

    // Wait for state to be received
    for (int i = 0; i < 100 && !stateReceived; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(stateReceived.load());
    EXPECT_EQ(receivedEvent.state, network::GameState::Running);
}

// ============================================================================
// Single-Client Message Tests (ToClient methods)
// ============================================================================

TEST_F(NetworkApiTest, SpawnEntityToClient) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> spawnReceived{false};
    client::EntitySpawnEvent receivedSpawn{};

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    client_->onEntitySpawn([&](client::EntitySpawnEvent event) {
        receivedSpawn = event;
        spawnReceived = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    // Allow more time on slower CI/Windows environments
    ASSERT_TRUE(waitFor(clientConnected, std::chrono::milliseconds(3000)));

    auto clients = server_->getConnectedClients();
    ASSERT_EQ(clients.size(), 1u);

    // Use ToClient method
    server_->spawnEntityToClient(clients[0], 999, network::EntityType::Bydos,
                                 0, 100.0f, 200.0f);

    ASSERT_TRUE(waitFor(spawnReceived, std::chrono::milliseconds(3000)));
    EXPECT_EQ(receivedSpawn.entityId, 999u);
    EXPECT_FLOAT_EQ(receivedSpawn.x, 100.0f);
    EXPECT_FLOAT_EQ(receivedSpawn.y, 200.0f);
}

TEST_F(NetworkApiTest, MoveEntityToClient) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> moveReceived{false};
    client::EntityMoveEvent receivedMove{};

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    client_->onEntityMove([&](client::EntityMoveEvent event) {
        receivedMove = event;
        moveReceived = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    auto clients = server_->getConnectedClients();
    ASSERT_EQ(clients.size(), 1u);

    server_->moveEntityToClient(clients[0], 888, 50.0f, 60.0f, 1.0f, 2.0f);

    for (int i = 0; i < 100 && !moveReceived; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(moveReceived.load());
    EXPECT_EQ(receivedMove.entityId, 888u);
    EXPECT_FLOAT_EQ(receivedMove.x, 50.0f);
    EXPECT_FLOAT_EQ(receivedMove.y, 60.0f);
}

TEST_F(NetworkApiTest, DestroyEntityToClient) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> destroyReceived{false};
    std::uint32_t receivedEntityId = 0;

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    client_->onEntityDestroy([&](std::uint32_t entityId) {
        receivedEntityId = entityId;
        destroyReceived = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    auto clients = server_->getConnectedClients();
    ASSERT_EQ(clients.size(), 1u);

    server_->destroyEntityToClient(clients[0], 777);

    for (int i = 0; i < 100 && !destroyReceived; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(destroyReceived.load());
    EXPECT_EQ(receivedEntityId, 777u);
}

TEST_F(NetworkApiTest, UpdateGameStateToClient) {
    std::atomic<bool> clientConnected{false};
    std::atomic<bool> stateReceived{false};
    client::GameStateEvent receivedEvent{};

    client_->onConnected([&](std::uint32_t userId) {
        (void)userId;
        clientConnected = true;
    });

    client_->onGameStateChange([&](client::GameStateEvent event) {
        receivedEvent = event;
        stateReceived = true;
    });

    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    for (int i = 0; i < 100 && !clientConnected; ++i) {
        pollBoth(10ms);
    }
    ASSERT_TRUE(clientConnected.load());

    auto clients = server_->getConnectedClients();
    ASSERT_EQ(clients.size(), 1u);

    server_->updateGameStateToClient(clients[0], network::GameState::GameOver);

    for (int i = 0; i < 100 && !stateReceived; ++i) {
        pollBoth(10ms);
    }

    EXPECT_TRUE(stateReceived.load());
    EXPECT_EQ(receivedEvent.state, network::GameState::GameOver);
}

TEST_F(NetworkApiTest, ToClientMethodsWithInvalidUser) {
    EXPECT_TRUE(server_->start(TEST_PORT));

    // These should not crash when user doesn't exist
    EXPECT_NO_THROW(server_->spawnEntityToClient(
        99999, 1, network::EntityType::Player, 0, 0.0f, 0.0f));
    EXPECT_NO_THROW(
        server_->moveEntityToClient(99999, 1, 0.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_NO_THROW(server_->destroyEntityToClient(99999, 1));
    EXPECT_NO_THROW(
        server_->updateGameStateToClient(99999, network::GameState::Lobby));
}

// ============================================================================
// Server Edge Cases
// ============================================================================

TEST_F(NetworkApiTest, ServerStopWithoutStart) {
    // Should not crash
    EXPECT_NO_THROW(server_->stop());
    EXPECT_FALSE(server_->isRunning());
}

TEST_F(NetworkApiTest, ServerPollWithoutStart) {
    // Should not crash
    EXPECT_NO_THROW(server_->poll());
}

TEST_F(NetworkApiTest, ServerBroadcastWithoutClients) {
    EXPECT_TRUE(server_->start(TEST_PORT));

    // Broadcasting to no clients should not crash
    EXPECT_NO_THROW(
        server_->spawnEntity(1, network::EntityType::Bydos, 0, 0.0f, 0.0f));
    EXPECT_NO_THROW(server_->moveEntity(1, 0.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_NO_THROW(server_->destroyEntity(1));
    EXPECT_NO_THROW(server_->updateGameState(network::GameState::Running));
}

// ============================================================================
// Client Edge Cases
// ============================================================================

TEST_F(NetworkApiTest, ClientPollWithoutConnect) {
    // Should not crash
    EXPECT_NO_THROW(client_->poll());
}

TEST_F(NetworkApiTest, ClientDisconnectWithoutConnect) {
    // Should not crash
    EXPECT_NO_THROW(client_->disconnect());
    EXPECT_FALSE(client_->isConnected());
}

TEST_F(NetworkApiTest, ClientDoubleDisconnect) {
    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    std::atomic<bool> connected{false};
    client_->onConnected([&](std::uint32_t) { connected = true; });

    for (int i = 0; i < 100 && !connected; ++i) {
        pollBoth(10ms);
    }

    client_->disconnect();
    pollBoth(50ms);

    // Second disconnect should not crash
    EXPECT_NO_THROW(client_->disconnect());
}

TEST_F(NetworkApiTest, ClientConnectWhileConnected) {
    EXPECT_TRUE(server_->start(TEST_PORT));
    EXPECT_TRUE(client_->connect("127.0.0.1", TEST_PORT));

    std::atomic<bool> connected{false};
    client_->onConnected([&](std::uint32_t) { connected = true; });

    for (int i = 0; i < 100 && !connected; ++i) {
        pollBoth(10ms);
    }

    // Second connect should fail or be ignored
    bool secondConnect = client_->connect("127.0.0.1", TEST_PORT);
    EXPECT_FALSE(secondConnect);
}

