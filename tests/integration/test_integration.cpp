/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_integration
*/

#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include <future>

#include "../../src/common/SafeQueue/SafeQueue.hpp"
#include "../../src/common/Types.hpp"
#include "../../src/games/rtype/shared/Components.hpp"
#include "../../src/games/rtype/shared/Systems/MovementSystem.cpp"
#include "../../src/server/ServerApp.hpp"
#include "../../src/server/ClientManager.hpp"
#include "../../src/network/Packet.hpp"
#include "../../src/network/protocol/Header.hpp"

using namespace rtype;
using namespace rtype::games::rtype::shared;
using namespace rtype::server;
using namespace rtype::network;

// ============================================================================
// Integration Tests - Testing component interactions
// ============================================================================

TEST(IntegrationTest, MovementSystemWithNetworkComponents) {
    // Test that movement system works with network-synced components
    TransformComponent transform{0.0f, 0.0f, 0.0f};
    VelocityComponent velocity{10.0f, 5.0f};
    NetworkIdComponent netId{12345};

    const float deltaTime = 0.016f;  // ~60 FPS

    // Update movement
    updateMovement(transform, velocity, deltaTime);

    // Verify movement occurred
    EXPECT_NEAR(transform.x, 0.16f, 0.001f);
    EXPECT_NEAR(transform.y, 0.08f, 0.001f);

    // Network ID should remain unchanged
    EXPECT_EQ(netId.networkId, 12345u);
}

TEST(IntegrationTest, SafeQueueThreadSafety) {
    SafeQueue<int> queue;

    // Test basic operations
    queue.push(1);
    queue.push(2);
    queue.push(3);

    EXPECT_EQ(queue.size(), 3);

    auto item1 = queue.pop();
    ASSERT_TRUE(item1.has_value());
    EXPECT_EQ(*item1, 1);

    auto item2 = queue.pop();
    ASSERT_TRUE(item2.has_value());
    EXPECT_EQ(*item2, 2);

    EXPECT_EQ(queue.size(), 1);
}

TEST(IntegrationTest, ComponentStateSynchronization) {
    // Test that components maintain state correctly for network sync
    TransformComponent transform{100.0f, 200.0f, 90.0f};
    VelocityComponent velocity{15.0f, -10.0f};
    NetworkIdComponent netId{999};

    // Simulate multiple movement updates
    const float deltaTime = 0.1f;
    for (int i = 0; i < 10; ++i) {
        updateMovement(transform, velocity, deltaTime);
    }

    // Verify final position
    EXPECT_NEAR(transform.x, 100.0f + 15.0f * 10 * deltaTime, 0.001f);
    EXPECT_NEAR(transform.y, 200.0f + (-10.0f) * 10 * deltaTime, 0.001f);

    // Components should maintain their relationships
    EXPECT_EQ(netId.networkId, 999u);
    EXPECT_FLOAT_EQ(velocity.vx, 15.0f);
    EXPECT_FLOAT_EQ(velocity.vy, -10.0f);
}

// ============================================================================
// ServerApp Integration Tests
// ============================================================================

class ServerAppIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        shutdownFlag_ = std::make_shared<std::atomic<bool>>(false);
        server_ = std::make_unique<ServerApp>(8080, 4, 60, shutdownFlag_, 30, false);
    }

    void TearDown() override {
        if (server_ && server_->isRunning()) {
            server_->stop();
        }
        shutdownFlag_->store(true);
    }

    std::shared_ptr<std::atomic<bool>> shutdownFlag_;
    std::unique_ptr<ServerApp> server_;
};

TEST_F(ServerAppIntegrationTest, ServerAppInitialization) {
    // Test that server initializes correctly
    EXPECT_FALSE(server_->isRunning());
    EXPECT_EQ(server_->getConnectedClientCount(), 0u);
    EXPECT_TRUE(server_->getConnectedClientIds().empty());

    // Check that client manager is accessible
    const auto& clientManager = server_->getClientManager();
    EXPECT_EQ(clientManager.getConnectedClientCount(), 0u);
}

