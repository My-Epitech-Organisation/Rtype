/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_server_app_unit - Comprehensive unit tests for ServerApp
*/

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "server/serverApp/ServerApp.hpp"
#include "server/shared/IGameConfig.hpp"

using namespace rtype::server;

// ============================================================================
// MOCK GAME CONFIG
// ============================================================================

class MockGameConfigUnit : public IGameConfig {
   public:
    MockGameConfigUnit() = default;

    bool initialize(const std::string& configDir) override {
        _configDir = configDir;
        _initialized = !_shouldFailInit;
        return _initialized;
    }

    bool reloadConfiguration() override {
        _reloadCount++;
        if (_shouldFailReload) {
            return false;
        }
        _gameplaySettings.enemySpeedMultiplier += 0.1f;
        return true;
    }

    bool isInitialized() const noexcept override { return _initialized; }

    GenericServerSettings getServerSettings() const noexcept override {
        return _serverSettings;
    }

    GenericGameplaySettings getGameplaySettings() const noexcept override {
        return _gameplaySettings;
    }

    std::string getSavesPath() const noexcept override { return _savesPath; }

    bool saveGame(const std::string& slotName,
                  const std::vector<uint8_t>& gameStateData) override {
        _savedSlots[slotName] = gameStateData;
        return true;
    }

    std::vector<uint8_t> loadGame(const std::string& slotName) override {
        auto it = _savedSlots.find(slotName);
        if (it != _savedSlots.end()) {
            return it->second;
        }
        return {};
    }

    std::vector<GenericSaveInfo> listSaves() const override {
        std::vector<GenericSaveInfo> saves;
        for (const auto& [name, data] : _savedSlots) {
            GenericSaveInfo info;
            info.filename = name + ".sav";
            info.saveName = name;
            info.isValid = true;
            saves.push_back(info);
        }
        return saves;
    }

    bool saveExists(const std::string& slotName) const override {
        return _savedSlots.count(slotName) > 0;
    }

    bool deleteSave(const std::string& slotName) override {
        return _savedSlots.erase(slotName) > 0;
    }

    const std::string& getLastError() const noexcept override {
        return _lastError;
    }

    std::string getGameId() const noexcept override { return _gameId; }

    // Test helpers
    void setShouldFailInit(bool fail) { _shouldFailInit = fail; }
    void setShouldFailReload(bool fail) { _shouldFailReload = fail; }
    void setServerSettings(const GenericServerSettings& settings) {
        _serverSettings = settings;
    }
    void setGameplaySettings(const GenericGameplaySettings& settings) {
        _gameplaySettings = settings;
    }
    void setGameId(const std::string& id) { _gameId = id; }
    void setInitialized(bool init) { _initialized = init; }
    int getReloadCount() const { return _reloadCount; }

   private:
    bool _initialized = false;
    bool _shouldFailInit = false;
    bool _shouldFailReload = false;
    int _reloadCount = 0;
    std::string _configDir;
    std::string _savesPath = "/tmp/saves";
    std::string _lastError;
    std::string _gameId = "mock_game";
    GenericServerSettings _serverSettings;
    GenericGameplaySettings _gameplaySettings;
    std::unordered_map<std::string, std::vector<uint8_t>> _savedSlots;
};

// ============================================================================
// TEST FIXTURE
// ============================================================================

class ServerAppUnitTest : public ::testing::Test {
   protected:
    void SetUp() override {
        shutdownFlag_ = std::make_shared<std::atomic<bool>>(false);
    }

    void TearDown() override {
        shutdownFlag_->store(true);
    }

    std::shared_ptr<std::atomic<bool>> shutdownFlag_;
};

// ============================================================================
// GAME STATE TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, GameState_InitiallyWaitingForPlayers) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
    EXPECT_EQ(server.getGameState(), GameState::WaitingForPlayers);
    EXPECT_FALSE(server.isPlaying());
}

TEST_F(ServerAppUnitTest, GameState_ReadyPlayerCount_InitiallyZero) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
    EXPECT_EQ(server.getReadyPlayerCount(), 0u);
}

