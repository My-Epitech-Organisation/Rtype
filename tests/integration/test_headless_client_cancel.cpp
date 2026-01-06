#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "server/serverApp/ServerApp.hpp"
#include "client/network/NetworkClient.hpp"

using namespace rtype::server;
using namespace rtype::client;

class HeadlessClientIntegrationTest : public ::testing::Test {
   protected:
    void SetUp() override { _shutdownFlag = std::make_shared<std::atomic<bool>>(false); }
    void TearDown() override { _shutdownFlag->store(true); }

    std::shared_ptr<std::atomic<bool>> _shutdownFlag;
};

TEST_F(HeadlessClientIntegrationTest, ClientReceivesServerCancel) {
    const uint16_t port = 4270;

    ServerApp server(port, 4, 60, _shutdownFlag, 10, false);

    std::thread serverThread([&server]() { server.run(); });

    // Give server time to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ASSERT_TRUE(server.isRunning());

    NetworkClient::Config cfg;
    NetworkClient client(cfg);

    std::mutex mtx;
    std::condition_variable cv;

    bool connected = false;
    uint32_t myId = 0;

    bool gameStartReceived = false;
    float receivedDuration = -1.0f;

    client.onConnected([&](uint32_t id) {
        std::unique_lock l(mtx);
        connected = true;
        myId = id;
        cv.notify_one();
    });

    client.onGameStart([&](float duration) {
        std::unique_lock l(mtx);
        gameStartReceived = true;
        receivedDuration = duration;
        cv.notify_one();
    });

    // Connect to server
    EXPECT_TRUE(client.connect("127.0.0.1", port));

    // Wait for onConnected
    {
        std::unique_lock l(mtx);
        bool signaled = cv.wait_for(l, std::chrono::milliseconds(500), [&] { return connected; });
        EXPECT_TRUE(signaled);
    }

    // Start countdown for our client id
    server.playerReady(myId);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(server.isCountdownActive());

    // Cancel via unready
    server.playerNotReady(myId);

    // Wait for client to receive the cancel (timeout 500ms)
    {
        std::unique_lock l(mtx);
        bool signaled = cv.wait_for(l, std::chrono::milliseconds(500), [&] { return gameStartReceived; });
        EXPECT_TRUE(signaled);
    }

    EXPECT_TRUE(gameStartReceived);
    EXPECT_FLOAT_EQ(receivedDuration, 0.0f);

    _shutdownFlag->store(true);
    if (serverThread.joinable()) serverThread.join();
}
