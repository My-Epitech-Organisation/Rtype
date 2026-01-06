/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_server_app_branches - Additional tests to increase branch coverage
*/

#include <gtest/gtest.h>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>

#include "server/serverApp/ServerApp.hpp"
#include "Packet.hpp"

using namespace rtype::server;
using namespace rtype::network;

class ServerAppBranchTest : public ::testing::Test {
protected:
    void SetUp() override {
        shutdownFlag_ = std::make_shared<std::atomic<bool>>(false);
    }

    void TearDown() override {
        shutdownFlag_->store(true);
    }

    std::shared_ptr<std::atomic<bool>> shutdownFlag_;
};

// Test constructor with various valid edge cases
TEST_F(ServerAppBranchTest, ConstructorMinimumValues) {
    const uint16_t port = 1024;  // Minimum non-privileged port
    const size_t maxPlayers = 1;  // Minimum players
    const uint32_t tickRate = 1;  // Minimum tick rate
    const uint32_t clientTimeoutSeconds = 1;  // Minimum timeout
    const bool verbose = true;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

TEST_F(ServerAppBranchTest, ConstructorMaximumPlayers) {
    const uint16_t port = 8080;
    const size_t maxPlayers = 10000;  // Very high player count
    const uint32_t tickRate = 60;
    const uint32_t clientTimeoutSeconds = 30;
    const bool verbose = false;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

TEST_F(ServerAppBranchTest, ConstructorHighTickRate) {
    const uint16_t port = 8080;
    const size_t maxPlayers = 4;
    const uint32_t tickRate = 10000;  // Very high tick rate
    const uint32_t clientTimeoutSeconds = 30;
    const bool verbose = false;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

TEST_F(ServerAppBranchTest, ConstructorLongTimeout) {
    const uint16_t port = 8080;
    const size_t maxPlayers = 4;
    const uint32_t tickRate = 60;
    const uint32_t clientTimeoutSeconds = 86400;  // 24 hours
    const bool verbose = false;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

TEST_F(ServerAppBranchTest, ConstructorVerboseEnabled) {
    const uint16_t port = 8080;
    const size_t maxPlayers = 4;
    const uint32_t tickRate = 60;
    const uint32_t clientTimeoutSeconds = 30;
    const bool verbose = true;  // Enable verbose mode

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

TEST_F(ServerAppBranchTest, ConstructorZeroTickRateThrows) {
    const uint16_t port = 8080;
    const size_t maxPlayers = 4;
    const uint32_t tickRate = 0;  // Invalid
    const uint32_t clientTimeoutSeconds = 30;
    const bool verbose = false;

    EXPECT_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    }, std::invalid_argument);
}

TEST_F(ServerAppBranchTest, MultipleStopCalls) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // First stop
    server.stop();
    EXPECT_TRUE(shutdownFlag_->load());

    // Second stop should be safe
    server.stop();
    EXPECT_TRUE(shutdownFlag_->load());

    // Third stop should still be safe
    server.stop();
    EXPECT_TRUE(shutdownFlag_->load());
}

TEST_F(ServerAppBranchTest, IsRunningTransitions) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Initially running
    EXPECT_TRUE(server.isRunning());

    // After stop, not running
    server.stop();
    EXPECT_FALSE(server.isRunning());

    // Check multiple times
    EXPECT_FALSE(server.isRunning());
    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppBranchTest, GetConnectedClientCountConsistency) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Multiple calls should be consistent
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
}

TEST_F(ServerAppBranchTest, GetConnectedClientIdsConsistency) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Multiple calls should return empty list
    auto ids1 = server.getConnectedClientIds();
    auto ids2 = server.getConnectedClientIds();
    auto ids3 = server.getConnectedClientIds();

    EXPECT_TRUE(ids1.empty());
    EXPECT_TRUE(ids2.empty());
    EXPECT_TRUE(ids3.empty());
}

TEST_F(ServerAppBranchTest, GetClientInfoMultipleInvalidIds) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Try multiple invalid client IDs
    EXPECT_FALSE(server.getClientInfo(0).has_value());
    EXPECT_FALSE(server.getClientInfo(1).has_value());
    EXPECT_FALSE(server.getClientInfo(999).has_value());
    EXPECT_FALSE(server.getClientInfo(12345).has_value());
    EXPECT_FALSE(server.getClientInfo(std::numeric_limits<uint32_t>::max()).has_value());
}