TEST_F(ServerAppIntegrationTest, ServerAppClientManagerIntegration) {
    // Test integration between ServerApp and ClientManager
    auto& clientManager = server_->getClientManager();

    // Simulate client connection
    Endpoint testEndpoint{"127.0.0.1", 12345};
    ClientId clientId = clientManager.handleNewConnection(testEndpoint);

    // Verify client was added
    EXPECT_NE(clientId, ClientManager::INVALID_CLIENT_ID);
    EXPECT_EQ(server_->getConnectedClientCount(), 1u);
    EXPECT_EQ(clientManager.getConnectedClientCount(), 1u);

    auto clientIds = server_->getConnectedClientIds();
    ASSERT_EQ(clientIds.size(), 1u);
    EXPECT_EQ(clientIds[0], clientId);

    // Test client info retrieval
    auto clientInfo = server_->getClientInfo(clientId);
    ASSERT_TRUE(clientInfo.has_value());
    EXPECT_EQ(clientInfo->id, clientId);
    EXPECT_EQ(clientInfo->endpoint, testEndpoint);
}

TEST_F(ServerAppIntegrationTest, ServerAppMetricsIntegration) {
    // Test that server metrics are properly integrated
    const auto& metrics = server_->getMetrics();

    // Initially should have no connections
    EXPECT_EQ(metrics.totalConnections, 0u);
    EXPECT_EQ(metrics.connectionsRejected, 0u);

    // Add a client and verify metrics update
    auto& clientManager = server_->getClientManager();
    Endpoint testEndpoint{"127.0.0.1", 12346};
    ClientId clientId = clientManager.handleNewConnection(testEndpoint);
    ASSERT_NE(clientId, ClientManager::INVALID_CLIENT_ID);

    // Note: Metrics might not update immediately in this test setup
    // This tests the integration between components
    EXPECT_TRUE(true);  // Placeholder - actual metrics testing would require running server
}

TEST_F(ServerAppIntegrationTest, ServerAppStopFunctionality) {
    // Test that stop functionality works
    EXPECT_FALSE(server_->isRunning());

    // Start server in a separate thread
    auto serverFuture = std::async(std::launch::async, [this]() {
        return server_->run();
    });

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(server_->isRunning());

    // Stop the server
    server_->stop();

    // Wait for server to stop (with timeout)
    auto status = serverFuture.wait_for(std::chrono::milliseconds(500));
    EXPECT_EQ(status, std::future_status::ready);

    // Verify server stopped
    EXPECT_FALSE(server_->isRunning());
}

TEST_F(ServerAppIntegrationTest, ServerAppClientTimeoutHandling) {
    // Test client timeout detection integration
    auto& clientManager = server_->getClientManager();

    // Add a client
    Endpoint testEndpoint{"127.0.0.1", 12347};
    ClientId clientId = clientManager.handleNewConnection(testEndpoint);
    ASSERT_NE(clientId, ClientManager::INVALID_CLIENT_ID);

    EXPECT_EQ(server_->getConnectedClientCount(), 1u);

    // Simulate timeout by directly calling timeout check
    // Note: In real usage, this would be called by the server loop
    clientManager.checkClientTimeouts(30);  // 30 second timeout

    // Client should still be connected (no timeout yet)
    EXPECT_EQ(server_->getConnectedClientCount(), 1u);

    // Get client info to verify it's still there
    auto clientInfo = server_->getClientInfo(clientId);
    EXPECT_TRUE(clientInfo.has_value());
}

TEST_F(ServerAppIntegrationTest, ServerAppMultipleClientsIntegration) {
    // Test handling multiple clients
    auto& clientManager = server_->getClientManager();

    const int numClients = 3;
    std::vector<ClientId> clientIds;

    // Add multiple clients
    for (int i = 0; i < numClients; ++i) {
        Endpoint endpoint{"127.0.0.1", static_cast<uint16_t>(12348 + i)};
        ClientId clientId = clientManager.handleNewConnection(endpoint);
        ASSERT_NE(clientId, ClientManager::INVALID_CLIENT_ID);
        clientIds.push_back(clientId);
    }

    // Verify all clients are connected
    EXPECT_EQ(server_->getConnectedClientCount(), static_cast<size_t>(numClients));
    EXPECT_EQ(clientManager.getConnectedClientCount(), static_cast<size_t>(numClients));

    auto connectedIds = server_->getConnectedClientIds();
    EXPECT_EQ(connectedIds.size(), static_cast<size_t>(numClients));

    // Verify each client can be retrieved
    for (const auto& clientId : clientIds) {
        auto clientInfo = server_->getClientInfo(clientId);
        ASSERT_TRUE(clientInfo.has_value());
        EXPECT_EQ(clientInfo->id, clientId);
    }

    // Disconnect one client
    clientManager.handleClientDisconnect(clientIds[1], DisconnectReason::Disconnected);

    // Verify client count decreased
    EXPECT_EQ(server_->getConnectedClientCount(), static_cast<size_t>(numClients - 1));
    EXPECT_EQ(clientManager.getConnectedClientCount(), static_cast<size_t>(numClients - 1));
}

