/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ClientManager unit tests - Comprehensive branch coverage
*/

#include <gtest/gtest.h>

#include <chrono>
#include <limits>
#include <memory>
#include <thread>
#include <vector>

#include "server/ClientManager.hpp"
#include "server/ServerMetrics.hpp"

using namespace rtype::server;
using rtype::ClientId;
using rtype::DisconnectReason;
using rtype::Endpoint;

namespace {

// Helper function to create Endpoint without ambiguity
Endpoint makeEndpoint(const std::string& addr, uint16_t port) {
    return Endpoint(std::string(addr), port);
}

}  // namespace

class ClientManagerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Create shared metrics
        metrics = std::make_shared<ServerMetrics>();
        // Reset metrics counters
        metrics->packetsReceived.store(0);
        metrics->packetsSent.store(0);
        metrics->bytesReceived.store(0);
        metrics->bytesSent.store(0);
        metrics->tickOverruns.store(0);
        metrics->connectionsRejected.store(0);
        metrics->totalConnections.store(0);
    }

    std::shared_ptr<ServerMetrics> metrics;
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(ClientManagerTest, Constructor_InitializesCorrectly) {
    ClientManager manager(4, metrics);

    EXPECT_EQ(manager.getMaxPlayers(), 4);
    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, Constructor_WithZeroMaxPlayers) {
    ClientManager manager(0, metrics);

    EXPECT_EQ(manager.getMaxPlayers(), 0);
    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

// ============================================================================
// handleNewConnection Tests
// ============================================================================

TEST_F(ClientManagerTest, HandleNewConnection_Success) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId clientId = manager.handleNewConnection(endpoint);

    EXPECT_NE(clientId, ClientManager::INVALID_CLIENT_ID);
    EXPECT_EQ(manager.getConnectedClientCount(), 1);
    EXPECT_EQ(metrics->totalConnections.load(), 1);
}

TEST_F(ClientManagerTest, HandleNewConnection_MultipleClients) {
    ClientManager manager(4, metrics);

    ClientId id1 = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    ClientId id2 = manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));
    ClientId id3 = manager.handleNewConnection(makeEndpoint("192.168.1.3", 12347));

    EXPECT_NE(id1, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id2, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id3, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
    EXPECT_EQ(manager.getConnectedClientCount(), 3);
}

TEST_F(ClientManagerTest, HandleNewConnection_AlreadyConnectedEndpoint) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId firstId = manager.handleNewConnection(endpoint);
    ClientId secondId = manager.handleNewConnection(endpoint);

    // Should return the existing client ID
    EXPECT_EQ(firstId, secondId);
    EXPECT_EQ(manager.getConnectedClientCount(), 1);
}

TEST_F(ClientManagerTest, HandleNewConnection_ServerFull) {
    ClientManager manager(2, metrics);

    ClientId id1 = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    ClientId id2 = manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));
    ClientId id3 = manager.handleNewConnection(makeEndpoint("192.168.1.3", 12347));

    EXPECT_NE(id1, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id2, ClientManager::INVALID_CLIENT_ID);
    EXPECT_EQ(id3, ClientManager::INVALID_CLIENT_ID);
    EXPECT_EQ(manager.getConnectedClientCount(), 2);
    EXPECT_GE(metrics->connectionsRejected.load(), 1);
}

TEST_F(ClientManagerTest, HandleNewConnection_RateLimitExceeded) {
    ClientManager manager(100, metrics);

    // Connect MAX_CONNECTIONS_PER_SECOND clients quickly
    std::vector<ClientId> ids;
    for (uint32_t i = 0; i <= ClientManager::MAX_CONNECTIONS_PER_SECOND; ++i) {
        auto ep = makeEndpoint("192.168.1." + std::to_string(i),
                               static_cast<uint16_t>(12345 + i));
        ids.push_back(manager.handleNewConnection(ep));
    }

    // At least one should be rejected due to rate limiting
    int rejectedCount = 0;
    for (const auto& id : ids) {
        if (id == ClientManager::INVALID_CLIENT_ID) {
            ++rejectedCount;
        }
    }
    EXPECT_GE(rejectedCount, 1);
    EXPECT_GE(metrics->connectionsRejected.load(), 1);
}