TEST_F(ServerAppUnitTest, GameState_PlayerReady_IncreasesCount) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // First player ready triggers game start (MIN_PLAYERS_TO_START = 1)
    server.playerReady(1);
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);
    EXPECT_TRUE(server.isPlaying());  // Game starts immediately

    // Second player ready is ignored since game is already running
    server.playerReady(2);
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);  // Still 1, second player ignored
}

TEST_F(ServerAppUnitTest, GameState_PlayerReady_DuplicateIgnored) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    server.playerReady(1);
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);

    // Same player ready again - should not increase count
    server.playerReady(1);
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);
}

TEST_F(ServerAppUnitTest, GameState_TransitionToPlaying) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    EXPECT_EQ(server.getGameState(), GameState::WaitingForPlayers);

    // One player ready should trigger game start (MIN_PLAYERS_TO_START = 1)
    server.playerReady(1);

    EXPECT_EQ(server.getGameState(), GameState::Playing);
    EXPECT_TRUE(server.isPlaying());
}

TEST_F(ServerAppUnitTest, GameState_PlayerReadyWhenAlreadyPlaying) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    server.playerReady(1);
    EXPECT_EQ(server.getGameState(), GameState::Playing);

    // Player ready when already playing - should not crash
    server.playerReady(2);
    EXPECT_EQ(server.getGameState(), GameState::Playing);
}

// ============================================================================
// LOOP TIMING TESTS
// NOTE: These tests were disabled because getLoopTiming() was moved to ServerLoop class
// ============================================================================

/*
TEST_F(ServerAppUnitTest, LoopTiming_60Hz) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    auto timing = server.getLoopTiming();

    // 60 Hz = ~16.67ms per tick
    auto expectedNs = std::chrono::nanoseconds(
        static_cast<int64_t>(1e9 / 60.0));
    EXPECT_NEAR(timing.fixedDeltaNs.count(), expectedNs.count(), 1000);
    EXPECT_EQ(timing.maxUpdatesPerFrame, 5u);
}

TEST_F(ServerAppUnitTest, LoopTiming_120Hz) {
    ServerApp server(8080, 4, 120, shutdownFlag_, 30, false);

    auto timing = server.getLoopTiming();

    // 120 Hz = ~8.33ms per tick
    auto expectedNs = std::chrono::nanoseconds(
        static_cast<int64_t>(1e9 / 120.0));
    EXPECT_NEAR(timing.fixedDeltaNs.count(), expectedNs.count(), 1000);
}

TEST_F(ServerAppUnitTest, LoopTiming_30Hz) {
    ServerApp server(8080, 4, 30, shutdownFlag_, 30, false);

    auto timing = server.getLoopTiming();

    // 30 Hz = ~33.33ms per tick
    auto expectedNs = std::chrono::nanoseconds(
        static_cast<int64_t>(1e9 / 30.0));
    EXPECT_NEAR(timing.fixedDeltaNs.count(), expectedNs.count(), 1000);
}

TEST_F(ServerAppUnitTest, LoopTiming_1Hz) {
    ServerApp server(8080, 4, 1, shutdownFlag_, 30, false);

    auto timing = server.getLoopTiming();

    // 1 Hz = 1000ms per tick
    auto expectedNs = std::chrono::nanoseconds(1000000000);
    EXPECT_EQ(timing.fixedDeltaNs.count(), expectedNs.count());
}

TEST_F(ServerAppUnitTest, LoopTiming_MaxFrameTime) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    auto timing = server.getLoopTiming();

    // MAX_FRAME_TIME_MS = 250
    auto expectedMaxFrameNs = std::chrono::nanoseconds(250 * 1000000);
    EXPECT_EQ(timing.maxFrameTime.count(), expectedMaxFrameNs.count());
}
*/

// ============================================================================
// GAME CONFIG TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, GameConfig_NoConfig) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    EXPECT_EQ(server.getGameConfig(), nullptr);
    EXPECT_FALSE(server.hasGameConfig());
}