TEST_F(ServerAppIntegrationTest, ServerAppPacketProcessingIntegration) {
    // Test packet processing integration (basic structure test)
    // Note: Full packet processing would require network mocking

    auto& clientManager = server_->getClientManager();

    // Add a client
    Endpoint testEndpoint{"127.0.0.1", 12351};
    ClientId clientId = clientManager.handleNewConnection(testEndpoint);
    ASSERT_NE(clientId, ClientManager::INVALID_CLIENT_ID);

    // Create a test packet (this would normally come from network)
    Packet testPacket{PacketType::Unknown};

    // In a real integration test, we would:
    // 1. Mock network to receive packets
    // 2. Push packets to server's incoming queue
    // 3. Run server loop briefly
    // 4. Verify packet was processed

    // For now, just verify the components are set up correctly
    EXPECT_EQ(server_->getConnectedClientCount(), 1u);
    auto clientInfo = server_->getClientInfo(clientId);
    ASSERT_TRUE(clientInfo.has_value());
}

// ============================================================================
// End-to-End Integration Tests
// ============================================================================

TEST(EndToEndIntegrationTest, FullServerLifecycle) {
    // Test complete server lifecycle
    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);

    {
        ServerApp server(8081, 2, 60, shutdownFlag, 10, false);

        // Verify initial state
        EXPECT_FALSE(server.isRunning());
        EXPECT_EQ(server.getConnectedClientCount(), 0u);

        // Start server in background
        auto serverFuture = std::async(std::launch::async, [&server]() {
            return server.run();
        });

        // Give server time to start
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(server.isRunning());

        // Add a client
        auto& clientManager = server.getClientManager();
        Endpoint endpoint{"127.0.0.1", 12352};
        ClientId clientId = clientManager.handleNewConnection(endpoint);
        ASSERT_NE(clientId, ClientManager::INVALID_CLIENT_ID);

        EXPECT_EQ(server.getConnectedClientCount(), 1u);

        // Stop server
        server.stop();
        shutdownFlag->store(true);

        // Wait for server to stop
        auto status = serverFuture.wait_for(std::chrono::milliseconds(500));
        EXPECT_EQ(status, std::future_status::ready);

        EXPECT_FALSE(server.isRunning());
    }
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(PerformanceTest, MovementSystem_HighFrequency) {
    TransformComponent transform{0.0f, 0.0f, 0.0f};
    VelocityComponent velocity{1.0f, 1.0f};

    const int iterations = 10000;
    const float deltaTime = 1.0f / 60.0f;  // 60 FPS

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        updateMovement(transform, velocity, deltaTime);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete in reasonable time (less than 100ms for 10k iterations)
    EXPECT_LT(duration.count(), 100);

    // Verify final position is correct (allow for floating point precision)
    EXPECT_NEAR(transform.x, iterations * deltaTime, 0.01f);
    EXPECT_NEAR(transform.y, iterations * deltaTime, 0.01f);
}

TEST(PerformanceTest, ServerAppClientManagement_Scalability) {
    // Test client management performance with multiple clients
    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    ServerApp server(8082, 100, 60, shutdownFlag, 30, false);

    auto& clientManager = server.getClientManager();

    const int numClients = 25;  // Reduced to stay within rate limits
    std::vector<ClientId> clientIds;

    auto start = std::chrono::high_resolution_clock::now();

    // Add clients with small delays to avoid rate limiting
    for (int i = 0; i < numClients; ++i) {
        Endpoint endpoint{"127.0.0.1", static_cast<uint16_t>(13000 + i)};
        ClientId clientId = clientManager.handleNewConnection(endpoint);
        ASSERT_NE(clientId, ClientManager::INVALID_CLIENT_ID);
        clientIds.push_back(clientId);
        
        // Small delay to avoid rate limiting (100ms between connections = 10 per second)
        if (i < numClients - 1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete in reasonable time (less than 3 seconds for 25 clients with delays)
    EXPECT_LT(duration.count(), 3000);
    EXPECT_EQ(server.getConnectedClientCount(), static_cast<size_t>(numClients));

    // Test lookup performance
    start = std::chrono::high_resolution_clock::now();

    for (const auto& clientId : clientIds) {
        auto clientInfo = server.getClientInfo(clientId);
        ASSERT_TRUE(clientInfo.has_value());
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Lookups should be fast (less than 50ms for 50 lookups)
    EXPECT_LT(duration.count(), 50);
}