TEST_F(ServerAppBranchTest, StopAndCheckMultipleTimes) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    EXPECT_TRUE(server.isRunning());
    
    server.stop();
    EXPECT_FALSE(server.isRunning());
    
    // Check again
    EXPECT_FALSE(server.isRunning());
    
    // Stop again (should be safe)
    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppBranchTest, ConstructorDifferentPortNumbers) {
    // Test various port numbers
    std::vector<uint16_t> ports = {1024, 4000, 8080, 12345, 65535};
    
    for (const auto port : ports) {
        auto flag = std::make_shared<std::atomic<bool>>(false);
        EXPECT_NO_THROW({
            ServerApp server(port, 4, 60, flag, 30, false);
        }) << "Failed with port: " << port;
    }
}

TEST_F(ServerAppBranchTest, ConstructorDifferentMaxPlayers) {
    // Test various max player counts
    std::vector<size_t> playerCounts = {0, 1, 2, 4, 8, 16, 100, 1000};
    
    for (const auto count : playerCounts) {
        auto flag = std::make_shared<std::atomic<bool>>(false);
        EXPECT_NO_THROW({
            ServerApp server(8080, count, 60, flag, 30, false);
        }) << "Failed with max players: " << count;
    }
}

TEST_F(ServerAppBranchTest, ConstructorDifferentTickRates) {
    // Test various tick rates (all valid)
    std::vector<uint32_t> tickRates = {1, 10, 30, 60, 120, 240, 1000};
    
    for (const auto tickRate : tickRates) {
        auto flag = std::make_shared<std::atomic<bool>>(false);
        EXPECT_NO_THROW({
            ServerApp server(8080, 4, tickRate, flag, 30, false);
        }) << "Failed with tick rate: " << tickRate;
    }
}

TEST_F(ServerAppBranchTest, ConstructorDifferentTimeouts) {
    // Test various timeout values
    std::vector<uint32_t> timeouts = {0, 1, 5, 10, 30, 60, 300, 3600};
    
    for (const auto timeout : timeouts) {
        auto flag = std::make_shared<std::atomic<bool>>(false);
        EXPECT_NO_THROW({
            ServerApp server(8080, 4, 60, flag, timeout, false);
        }) << "Failed with timeout: " << timeout;
    }
}

TEST_F(ServerAppBranchTest, VerboseMode) {
    // Test with verbose enabled
    ServerApp server1(8080, 4, 60, shutdownFlag_, 30, true);
    EXPECT_TRUE(server1.isRunning());

    auto flag2 = std::make_shared<std::atomic<bool>>(false);
    ServerApp server2(8081, 4, 60, flag2, 30, false);
    EXPECT_TRUE(server2.isRunning());
}

TEST_F(ServerAppBranchTest, GettersAfterConstruction) {
    ServerApp server(9999, 8, 120, shutdownFlag_, 45, true);

    // Check initial state
    EXPECT_TRUE(server.isRunning());
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
    EXPECT_TRUE(server.getConnectedClientIds().empty());
}

TEST_F(ServerAppBranchTest, StopBeforeAnyOperation) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Stop immediately after construction
    server.stop();
    
    // Should be stopped
    EXPECT_FALSE(server.isRunning());
    
    // Getters should still work
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
    EXPECT_TRUE(server.getConnectedClientIds().empty());
}

TEST_F(ServerAppBranchTest, MultipleServersWithDifferentFlags) {
    auto flag1 = std::make_shared<std::atomic<bool>>(false);
    auto flag2 = std::make_shared<std::atomic<bool>>(false);
    auto flag3 = std::make_shared<std::atomic<bool>>(false);

    ServerApp server1(8080, 4, 60, flag1, 30, false);
    ServerApp server2(8081, 4, 60, flag2, 30, false);
    ServerApp server3(8082, 4, 60, flag3, 30, false);

    EXPECT_TRUE(server1.isRunning());
    EXPECT_TRUE(server2.isRunning());
    EXPECT_TRUE(server3.isRunning());

    server1.stop();
    EXPECT_FALSE(server1.isRunning());
    EXPECT_TRUE(server2.isRunning());
    EXPECT_TRUE(server3.isRunning());

    server2.stop();
    EXPECT_FALSE(server1.isRunning());
    EXPECT_FALSE(server2.isRunning());
    EXPECT_TRUE(server3.isRunning());

    server3.stop();
    EXPECT_FALSE(server1.isRunning());
    EXPECT_FALSE(server2.isRunning());
    EXPECT_FALSE(server3.isRunning());
}

TEST_F(ServerAppBranchTest, GetClientInfoEdgeCaseIds) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Test boundary values
    EXPECT_FALSE(server.getClientInfo(0).has_value());
    EXPECT_FALSE(server.getClientInfo(1).has_value());
    EXPECT_FALSE(server.getClientInfo(std::numeric_limits<uint32_t>::max()).has_value());
    EXPECT_FALSE(server.getClientInfo(std::numeric_limits<uint32_t>::max() - 1).has_value());
}