TEST_F(ServerAppUnitTest, GameConfig_WithInitializedConfig) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);
    config->setGameId("test_game");

    GenericServerSettings settings;
    settings.port = 5000;
    settings.maxPlayers = 8;
    settings.tickRate = 60;
    config->setServerSettings(settings);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_NE(server.getGameConfig(), nullptr);
    EXPECT_TRUE(server.hasGameConfig());
    EXPECT_EQ(server.getGameConfig()->getGameId(), "test_game");
}

TEST_F(ServerAppUnitTest, GameConfig_WithUninitializedConfig) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(false);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_NE(server.getGameConfig(), nullptr);
    EXPECT_FALSE(server.hasGameConfig());
}

TEST_F(ServerAppUnitTest, GameConfig_NullConfig) {
    ServerApp server(nullptr, shutdownFlag_, false);

    EXPECT_EQ(server.getGameConfig(), nullptr);
    EXPECT_FALSE(server.hasGameConfig());
}

TEST_F(ServerAppUnitTest, GameConfig_ConstAccess) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);
    config->setGameId("const_test");

    const ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_NE(server.getGameConfig(), nullptr);
    EXPECT_EQ(server.getGameConfig()->getGameId(), "const_test");
}

// ============================================================================
// RELOAD CONFIGURATION TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, ReloadConfiguration_NoConfig) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    EXPECT_FALSE(server.reloadConfiguration());
}

TEST_F(ServerAppUnitTest, ReloadConfiguration_UninitializedConfig) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(false);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_FALSE(server.reloadConfiguration());
}

TEST_F(ServerAppUnitTest, ReloadConfiguration_Success) {
    auto config = std::make_unique<MockGameConfigUnit>();
    auto configPtr = config.get();
    config->setInitialized(true);

    GenericServerSettings settings;
    settings.port = 5000;
    config->setServerSettings(settings);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_TRUE(server.reloadConfiguration());
    EXPECT_EQ(configPtr->getReloadCount(), 1);
}

TEST_F(ServerAppUnitTest, ReloadConfiguration_Failure) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);
    config->setShouldFailReload(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_FALSE(server.reloadConfiguration());
}

TEST_F(ServerAppUnitTest, ReloadConfiguration_PortChange) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);

    GenericServerSettings settings;
    settings.port = 5000;  // Different from what will be used
    config->setServerSettings(settings);

    ServerApp server(std::move(config), shutdownFlag_, false);

    // Reload should succeed but warn about port change
    EXPECT_TRUE(server.reloadConfiguration());
}

// ============================================================================
// CLIENT MANAGER TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, ClientManager_Access) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    ClientManager& cm = server.getClientManager();
    EXPECT_EQ(cm.getMaxPlayers(), 4u);
}

TEST_F(ServerAppUnitTest, ClientManager_ConstAccess) {
    const ServerApp server(8080, 8, 60, shutdownFlag_, 30, false);

    const ClientManager& cm = server.getClientManager();
    EXPECT_EQ(cm.getMaxPlayers(), 8u);
}

TEST_F(ServerAppUnitTest, ClientManager_DifferentMaxPlayers) {
    ServerApp server1(8080, 1, 60, shutdownFlag_, 30, false);
    EXPECT_EQ(server1.getClientManager().getMaxPlayers(), 1u);

    auto shutdownFlag2 = std::make_shared<std::atomic<bool>>(false);
    ServerApp server2(8081, 100, 60, shutdownFlag2, 30, false);
    EXPECT_EQ(server2.getClientManager().getMaxPlayers(), 100u);
    shutdownFlag2->store(true);
}

// ============================================================================
// METRICS TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, Metrics_InitialValues) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    const ServerMetrics& metrics = server.getMetrics();
    EXPECT_EQ(metrics.totalConnections.load(), 0u);
    EXPECT_EQ(metrics.tickOverruns.load(), 0u);
    EXPECT_EQ(metrics.packetsDropped.load(), 0u);
    EXPECT_EQ(metrics.connectionsRejected.load(), 0u);
}

// ============================================================================
// VERBOSE MODE TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, VerboseMode_False) {
    EXPECT_NO_THROW({
        ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
    });
}

