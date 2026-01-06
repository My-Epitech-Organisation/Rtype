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

TEST_F(BroadcastIntegrationTest, CancelBroadcastsPropagateToClientLobby) {
    // Server on a different port to avoid collision
    ServerApp server(4270, 4, 60, _shutdownFlag, 10, false);

    std::mutex mtx;
    std::condition_variable cv;
    bool connected = false;
    std::uint32_t myId = 0;
    bool gameStartCalled = false;
    float lastDuration = -1.0f;

    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Setup client and lobby
    auto registry = std::make_shared<ECS::Registry>();
    rtype::game::config::RTypeGameConfig cfg;
    auto assets = std::make_shared<AssetManager>(cfg);

    NetworkClient::Config netCfg;
    auto netClient = std::make_shared<NetworkClient>(netCfg);

    // Capture connect and game start
    netClient->onConnected([&](std::uint32_t id) {
        std::unique_lock l(mtx);
        connected = true;
        myId = id;
        cv.notify_one();
    });

    netClient->onGameStart([&](float duration) {
        std::unique_lock l(mtx);
        gameStartCalled = true;
        lastDuration = duration;
        cv.notify_one();
    });

    // Create lobby that listens to netClient callbacks
    Lobby lobby(registry, assets, nullptr, [](const SceneManager::Scene&) {}, netClient, nullptr, nullptr);

    // Connect client to server
    EXPECT_TRUE(netClient->connect("127.0.0.1", 4270));

    // Wait for connection and assigned id
    {
        std::unique_lock l(mtx);
        bool ok = cv.wait_for(l, std::chrono::milliseconds(500), [&] { return connected; });
        EXPECT_TRUE(ok);
    }

    // Start countdown by marking player ready on server
    server.playerReady(myId);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_TRUE(server.isCountdownActive());

    // Cancel countdown
    server.playerNotReady(myId);

    // Wait for client to receive game start cancel broadcast
    {
        std::unique_lock l(mtx);
        bool signaled = cv.wait_for(l, std::chrono::milliseconds(500), [&] { return gameStartCalled; });
        EXPECT_TRUE(signaled);
    }

    EXPECT_TRUE(gameStartCalled);
    EXPECT_FLOAT_EQ(lastDuration, 0.0f);

    // Lobby should have cleared its countdown state
    EXPECT_FALSE(lobby.isCountdownActive());
    EXPECT_FLOAT_EQ(lobby.getCountdownTimer(), 0.0f);

    _shutdownFlag->store(true);
    if (serverThread.joinable()) serverThread.join();
}