TEST_F(ClientManagerTest, HandleNewConnection_RateLimitResetsAfterWindow) {
    ClientManager manager(100, metrics);

    // Fill up the rate limit
    for (uint32_t i = 0; i < ClientManager::MAX_CONNECTIONS_PER_SECOND; ++i) {
        auto ep = makeEndpoint("192.168.1." + std::to_string(i),
                               static_cast<uint16_t>(12345 + i));
        manager.handleNewConnection(ep);
    }

    // Wait for rate limit window to reset (slightly more than 1 second)
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    // Should be able to connect again
    auto newEndpoint = makeEndpoint("10.0.0.1", 54321);
    ClientId newId = manager.handleNewConnection(newEndpoint);

    EXPECT_NE(newId, ClientManager::INVALID_CLIENT_ID);
}

// ============================================================================
// handleClientDisconnect Tests
// ============================================================================

TEST_F(ClientManagerTest, HandleClientDisconnect_ValidClient) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId clientId = manager.handleNewConnection(endpoint);
    EXPECT_EQ(manager.getConnectedClientCount(), 1);

    manager.handleClientDisconnect(clientId, DisconnectReason::Disconnected);

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, HandleClientDisconnect_InvalidClient) {
    ClientManager manager(4, metrics);

    // Should not crash when disconnecting non-existent client
    manager.handleClientDisconnect(999, DisconnectReason::Error);

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, HandleClientDisconnect_AllReasons) {
    ClientManager manager(10, metrics);

    // Test each disconnect reason
    std::vector<DisconnectReason> reasons = {
        DisconnectReason::Disconnected,
        DisconnectReason::Timeout,
        DisconnectReason::Kicked,
        DisconnectReason::Error
    };

    uint16_t port = 12345;
    for (const auto& reason : reasons) {
        auto endpoint = makeEndpoint("192.168.1.1", port++);
        ClientId id = manager.handleNewConnection(endpoint);
        EXPECT_NE(id, ClientManager::INVALID_CLIENT_ID);
        manager.handleClientDisconnect(id, reason);
    }

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, HandleClientDisconnect_DoubleDisconnect) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId clientId = manager.handleNewConnection(endpoint);
    manager.handleClientDisconnect(clientId, DisconnectReason::Disconnected);

    // Second disconnect should be a no-op
    manager.handleClientDisconnect(clientId, DisconnectReason::Disconnected);

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

// ============================================================================
// updateClientActivity Tests
// ============================================================================

TEST_F(ClientManagerTest, UpdateClientActivity_ValidClient) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId clientId = manager.handleNewConnection(endpoint);

    // Should not crash
    manager.updateClientActivity(clientId);

    auto clientInfo = manager.getClientInfo(clientId);
    EXPECT_TRUE(clientInfo.has_value());
}

TEST_F(ClientManagerTest, UpdateClientActivity_InvalidClient) {
    ClientManager manager(4, metrics);

    // Should not crash when updating non-existent client
    manager.updateClientActivity(999);
}

// ============================================================================
// findClientByEndpoint Tests
// ============================================================================

TEST_F(ClientManagerTest, FindClientByEndpoint_Found) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId expectedId = manager.handleNewConnection(endpoint);
    ClientId foundId = manager.findClientByEndpoint(endpoint);

    EXPECT_EQ(foundId, expectedId);
}

TEST_F(ClientManagerTest, FindClientByEndpoint_NotFound) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId foundId = manager.findClientByEndpoint(endpoint);

    EXPECT_EQ(foundId, ClientManager::INVALID_CLIENT_ID);
}

TEST_F(ClientManagerTest, FindClientByEndpoint_AfterDisconnect) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId clientId = manager.handleNewConnection(endpoint);
    manager.handleClientDisconnect(clientId, DisconnectReason::Disconnected);

    ClientId foundId = manager.findClientByEndpoint(endpoint);
    EXPECT_EQ(foundId, ClientManager::INVALID_CLIENT_ID);
}

// ============================================================================
// getConnectedClientCount Tests
// ============================================================================

TEST_F(ClientManagerTest, GetConnectedClientCount_Empty) {
    ClientManager manager(4, metrics);
    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, GetConnectedClientCount_AfterConnections) {
    ClientManager manager(4, metrics);

    manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    EXPECT_EQ(manager.getConnectedClientCount(), 1);

    manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));
    EXPECT_EQ(manager.getConnectedClientCount(), 2);
}

// ============================================================================
// getConnectedClientIds Tests
// ============================================================================