TEST_F(ServerAppUnitTest, VerboseMode_True) {
    EXPECT_NO_THROW({
        ServerApp server(8080, 4, 60, shutdownFlag_, 30, true);
    });
}

// ============================================================================
// RUN AND STOP TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, Run_ImmediateShutdown) {
    ServerApp server(14250, 4, 60, shutdownFlag_, 30, false);

    shutdownFlag_->store(true);

    std::thread serverThread([&]() {
        [[maybe_unused]] bool result = server.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    serverThread.join();

    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppUnitTest, Run_ShutdownAfterBrief) {
    ServerApp server(14251, 4, 60, shutdownFlag_, 30, false);

    std::thread serverThread([&]() {
        [[maybe_unused]] bool result = server.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    server.stop();

    serverThread.join();
    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppUnitTest, Run_MultipleStopCalls) {
    ServerApp server(14252, 4, 60, shutdownFlag_, 30, false);

    std::thread serverThread([&]() {
        [[maybe_unused]] bool result = server.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Multiple stop calls should be safe
    server.stop();
    server.stop();
    server.stop();

    serverThread.join();
    EXPECT_FALSE(server.isRunning());
}

// ============================================================================
// SECURITY CONTEXT TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, RegisterUserIdMapping) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    Endpoint endpoint("127.0.0.1", 12345);

    // Should not throw
    EXPECT_NO_THROW({
        server.registerUserIdMapping(endpoint, 1);
    });
}

TEST_F(ServerAppUnitTest, RegisterUserIdMapping_Multiple) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    Endpoint endpoint1("127.0.0.1", 12345);
    Endpoint endpoint2("127.0.0.1", 12346);
    Endpoint endpoint3("192.168.1.1", 12345);

    EXPECT_NO_THROW({
        server.registerUserIdMapping(endpoint1, 1);
        server.registerUserIdMapping(endpoint2, 2);
        server.registerUserIdMapping(endpoint3, 3);
    });
}

// ============================================================================
// CONNECTED CLIENT TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, GetConnectedClientCount_NoClients) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
    EXPECT_EQ(server.getConnectedClientCount(), 0u);
}

TEST_F(ServerAppUnitTest, GetConnectedClientIds_NoClients) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    auto ids = server.getConnectedClientIds();
    EXPECT_TRUE(ids.empty());
}

TEST_F(ServerAppUnitTest, GetClientInfo_NonExistent) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    auto info = server.getClientInfo(12345);
    EXPECT_FALSE(info.has_value());
}

TEST_F(ServerAppUnitTest, GetClientInfo_InvalidId) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    auto info = server.getClientInfo(0);
    EXPECT_FALSE(info.has_value());
}

// ============================================================================
// GAME CONFIG WITH DIFFERENT SETTINGS
// ============================================================================

TEST_F(ServerAppUnitTest, GameConfig_CustomPort) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);

    GenericServerSettings settings;
    settings.port = 9999;
    settings.maxPlayers = 16;
    settings.tickRate = 120;
    config->setServerSettings(settings);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_EQ(server.getClientManager().getMaxPlayers(), 16u);
}

TEST_F(ServerAppUnitTest, GameConfig_GameplaySettings) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);

    GenericGameplaySettings gameplay;
    gameplay.playerSpeed = 300.0f;
    gameplay.difficulty = "hard";
    gameplay.startingLives = 5;
    config->setGameplaySettings(gameplay);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_TRUE(server.hasGameConfig());
    EXPECT_EQ(server.getGameConfig()->getGameplaySettings().difficulty, "hard");
}

// ============================================================================
// DESTRUCTOR TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, Destructor_CleanShutdown) {
    {
        ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
        // Destructor should handle cleanup
    }
    // No crash = success
    EXPECT_TRUE(true);
}

TEST_F(ServerAppUnitTest, Destructor_AfterStop) {
    {
        ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);
        server.stop();
        // Destructor after stop should be safe
    }
    EXPECT_TRUE(true);
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

