/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_server_app
*/

#include <gtest/gtest.h>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>

#include "../../../src/server/ServerApp.hpp"
#include "../../../lib/network/src/Packet.hpp"

using namespace rtype::server;
using namespace rtype::network;

class ServerAppTest : public ::testing::Test {
protected:
    void SetUp() override {
        shutdownFlag_ = std::make_shared<std::atomic<bool>>(false);
    }

    void TearDown() override {
        shutdownFlag_->store(true);
    }

    std::shared_ptr<std::atomic<bool>> shutdownFlag_;
};

TEST_F(ServerAppTest, Constructor_ValidParameters) {
    const uint16_t port = 8080;
    const size_t maxPlayers = 4;
    const uint32_t tickRate = 60;
    const uint32_t clientTimeoutSeconds = 30;
    const bool verbose = false;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

TEST_F(ServerAppTest, Constructor_ExtremeValues) {
    // Test with extreme but valid values
    const uint16_t port = 65535;  // Max port
    const size_t maxPlayers = 1000;
    const uint32_t tickRate = 1000;  // Very high tick rate
    const uint32_t clientTimeoutSeconds = 3600;  // 1 hour
    const bool verbose = true;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

TEST_F(ServerAppTest, Constructor_ZeroMaxPlayers) {
    const uint16_t port = 8080;
    const size_t maxPlayers = 0;  // Edge case
    const uint32_t tickRate = 60;
    const uint32_t clientTimeoutSeconds = 30;
    const bool verbose = false;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

TEST_F(ServerAppTest, Constructor_ZeroTickRate) {
    const uint16_t port = 8080;
    const size_t maxPlayers = 4;
    const uint32_t tickRate = 0;
    const uint32_t clientTimeoutSeconds = 30;
    const bool verbose = false;

    // Should throw an exception for zero tick rate
    EXPECT_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    }, std::invalid_argument);
}

TEST_F(ServerAppTest, Constructor_ZeroTimeout) {
    const uint16_t port = 8080;
    const size_t maxPlayers = 4;
    const uint32_t tickRate = 60;
    const uint32_t clientTimeoutSeconds = 0;  // Edge case
    const bool verbose = false;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

TEST_F(ServerAppTest, Stop_SetsShutdownFlag) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    EXPECT_FALSE(shutdownFlag_->load());
    server.stop();
    EXPECT_TRUE(shutdownFlag_->load());
}

TEST_F(ServerAppTest, IsRunning_ReturnsCorrectState) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    EXPECT_TRUE(server.isRunning());
    shutdownFlag_->store(true);
    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppTest, GetConnectedClientCount_InitiallyZero) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    EXPECT_EQ(server.getConnectedClientCount(), 0u);
}

TEST_F(ServerAppTest, GetConnectedClientIds_InitiallyEmpty) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    auto ids = server.getConnectedClientIds();
    EXPECT_TRUE(ids.empty());
}

TEST_F(ServerAppTest, GetClientInfo_InvalidClient_ReturnsNullopt) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    auto clientInfo = server.getClientInfo(999);
    EXPECT_FALSE(clientInfo.has_value());
}

TEST_F(ServerAppTest, StopBeforeRun) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Should be ready to run initially
    EXPECT_TRUE(server.isRunning());

    // Stop should make it not ready to run
    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppTest, StopMultipleTimes) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Multiple stops should be safe
    server.stop();
    server.stop();
    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppTest, GetClientManager) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Should be able to get client manager
    ClientManager& cm = server.getClientManager();
    EXPECT_EQ(cm.getMaxPlayers(), 4u);

    const ClientManager& cmConst = server.getClientManager();
    EXPECT_EQ(cmConst.getMaxPlayers(), 4u);
}

TEST_F(ServerAppTest, GetMetrics) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Should be able to get metrics
    const ServerMetrics& metrics = server.getMetrics();
    EXPECT_GE(metrics.totalConnections, 0u);
}

TEST_F(ServerAppTest, GetLoopTiming_ValidValues) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    auto timing = server.getLoopTiming();

    // Check that timing values are reasonable for 60 FPS
    EXPECT_GT(timing.fixedDeltaNs.count(), 0);
    EXPECT_LT(timing.fixedDeltaNs.count(), 1000000000);  // Less than 1 second
    EXPECT_GE(timing.maxFrameTime.count(), 0);
    EXPECT_EQ(timing.maxUpdatesPerFrame, 5u);  // MAX_UPDATES_PER_FRAME
}

TEST_F(ServerAppTest, Constructor_MinimumValues) {
    // Test with minimum reasonable values
    const uint16_t port = 1024;  // Above well-known ports
    const size_t maxPlayers = 1;
    const uint32_t tickRate = 1;  // 1 FPS
    const uint32_t clientTimeoutSeconds = 1;
    const bool verbose = false;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
}

// Note: Testing ServerApp::run() would require network mocking and
// is complex due to the infinite loop. Integration tests would be
// better suited for full server testing with actual network components.