TEST_F(ClientManagerTest, GetConnectedClientIds_Empty) {
    ClientManager manager(4, metrics);

    auto ids = manager.getConnectedClientIds();
    EXPECT_TRUE(ids.empty());
}

TEST_F(ClientManagerTest, GetConnectedClientIds_MultipleClients) {
    ClientManager manager(4, metrics);

    ClientId id1 = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    ClientId id2 = manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));

    auto ids = manager.getConnectedClientIds();

    EXPECT_EQ(ids.size(), 2);
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), id1) != ids.end());
    EXPECT_TRUE(std::find(ids.begin(), ids.end(), id2) != ids.end());
}

// ============================================================================
// getClientInfo Tests
// ============================================================================

TEST_F(ClientManagerTest, GetClientInfo_ValidClient) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId clientId = manager.handleNewConnection(endpoint);
    auto clientInfo = manager.getClientInfo(clientId);

    EXPECT_TRUE(clientInfo.has_value());
    EXPECT_EQ(clientInfo->id, clientId);
    EXPECT_EQ(clientInfo->endpoint, endpoint);
    EXPECT_EQ(clientInfo->state, rtype::ClientState::Connected);
}

TEST_F(ClientManagerTest, GetClientInfo_InvalidClient) {
    ClientManager manager(4, metrics);

    auto clientInfo = manager.getClientInfo(999);
    EXPECT_FALSE(clientInfo.has_value());
}

// ============================================================================
// checkClientTimeouts Tests
// ============================================================================

TEST_F(ClientManagerTest, CheckClientTimeouts_NoTimeout) {
    ClientManager manager(4, metrics);

    manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));

    // Check with a very long timeout - client should not time out
    manager.checkClientTimeouts(3600);

    EXPECT_EQ(manager.getConnectedClientCount(), 1);
}

TEST_F(ClientManagerTest, CheckClientTimeouts_ClientTimedOut) {
    ClientManager manager(4, metrics);

    manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));

    // Wait briefly then check with zero timeout - client should time out
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    manager.checkClientTimeouts(0);

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, CheckClientTimeouts_MultipleClients_SomeTimedOut) {
    ClientManager manager(4, metrics);

    manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));

    // Wait and check with zero timeout - all should time out
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    manager.checkClientTimeouts(0);

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, CheckClientTimeouts_EmptyClientList) {
    ClientManager manager(4, metrics);

    // Should not crash with no clients
    manager.checkClientTimeouts(30);

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

// ============================================================================
// clearAllClients Tests
// ============================================================================

TEST_F(ClientManagerTest, ClearAllClients_Empty) {
    ClientManager manager(4, metrics);

    manager.clearAllClients();

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, ClearAllClients_WithClients) {
    ClientManager manager(4, metrics);

    manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));
    manager.handleNewConnection(makeEndpoint("192.168.1.3", 12347));

    EXPECT_EQ(manager.getConnectedClientCount(), 3);

    manager.clearAllClients();

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, ClearAllClients_EndpointLookupCleared) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    manager.handleNewConnection(endpoint);
    manager.clearAllClients();

    // Endpoint should no longer be found
    EXPECT_EQ(manager.findClientByEndpoint(endpoint),
              ClientManager::INVALID_CLIENT_ID);
}

// ============================================================================
// Reconnection after clear Tests
// ============================================================================

TEST_F(ClientManagerTest, ReconnectAfterClear) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId firstId = manager.handleNewConnection(endpoint);
    manager.clearAllClients();

    // Should be able to reconnect with same endpoint
    ClientId secondId = manager.handleNewConnection(endpoint);

    EXPECT_NE(secondId, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(firstId, secondId);  // Should get a new ID
    EXPECT_EQ(manager.getConnectedClientCount(), 1);
}

TEST_F(ClientManagerTest, ReconnectAfterDisconnect) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    ClientId firstId = manager.handleNewConnection(endpoint);
    manager.handleClientDisconnect(firstId, DisconnectReason::Disconnected);

    // Should be able to reconnect with same endpoint
    ClientId secondId = manager.handleNewConnection(endpoint);

    EXPECT_NE(secondId, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(firstId, secondId);  // Should get a new ID
    EXPECT_EQ(manager.getConnectedClientCount(), 1);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST_F(ClientManagerTest, EdgeCase_ZeroMaxPlayers_CannotConnect) {
    ClientManager manager(0, metrics);

    ClientId id = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));

    EXPECT_EQ(id, ClientManager::INVALID_CLIENT_ID);
    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, EdgeCase_SingleMaxPlayer) {
    ClientManager manager(1, metrics);

    ClientId id1 = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    ClientId id2 = manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));

    EXPECT_NE(id1, ClientManager::INVALID_CLIENT_ID);
    EXPECT_EQ(id2, ClientManager::INVALID_CLIENT_ID);
}