// NOTE: This test was disabled because getLoopTiming() was moved to ServerLoop class
/*
TEST_F(ServerAppUnitTest, EdgeCase_HighTickRate) {
    ServerApp server(8080, 4, 1000, shutdownFlag_, 30, false);

    auto timing = server.getLoopTiming();
    EXPECT_GT(timing.fixedDeltaNs.count(), 0);
    EXPECT_LT(timing.fixedDeltaNs.count(), 2000000);  // Less than 2ms
}
*/

TEST_F(ServerAppUnitTest, EdgeCase_ManyPlayersReady) {
    ServerApp server(8080, 100, 60, shutdownFlag_, 30, false);

    // First player ready triggers game start (MIN_PLAYERS_TO_START = 1)
    // Subsequent playerReady calls are ignored once game is playing
    for (std::uint32_t i = 1; i <= 50; ++i) {
        server.playerReady(i);
    }

    // Only first player is counted - game started after first ready
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);
    EXPECT_TRUE(server.isPlaying());
}

TEST_F(ServerAppUnitTest, EdgeCase_LargeUserId) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    server.playerReady(0xFFFFFFFF);
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);
}

// ============================================================================
// GAME STATE TRANSITION TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, GameState_TransitionToPlaying_WithNetworkSystem) {
    // Use constructor that initializes network system through run()
    shutdownFlag_->store(true);  // Will exit immediately

    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Make player ready before run
    server.playerReady(1);

    EXPECT_TRUE(server.isPlaying());
    EXPECT_EQ(server.getGameState(), GameState::Playing);
}

TEST_F(ServerAppUnitTest, GameState_CheckGameStart_AlreadyPlaying) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // Transition to playing
    server.playerReady(1);
    EXPECT_TRUE(server.isPlaying());

    // Additional playerReady should not change state
    server.playerReady(2);
    server.playerReady(3);

    EXPECT_TRUE(server.isPlaying());
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);  // Only first counted
}

// ============================================================================
// RUN WITH DIFFERENT CONFIGURATIONS
// ============================================================================

TEST_F(ServerAppUnitTest, Run_WithGameConfig_Initialized) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);

    GenericServerSettings settings;
    settings.port = 8085;
    settings.maxPlayers = 8;
    settings.tickRate = 30;
    config->setServerSettings(settings);

    shutdownFlag_->store(true);  // Exit immediately

    ServerApp server(std::move(config), shutdownFlag_, false);

    // Should use config values
    EXPECT_EQ(server.getClientManager().getMaxPlayers(), 8u);
}

TEST_F(ServerAppUnitTest, Run_WithGameConfig_NotInitialized) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(false);

    shutdownFlag_->store(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    // Should use default values
    EXPECT_EQ(server.getClientManager().getMaxPlayers(), 4u);
}

// ============================================================================
// RELOAD CONFIGURATION TESTS (MORE BRANCHES)
// ============================================================================

TEST_F(ServerAppUnitTest, ReloadConfig_SuccessfulReload) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_TRUE(server.reloadConfiguration());
}

TEST_F(ServerAppUnitTest, ReloadConfig_FailedReload) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);
    config->setShouldFailReload(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_FALSE(server.reloadConfiguration());
}

TEST_F(ServerAppUnitTest, ReloadConfig_NullConfig) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // No game config, should fail
    EXPECT_FALSE(server.reloadConfiguration());
}

TEST_F(ServerAppUnitTest, ReloadConfig_UninitializedConfig) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(false);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_FALSE(server.reloadConfiguration());
}

// ============================================================================
// SECURITY CONTEXT TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, SecurityContext_RegisterMapping) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    Endpoint endpoint("192.168.1.100", 54321);

    EXPECT_NO_THROW({
        server.registerUserIdMapping(endpoint, 42);
    });
}

TEST_F(ServerAppUnitTest, SecurityContext_MultipleEndpoints) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    for (int i = 0; i < 10; ++i) {
        Endpoint endpoint("192.168.1." + std::to_string(i), 50000 + i);
        EXPECT_NO_THROW({
            server.registerUserIdMapping(endpoint, static_cast<uint32_t>(i));
        });
    }
}

// ============================================================================
// LOOP TIMING EDGE CASES
// NOTE: These tests were disabled because getLoopTiming() was moved to ServerLoop class
// ============================================================================

