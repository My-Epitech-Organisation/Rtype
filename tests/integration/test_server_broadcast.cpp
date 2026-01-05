#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "server/serverApp/ServerApp.hpp"

using namespace rtype::server;

class BroadcastIntegrationTest : public ::testing::Test {
   protected:
    void SetUp() override { _shutdownFlag = std::make_shared<std::atomic<bool>>(false); }
    void TearDown() override { _shutdownFlag->store(true); }

    std::shared_ptr<std::atomic<bool>> _shutdownFlag;
};

TEST_F(BroadcastIntegrationTest, BroadcastsCancelOnUnready) {
    ServerApp server(4260, 4, 60, _shutdownFlag, 10, false);

    std::mutex mtx;
    std::condition_variable cv;
    bool callbackCalled = false;
    float lastDuration = -1.0f;

    server.setOnGameStartBroadcastCallback([&](float duration) {
        std::unique_lock l(mtx);
        callbackCalled = true;
        lastDuration = duration;
        cv.notify_one();
    });

    std::thread serverThread([&server]() { server.run(); });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Start countdown
    server.playerReady(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(server.isCountdownActive());

    // Cancel countdown via unready
    server.playerNotReady(1);

    // Wait for callback (timeout 500ms)
    {
        std::unique_lock l(mtx);
        bool signaled = cv.wait_for(l, std::chrono::milliseconds(500), [&] { return callbackCalled; });
        EXPECT_TRUE(signaled);
    }

    EXPECT_TRUE(callbackCalled);
    EXPECT_FLOAT_EQ(lastDuration, 0.0f);

    _shutdownFlag->store(true);
    if (serverThread.joinable()) serverThread.join();
}