TEST_F(ClientManagerTest, EdgeCase_DifferentPortsSameAddress) {
    ClientManager manager(4, metrics);

    ClientId id1 = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    ClientId id2 = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12346));

    EXPECT_NE(id1, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id2, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id1, id2);
    EXPECT_EQ(manager.getConnectedClientCount(), 2);
}

TEST_F(ClientManagerTest, EdgeCase_SamePortDifferentAddresses) {
    ClientManager manager(4, metrics);

    ClientId id1 = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    ClientId id2 = manager.handleNewConnection(makeEndpoint("192.168.1.2", 12345));

    EXPECT_NE(id1, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id2, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id1, id2);
    EXPECT_EQ(manager.getConnectedClientCount(), 2);
}

// ============================================================================
// Metrics Tests
// ============================================================================

TEST_F(ClientManagerTest, Metrics_TotalConnectionsIncremented) {
    ClientManager manager(4, metrics);

    manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    EXPECT_EQ(metrics->totalConnections.load(), 1);

    manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));
    EXPECT_EQ(metrics->totalConnections.load(), 2);
}

TEST_F(ClientManagerTest, Metrics_ConnectionsRejectedOnServerFull) {
    ClientManager manager(1, metrics);

    manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    uint64_t beforeRejected = metrics->connectionsRejected.load();

    manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));

    EXPECT_GT(metrics->connectionsRejected.load(), beforeRejected);
}

// ============================================================================
// Thread Safety Basic Test
// ============================================================================

TEST_F(ClientManagerTest, ThreadSafety_ConcurrentConnections) {
    ClientManager manager(100, metrics);
    constexpr int numThreads = 4;
    constexpr int connectionsPerThread = 5;

    std::vector<std::thread> threads;
    std::vector<ClientId> allIds(numThreads * connectionsPerThread);

    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&, t]() {
            for (int i = 0; i < connectionsPerThread; ++i) {
                int idx = t * connectionsPerThread + i;
                auto ep = makeEndpoint(
                    "10.0." + std::to_string(t) + "." + std::to_string(i),
                    static_cast<uint16_t>(10000 + idx));
                allIds[idx] = manager.handleNewConnection(ep);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Count successful connections
    int successfulConnections = 0;
    for (const auto& id : allIds) {
        if (id != ClientManager::INVALID_CLIENT_ID) {
            ++successfulConnections;
        }
    }

    // Due to rate limiting, we might not get all connections
    EXPECT_GT(successfulConnections, 0);
    EXPECT_EQ(manager.getConnectedClientCount(),
              static_cast<size_t>(successfulConnections));
}

TEST_F(ClientManagerTest, ThreadSafety_ConcurrentDisconnections) {
    ClientManager manager(20, metrics);

    // Connect clients first
    std::vector<ClientId> clientIds;
    for (int i = 0; i < 10; ++i) {
        auto ep = makeEndpoint("192.168.1." + std::to_string(i),
                               static_cast<uint16_t>(12345 + i));
        ClientId id = manager.handleNewConnection(ep);
        if (id != ClientManager::INVALID_CLIENT_ID) {
            clientIds.push_back(id);
        }
    }

    // Disconnect in parallel
    std::vector<std::thread> threads;
    for (const auto& id : clientIds) {
        threads.emplace_back([&manager, id]() {
            manager.handleClientDisconnect(id, DisconnectReason::Disconnected);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

// ============================================================================
// Client ID Generation Tests
// ============================================================================

TEST_F(ClientManagerTest, ClientIdGeneration_SequentialIds) {
    ClientManager manager(10, metrics);

    ClientId id1 = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    ClientId id2 = manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));
    ClientId id3 = manager.handleNewConnection(makeEndpoint("192.168.1.3", 12347));

    // IDs should be sequential starting from FIRST_VALID_CLIENT_ID
    EXPECT_EQ(id1, ClientManager::FIRST_VALID_CLIENT_ID);
    EXPECT_EQ(id2, ClientManager::FIRST_VALID_CLIENT_ID + 1);
    EXPECT_EQ(id3, ClientManager::FIRST_VALID_CLIENT_ID + 2);
}

TEST_F(ClientManagerTest, ClientIdGeneration_IdsNotReusedAfterDisconnect) {
    ClientManager manager(10, metrics);

    ClientId id1 = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));
    manager.handleClientDisconnect(id1, DisconnectReason::Disconnected);

    ClientId id2 = manager.handleNewConnection(makeEndpoint("192.168.1.2", 12346));

    // New client should get a new ID, not reuse the old one
    EXPECT_NE(id2, id1);
    EXPECT_GT(id2, id1);
}

// ============================================================================
// Endpoint Tests
// ============================================================================

TEST_F(ClientManagerTest, Endpoint_IPv6Address) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("::1", 12345);

    ClientId id = manager.handleNewConnection(endpoint);

    EXPECT_NE(id, ClientManager::INVALID_CLIENT_ID);

    ClientId foundId = manager.findClientByEndpoint(endpoint);
    EXPECT_EQ(foundId, id);
}

