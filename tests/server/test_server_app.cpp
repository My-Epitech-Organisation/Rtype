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

#include "../../../src/server/ServerApp.hpp"

using namespace rtype::server;

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
    const uint32_t tickRate = 0;  // Edge case - should handle gracefully
    const uint32_t clientTimeoutSeconds = 30;
    const bool verbose = false;

    EXPECT_NO_THROW({
        ServerApp server(port, maxPlayers, tickRate, shutdownFlag_,
                        clientTimeoutSeconds, verbose);
    });
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