/*
TEST_F(ServerAppUnitTest, LoopTiming_VeryLowTickRate) {
    ServerApp server(8080, 4, 1, shutdownFlag_, 30, false);  // 1 FPS

    auto timing = server.getLoopTiming();
    // At 1 FPS, fixed delta should be ~1 second
    EXPECT_GT(timing.fixedDeltaNs.count(), 900000000);  // > 900ms
    EXPECT_LT(timing.fixedDeltaNs.count(), 1100000000); // < 1100ms
}

TEST_F(ServerAppUnitTest, LoopTiming_VeryHighTickRate) {
    ServerApp server(8080, 4, 1000, shutdownFlag_, 30, false);  // 1000 FPS

    auto timing = server.getLoopTiming();
    // At 1000 FPS, fixed delta should be ~1ms
    EXPECT_GT(timing.fixedDeltaNs.count(), 900000);   // > 0.9ms
    EXPECT_LT(timing.fixedDeltaNs.count(), 1100000);  // < 1.1ms
}

TEST_F(ServerAppUnitTest, LoopTiming_StandardTickRate) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);  // 60 FPS

    auto timing = server.getLoopTiming();
    // At 60 FPS, fixed delta should be ~16.67ms
    EXPECT_GT(timing.fixedDeltaNs.count(), 15000000);  // > 15ms
    EXPECT_LT(timing.fixedDeltaNs.count(), 18000000);  // < 18ms
}
*/

// ============================================================================
// HAS GAME CONFIG TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, HasGameConfig_WithConfig) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_TRUE(server.hasGameConfig());
}

TEST_F(ServerAppUnitTest, HasGameConfig_WithoutConfig) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    EXPECT_FALSE(server.hasGameConfig());
}

// ============================================================================
// GET GAME CONFIG TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, GetGameConfig_WithConfig) {
    auto config = std::make_unique<MockGameConfigUnit>();
    config->setInitialized(true);

    GenericGameplaySettings gameplay;
    gameplay.playerSpeed = 500.0f;
    config->setGameplaySettings(gameplay);

    ServerApp server(std::move(config), shutdownFlag_, false);

    auto* gameConfig = server.getGameConfig();
    ASSERT_NE(gameConfig, nullptr);
    EXPECT_FLOAT_EQ(gameConfig->getGameplaySettings().playerSpeed, 500.0f);
}

TEST_F(ServerAppUnitTest, GetGameConfig_WithoutConfig) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    auto* gameConfig = server.getGameConfig();
    EXPECT_EQ(gameConfig, nullptr);
}

// ============================================================================
// PLAYER READY DUPLICATE TESTS
// ============================================================================

TEST_F(ServerAppUnitTest, PlayerReady_SamePlayerMultipleTimes) {
    ServerApp server(8080, 4, 60, shutdownFlag_, 30, false);

    // First player ready
    server.playerReady(1);
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);
    EXPECT_TRUE(server.isPlaying());

    // Same player again (game already playing)
    server.playerReady(1);
    server.playerReady(1);
    server.playerReady(1);

    // Count should still be 1
    EXPECT_EQ(server.getReadyPlayerCount(), 1u);
}

// ============================================================================
// RUN AND SHUTDOWN INTEGRATION
// ============================================================================

TEST_F(ServerAppUnitTest, Run_QuickShutdown) {
    ServerApp server(14260, 4, 60, shutdownFlag_, 30, false);

    // Start server in background
    std::thread serverThread([&]() {
        [[maybe_unused]] bool result = server.run();
    });

    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Signal shutdown
    server.stop();

    // Wait for thread
    serverThread.join();

    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppUnitTest, Run_WithPlayerReadyDuringRun) {
    ServerApp server(14261, 4, 60, shutdownFlag_, 30, false);

    std::thread serverThread([&]() {
        [[maybe_unused]] bool result = server.run();
    });

    // Let server initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Player ready during game
    server.playerReady(1);

    // Let game run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    server.stop();
    serverThread.join();

    EXPECT_TRUE(server.isPlaying());
}