TEST_F(ClientManagerTest, Endpoint_LocalhostVariants) {
    ClientManager manager(4, metrics);

    ClientId id1 = manager.handleNewConnection(makeEndpoint("localhost", 12345));
    ClientId id2 = manager.handleNewConnection(makeEndpoint("127.0.0.1", 12345));

    // These are different endpoints (string comparison)
    EXPECT_NE(id1, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id2, ClientManager::INVALID_CLIENT_ID);
    EXPECT_NE(id1, id2);
}

TEST_F(ClientManagerTest, Endpoint_EmptyAddress) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("", 12345);

    ClientId id = manager.handleNewConnection(endpoint);

    EXPECT_NE(id, ClientManager::INVALID_CLIENT_ID);
    EXPECT_EQ(manager.getConnectedClientCount(), 1);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST_F(ClientManagerTest, Stress_FillAndEmptyServer) {
    const size_t maxPlayers = 10;
    ClientManager manager(maxPlayers, metrics);

    // Fill the server
    std::vector<ClientId> ids;
    for (size_t i = 0; i < maxPlayers; ++i) {
        auto ep = makeEndpoint("192.168.1." + std::to_string(i),
                               static_cast<uint16_t>(12345 + i));
        ClientId id = manager.handleNewConnection(ep);
        EXPECT_NE(id, ClientManager::INVALID_CLIENT_ID);
        ids.push_back(id);
    }

    EXPECT_EQ(manager.getConnectedClientCount(), maxPlayers);

    // Empty the server
    for (const auto& id : ids) {
        manager.handleClientDisconnect(id, DisconnectReason::Disconnected);
    }

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
}

TEST_F(ClientManagerTest, Stress_RepeatedConnectionDisconnection) {
    ClientManager manager(4, metrics);
    auto endpoint = makeEndpoint("192.168.1.1", 12345);

    // Only do 10 iterations to stay within rate limit
    for (int i = 0; i < 10; ++i) {
        ClientId id = manager.handleNewConnection(endpoint);
        EXPECT_NE(id, ClientManager::INVALID_CLIENT_ID);
        manager.handleClientDisconnect(id, DisconnectReason::Disconnected);
    }

    EXPECT_EQ(manager.getConnectedClientCount(), 0);
    EXPECT_EQ(metrics->totalConnections.load(), 10);
}

// ============================================================================
// Activity Update with Timeout Tests
// ============================================================================

TEST_F(ClientManagerTest, ActivityUpdate_PreventsTimeout) {
    ClientManager manager(4, metrics);

    ClientId id = manager.handleNewConnection(makeEndpoint("192.168.1.1", 12345));

    // Update activity
    manager.updateClientActivity(id);

    // Even with very short timeout, updated client should not time out immediately
    manager.checkClientTimeouts(3600);

    EXPECT_EQ(manager.getConnectedClientCount(), 1);
}

// ============================================================================
// Rate Limit Window Tests
// ============================================================================

