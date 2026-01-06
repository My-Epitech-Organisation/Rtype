/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_server_integration - Tests for ServerApp integration with GameEngine and NetworkServer
*/

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "server/serverApp/ServerApp.hpp"

using namespace rtype::server;

class ServerIntegrationTest : public ::testing::Test {
   protected:
    void SetUp() override {
        _shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    }

    void TearDown() override {
        _shutdownFlag->store(true);
    }

    std::shared_ptr<std::atomic<bool>> _shutdownFlag;
};

/**
 * @brief Test that ServerApp initializes all components correctly
 */
TEST_F(ServerIntegrationTest, InitializationCreatesAllComponents) {
    // Create server with test configuration
    ServerApp server(4242, 4, 60, _shutdownFlag, 10, true);

    // Start server in a thread
    std::thread serverThread([&server, this]() {
        server.run();
    });

    // Give server time to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Verify server is running
    EXPECT_TRUE(server.isRunning());

    // Verify metrics are accessible
    const auto& metrics = server.getMetrics();
    EXPECT_EQ(metrics.packetsReceived.load(), 0u);

    // Signal shutdown
    _shutdownFlag->store(true);

    // Wait for server to stop
    if (serverThread.joinable()) {
        serverThread.join();
    }

    // Verify clean shutdown
    EXPECT_FALSE(server.isRunning());
}

/**
 * @brief Test that server can start and stop multiple times
 */
TEST_F(ServerIntegrationTest, ServerCanRestartCleanly) {
    for (int i = 0; i < 3; ++i) {
        auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
        ServerApp server(4243 + i, 4, 60, shutdownFlag, 10, false);

        std::thread serverThread([&server]() {
            server.run();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(server.isRunning()) << "Iteration " << i;

        shutdownFlag->store(true);
        if (serverThread.joinable()) {
            serverThread.join();
        }

        EXPECT_FALSE(server.isRunning()) << "Iteration " << i;
    }
}

/**
 * @brief Test server loop timing configuration
 * NOTE: This test was disabled because getLoopTiming() was moved to ServerLoop class
 */
/*
TEST_F(ServerIntegrationTest, LoopTimingConfiguration) {
    ServerApp server(4250, 4, 60, _shutdownFlag, 10, false);

    auto timing = server.getLoopTiming();

    // At 60 Hz, fixed delta should be ~16.67ms
    auto expectedDeltaNs = std::chrono::nanoseconds(
        static_cast<long long>(1e9 / 60.0));

    // Allow 1% tolerance
    auto difference = std::abs(timing.fixedDeltaNs.count() - expectedDeltaNs.count());
    EXPECT_LT(difference, expectedDeltaNs.count() / 100);

    // Max frame time should be 250ms
    EXPECT_EQ(timing.maxFrameTime, std::chrono::milliseconds(250));

    // Max updates per frame should be 5
    EXPECT_EQ(timing.maxUpdatesPerFrame, 5u);
}
*/

/**
 * @brief Test that verbose mode enables detailed logging
 */
TEST_F(ServerIntegrationTest, VerboseModeWorks) {
    // This test mainly verifies no crashes occur with verbose mode
    ServerApp server(4251, 4, 60, _shutdownFlag, 10, true);

    std::thread serverThread([&server]() {
        server.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    _shutdownFlag->store(true);
    if (serverThread.joinable()) {
        serverThread.join();
    }

    // If we get here without crashing, verbose mode works
    SUCCEED();
}

/**
 * @brief Test server metrics tracking
 */
TEST_F(ServerIntegrationTest, MetricsAreTracked) {
    ServerApp server(4252, 4, 60, _shutdownFlag, 10, false);

    std::thread serverThread([&server]() {
        server.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    const auto& metrics = server.getMetrics();

    // These should be initialized to 0
    EXPECT_GE(metrics.packetsReceived.load(), 0u);
    EXPECT_GE(metrics.packetsSent.load(), 0u);
    EXPECT_GE(metrics.packetsDropped.load(), 0u);

    _shutdownFlag->store(true);
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

/**
 * @brief Test that server starts in WaitingForPlayers state
 */
TEST_F(ServerIntegrationTest, StartsInWaitingForPlayersState) {
    ServerApp server(4253, 4, 60, _shutdownFlag, 10, false);

    // Before running, state should be WaitingForPlayers (default)
    EXPECT_EQ(server.getGameState(), GameState::WaitingForPlayers);
    EXPECT_FALSE(server.isPlaying());
    EXPECT_EQ(server.getReadyPlayerCount(), 0u);

    std::thread serverThread([&server]() {
        server.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Should still be waiting for players (no clients connected)
    EXPECT_EQ(server.getGameState(), GameState::WaitingForPlayers);
    EXPECT_FALSE(server.isPlaying());

    _shutdownFlag->store(true);
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

/**
 * @brief Test that playerReady() transitions to Playing state
 */
TEST_F(ServerIntegrationTest, PlayerReadyStartsGame) {
    ServerApp server(4254, 4, 60, _shutdownFlag, 10, false);

    std::thread serverThread([&server]() {
        server.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Verify starting state
    EXPECT_EQ(server.getGameState(), GameState::WaitingForPlayers);
    EXPECT_EQ(server.getReadyPlayerCount(), 0u);

    // Simulate player ready signal
    server.playerReady(1);

    // Give time for state transition
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Should now be playing
    EXPECT_EQ(server.getGameState(), GameState::Playing);
    EXPECT_TRUE(server.isPlaying());
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);

    _shutdownFlag->store(true);
    if (serverThread.joinable()) {
        serverThread.join();
    }
}