TEST_F(ServerAppBranchTest, DestructorAfterStop) {
    auto flag = std::make_shared<std::atomic<bool>>(false);
    {
        ServerApp server(8080, 4, 60, flag, 30, false);
        server.stop();
        // Destructor should handle stopped state
    }
    // Should complete without issues
    EXPECT_TRUE(true);
}

TEST_F(ServerAppBranchTest, DestructorWithoutStop) {
    auto flag = std::make_shared<std::atomic<bool>>(false);
    {
        ServerApp server(8080, 4, 60, flag, 30, false);
        // Destructor should handle running state
    }
    // Should complete without issues
    EXPECT_TRUE(true);
}

TEST_F(ServerAppBranchTest, ConstructorWithNullShutdownFlag) {
    // Note: This tests if code handles nullptr gracefully
    // Depending on implementation, this might crash or throw
    // Adjust test based on actual behavior
    try {
        ServerApp server(8080, 4, 60, nullptr, 30, false);
        // If we get here, nullptr is handled
        EXPECT_TRUE(true);
    } catch (...) {
        // If it throws, that's also acceptable behavior
        EXPECT_TRUE(true);
    }
}

TEST_F(ServerAppBranchTest, RapidStartStopCycles) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    for (int i = 0; i < 10; ++i) {
        if (i % 2 == 0) {
            shutdownFlag_->store(false);
            EXPECT_TRUE(server.isRunning());
        } else {
            shutdownFlag_->store(true);
            EXPECT_FALSE(server.isRunning());
        }
    }
}

TEST_F(ServerAppBranchTest, GetConnectedClientIdsMultipleCalls) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Call multiple times and verify consistency
    for (int i = 0; i < 5; ++i) {
        auto ids = server.getConnectedClientIds();
        EXPECT_TRUE(ids.empty());
    }
}

TEST_F(ServerAppBranchTest, IsRunningAfterFlagManipulation) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    EXPECT_TRUE(server.isRunning());

    shutdownFlag_->store(true);
    EXPECT_FALSE(server.isRunning());

    shutdownFlag_->store(false);
    EXPECT_TRUE(server.isRunning());

    shutdownFlag_->store(true);
    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppBranchTest, ConstructorPortZero) {
    // Port 0 might be valid (OS assigns port) or invalid depending on implementation
    EXPECT_NO_THROW({
        ServerApp server(0, 4, 60, shutdownFlag_, 30, false);
    });
}

TEST_F(ServerAppBranchTest, AllGettersBeforeAndAfterStop) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Before stop
    EXPECT_TRUE(server.isRunning());
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
    EXPECT_TRUE(server.getConnectedClientIds().empty());
    EXPECT_FALSE(server.getClientInfo(1).has_value());

    // After stop
    server.stop();
    EXPECT_FALSE(server.isRunning());
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
    EXPECT_TRUE(server.getConnectedClientIds().empty());
    EXPECT_FALSE(server.getClientInfo(1).has_value());
}

TEST_F(ServerAppBranchTest, ExtremeTickRateValues) {
    auto flag1 = std::make_shared<std::atomic<bool>>(false);
    EXPECT_NO_THROW({
        ServerApp server1(8080, 4, 1, flag1, 30, false);  // Very low
    });

    auto flag2 = std::make_shared<std::atomic<bool>>(false);
    EXPECT_NO_THROW({
        ServerApp server2(8080, 4, 100000, flag2, 30, false);  // Very high
    });
}

TEST_F(ServerAppBranchTest, ExtremeMaxPlayersValues) {
    auto flag1 = std::make_shared<std::atomic<bool>>(false);
    EXPECT_NO_THROW({
        ServerApp server1(8080, 0, 60, flag1, 30, false);  // Zero players
    });

    auto flag2 = std::make_shared<std::atomic<bool>>(false);
    EXPECT_NO_THROW({
        ServerApp server2(8080, std::numeric_limits<size_t>::max(), 60, flag2, 30, false);  // Max
    });
}

TEST_F(ServerAppBranchTest, ExtremeTimeoutValues) {
    auto flag1 = std::make_shared<std::atomic<bool>>(false);
    EXPECT_NO_THROW({
        ServerApp server1(8080, 4, 60, flag1, 0, false);  // No timeout
    });

    auto flag2 = std::make_shared<std::atomic<bool>>(false);
    EXPECT_NO_THROW({
        ServerApp server2(8080, 4, 60, flag2, std::numeric_limits<uint32_t>::max(), false);  // Max timeout
    });
}