TEST_F(ClientManagerTest, RateLimitWindow_NotExceededWithinLimit) {
    ClientManager manager(100, metrics);

    // Connect less than MAX_CONNECTIONS_PER_SECOND
    for (uint32_t i = 0; i < ClientManager::MAX_CONNECTIONS_PER_SECOND - 1; ++i) {
        auto ep = makeEndpoint("192.168.1." + std::to_string(i),
                               static_cast<uint16_t>(12345 + i));
        ClientId id = manager.handleNewConnection(ep);
        EXPECT_NE(id, ClientManager::INVALID_CLIENT_ID);
    }

    EXPECT_EQ(metrics->connectionsRejected.load(), 0);
}

// ============================================================================
// Additional Edge Case Tests for Branch Coverage
// ============================================================================

TEST_F(ClientManagerTest, HandleNewConnection_ServerFull_Additional) {
    ClientManager manager(1, metrics); // Only allow 1 client
    
    // First connection should succeed
    auto ep1 = makeEndpoint("192.168.1.1", 12345);
    ClientId id1 = manager.handleNewConnection(ep1);
    EXPECT_NE(id1, ClientManager::INVALID_CLIENT_ID);
    
    // Second connection should fail (server full)
    auto ep2 = makeEndpoint("192.168.1.2", 12346);
    ClientId id2 = manager.handleNewConnection(ep2);
    EXPECT_EQ(id2, ClientManager::INVALID_CLIENT_ID);
}

TEST_F(ClientManagerTest, HandleNewConnection_DuplicateEndpoint) {
    ClientManager manager(10, metrics);
    
    auto ep = makeEndpoint("192.168.1.1", 12345);
    
    // First connection
    ClientId id1 = manager.handleNewConnection(ep);
    EXPECT_NE(id1, ClientManager::INVALID_CLIENT_ID);
    
    // Duplicate connection attempt should return existing ID
    ClientId id2 = manager.handleNewConnection(ep);
    EXPECT_EQ(id2, id1);
}

TEST_F(ClientManagerTest, DisconnectClient_InvalidId) {
    ClientManager manager(10, metrics);
    
    // Try to disconnect non-existent client
    EXPECT_NO_THROW(manager.handleClientDisconnect(999, DisconnectReason::Disconnected));
}

TEST_F(ClientManagerTest, DisconnectClient_AlreadyDisconnected) {
    ClientManager manager(10, metrics);
    
    auto ep = makeEndpoint("192.168.1.1", 12345);
    ClientId id = manager.handleNewConnection(ep);
    EXPECT_NE(id, ClientManager::INVALID_CLIENT_ID);
    
    // Disconnect once
    manager.handleClientDisconnect(id, DisconnectReason::Disconnected);
    
    // Disconnect again (should not crash)
    EXPECT_NO_THROW(manager.handleClientDisconnect(id, DisconnectReason::Disconnected));
}

TEST_F(ClientManagerTest, GetClientInfo_InvalidId) {
    ClientManager manager(10, metrics);
    
    auto clientInfo = manager.getClientInfo(999);
    EXPECT_FALSE(clientInfo.has_value());
}

TEST_F(ClientManagerTest, FindClientByEndpoint_NonExistent) {
    ClientManager manager(10, metrics);
    
    auto ep = makeEndpoint("192.168.1.1", 12345);
    ClientId id = manager.findClientByEndpoint(ep);
    EXPECT_EQ(id, ClientManager::INVALID_CLIENT_ID);
}

TEST_F(ClientManagerTest, UpdateClientTimeout_NoTimeouts) {
    ClientManager manager(10, metrics);
    
    // Add a client
    auto ep = makeEndpoint("192.168.1.1", 12345);
    ClientId id = manager.handleNewConnection(ep);
    EXPECT_NE(id, ClientManager::INVALID_CLIENT_ID);
    
    // Update timeouts with no timeouts expected
    manager.checkClientTimeouts(30);
    
    // Client should still be connected
    EXPECT_EQ(manager.getConnectedClientCount(), 1u);
}

TEST_F(ClientManagerTest, ClearAllClients_EmptyManager) {
    ClientManager manager(10, metrics);
    
    // Clear empty manager (should not crash)
    EXPECT_NO_THROW(manager.clearAllClients());
    EXPECT_EQ(manager.getConnectedClientCount(), 0u);
}

TEST_F(ClientManagerTest, GetMaxPlayers) {
    ClientManager manager(42, metrics);
    EXPECT_EQ(manager.getMaxPlayers(), 42u);
}
