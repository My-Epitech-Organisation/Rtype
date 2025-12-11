#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "server/serverApp/ServerApp.hpp"

using namespace rtype::server;

class ServerAppExtraTest : public ::testing::Test {
protected:
    void SetUp() override {
        shutdownFlag_ = std::make_shared<std::atomic<bool>>(false);
    }

    void TearDown() override {
        shutdownFlag_->store(true);
    }

    std::shared_ptr<std::atomic<bool>> shutdownFlag_;
};

TEST_F(ServerAppExtraTest, Run_StartupAndShutdown_NoExceptions) {
    ServerApp server(4242, 2, 60, shutdownFlag_, 10, false);

    std::thread serverThread([&]() {
        // run() is blocking - will exit when shutdownFlag is set
        EXPECT_TRUE(server.run());
    });

    // Allow the server to run a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Trigger stop
    server.stop();

    serverThread.join();

    // After run, server should be stopped
    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppExtraTest, StartStopNetworkThread_ThreadLifecycle) {
    ServerApp server(4242, 2, 60, shutdownFlag_, 10, false);

    // Start the run in the background so it spawns the network thread
    std::thread serverThread([&]() {
        EXPECT_TRUE(server.run());
    });

    // Give some time for the network thread to start
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    // Trigger stop to ensure stopNetworkThread is invoked
    server.stop();

    serverThread.join();

    EXPECT_FALSE(server.isRunning());
}
