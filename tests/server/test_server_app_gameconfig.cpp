/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Additional tests for ServerApp - GameConfig integration
*/

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include "../../../src/server/ServerApp.hpp"
#include "../../../src/server/IGameConfig.hpp"

using namespace rtype::server;

// ============================================================================
// MOCK GAME CONFIG
// ============================================================================

class MockGameConfig : public IGameConfig {
   public:
    MockGameConfig() = default;

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

class ServerAppGameConfigTest : public ::testing::Test {
   protected:
    void SetUp() override {
        shutdownFlag_ = std::make_shared<std::atomic<bool>>(false);
    }

    void TearDown() override { shutdownFlag_->store(true); }

    std::shared_ptr<std::atomic<bool>> shutdownFlag_;
};

// ============================================================================
// GAME CONFIG CONSTRUCTOR TESTS
// ============================================================================

TEST_F(ServerAppGameConfigTest, Constructor_WithInitializedConfig) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    GenericServerSettings serverSettings;
    serverSettings.port = 5000;
    serverSettings.maxPlayers = 16;
    serverSettings.tickRate = 120;
    config->setServerSettings(serverSettings);

    config->setGameId("test_game");

    EXPECT_NO_THROW({
        ServerApp server(std::move(config), shutdownFlag_, false);
    });
}

TEST_F(ServerAppGameConfigTest, Constructor_WithUninitializedConfig) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(false);

    EXPECT_NO_THROW({
        ServerApp server(std::move(config), shutdownFlag_, false);
    });
}

TEST_F(ServerAppGameConfigTest, Constructor_WithNullConfig) {
    EXPECT_NO_THROW({
        ServerApp server(nullptr, shutdownFlag_, false);
    });
}

TEST_F(ServerAppGameConfigTest, Constructor_ConfigUsesCorrectPort) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    GenericServerSettings serverSettings;
    serverSettings.port = 7777;
    serverSettings.maxPlayers = 8;
    serverSettings.tickRate = 60;
    config->setServerSettings(serverSettings);

    ServerApp server(std::move(config), shutdownFlag_, false);
    // Server created with config's port
    EXPECT_TRUE(server.isRunning());
}

TEST_F(ServerAppGameConfigTest, Constructor_ConfigUsesCorrectMaxPlayers) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    GenericServerSettings serverSettings;
    serverSettings.port = 4000;
    serverSettings.maxPlayers = 32;
    serverSettings.tickRate = 60;
    config->setServerSettings(serverSettings);

    ServerApp server(std::move(config), shutdownFlag_, false);
    EXPECT_EQ(server.getClientManager().getMaxPlayers(), 32);
}

TEST_F(ServerAppGameConfigTest, Constructor_VerboseMode) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    EXPECT_NO_THROW({
        ServerApp server(std::move(config), shutdownFlag_, true);
    });
}

// ============================================================================
// GET GAME CONFIG TESTS
// ============================================================================

TEST_F(ServerAppGameConfigTest, GetGameConfig_ReturnsConfig) {
    auto config = std::make_unique<MockGameConfig>();
    auto configPtr = config.get();
    config->setInitialized(true);
    config->setGameId("test_game");

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_NE(server.getGameConfig(), nullptr);
    EXPECT_EQ(server.getGameConfig()->getGameId(), "test_game");
}

TEST_F(ServerAppGameConfigTest, GetGameConfig_Const_ReturnsConfig) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);
    config->setGameId("const_test");

    const ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_NE(server.getGameConfig(), nullptr);
    EXPECT_EQ(server.getGameConfig()->getGameId(), "const_test");
}

TEST_F(ServerAppGameConfigTest, GetGameConfig_NullConfig_ReturnsNullptr) {
    ServerApp server(nullptr, shutdownFlag_, false);
    EXPECT_EQ(server.getGameConfig(), nullptr);
}

// ============================================================================
// HAS GAME CONFIG TESTS
// ============================================================================

TEST_F(ServerAppGameConfigTest, HasGameConfig_WithInitializedConfig_ReturnsTrue) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    ServerApp server(std::move(config), shutdownFlag_, false);
    EXPECT_TRUE(server.hasGameConfig());
}

TEST_F(ServerAppGameConfigTest, HasGameConfig_WithUninitializedConfig_ReturnsFalse) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(false);

    ServerApp server(std::move(config), shutdownFlag_, false);
    EXPECT_FALSE(server.hasGameConfig());
}

TEST_F(ServerAppGameConfigTest, HasGameConfig_WithNullConfig_ReturnsFalse) {
    ServerApp server(nullptr, shutdownFlag_, false);
    EXPECT_FALSE(server.hasGameConfig());
}

// ============================================================================
// RELOAD CONFIGURATION TESTS
// ============================================================================

TEST_F(ServerAppGameConfigTest, ReloadConfiguration_Success) {
    auto config = std::make_unique<MockGameConfig>();
    auto configPtr = config.get();
    config->setInitialized(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_TRUE(server.reloadConfiguration());
    EXPECT_EQ(configPtr->getReloadCount(), 1);
}

TEST_F(ServerAppGameConfigTest, ReloadConfiguration_Failure) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);
    config->setShouldFailReload(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_FALSE(server.reloadConfiguration());
}

TEST_F(ServerAppGameConfigTest, ReloadConfiguration_NoConfig_ReturnsFalse) {
    ServerApp server(nullptr, shutdownFlag_, false);
    EXPECT_FALSE(server.reloadConfiguration());
}

TEST_F(ServerAppGameConfigTest, ReloadConfiguration_UninitializedConfig_ReturnsFalse) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(false);

    ServerApp server(std::move(config), shutdownFlag_, false);
    EXPECT_FALSE(server.reloadConfiguration());
}

TEST_F(ServerAppGameConfigTest, ReloadConfiguration_MultipleTimes) {
    auto config = std::make_unique<MockGameConfig>();
    auto configPtr = config.get();
    config->setInitialized(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    EXPECT_TRUE(server.reloadConfiguration());
    EXPECT_TRUE(server.reloadConfiguration());
    EXPECT_TRUE(server.reloadConfiguration());
    EXPECT_EQ(configPtr->getReloadCount(), 3);
}

// ============================================================================
// LOOP TIMING TESTS
// ============================================================================

TEST_F(ServerAppGameConfigTest, GetLoopTiming_FromConfig) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    GenericServerSettings serverSettings;
    serverSettings.port = 4000;
    serverSettings.maxPlayers = 8;
    serverSettings.tickRate = 120;  // 120 Hz
    config->setServerSettings(serverSettings);

    ServerApp server(std::move(config), shutdownFlag_, false);

    auto timing = server.getLoopTiming();
    // 120 Hz = ~8.33ms per tick
    auto expectedNs = std::chrono::nanoseconds(
        static_cast<int64_t>(1e9 / 120.0));
    // Allow some tolerance
    EXPECT_NEAR(timing.fixedDeltaNs.count(), expectedNs.count(), 100);
}

TEST_F(ServerAppGameConfigTest, GetLoopTiming_DefaultTickRate) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(false);  // Will use default 60 Hz

    ServerApp server(std::move(config), shutdownFlag_, false);

    auto timing = server.getLoopTiming();
    // 60 Hz = ~16.67ms per tick
    auto expectedNs = std::chrono::nanoseconds(
        static_cast<int64_t>(1e9 / 60.0));
    EXPECT_NEAR(timing.fixedDeltaNs.count(), expectedNs.count(), 100);
}

// ============================================================================
// CLIENT MANAGER INTEGRATION TESTS
// ============================================================================

TEST_F(ServerAppGameConfigTest, GetClientManager_NonConst) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    GenericServerSettings serverSettings;
    serverSettings.port = 4000;
    serverSettings.maxPlayers = 10;
    serverSettings.tickRate = 60;
    config->setServerSettings(serverSettings);

    ServerApp server(std::move(config), shutdownFlag_, false);

    ClientManager& cm = server.getClientManager();
    EXPECT_EQ(cm.getMaxPlayers(), 10);
}

TEST_F(ServerAppGameConfigTest, GetClientManager_Const) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    GenericServerSettings serverSettings;
    serverSettings.port = 4000;
    serverSettings.maxPlayers = 6;
    serverSettings.tickRate = 60;
    config->setServerSettings(serverSettings);

    const ServerApp server(std::move(config), shutdownFlag_, false);

    const ClientManager& cm = server.getClientManager();
    EXPECT_EQ(cm.getMaxPlayers(), 6);
}

// ============================================================================
// METRICS TESTS
// ============================================================================

TEST_F(ServerAppGameConfigTest, GetMetrics_WithConfig) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    const ServerMetrics& metrics = server.getMetrics();
    EXPECT_EQ(metrics.totalConnections.load(), 0);
    EXPECT_EQ(metrics.tickOverruns.load(), 0);
}

// ============================================================================
// RUN INTEGRATION TESTS
// ============================================================================

TEST_F(ServerAppGameConfigTest, Run_ShutdownImmediately) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    // Set shutdown flag before run
    shutdownFlag_->store(true);

    // Run should return quickly since shutdown is already set
    std::thread serverThread([&]() {
        server.run();
    });

    // Wait a bit and check if thread finished
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    serverThread.join();

    EXPECT_FALSE(server.isRunning());
}

TEST_F(ServerAppGameConfigTest, Run_StopFromAnotherThread) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    ServerApp server(std::move(config), shutdownFlag_, false);

    std::thread serverThread([&]() {
        server.run();
    });

    // Let server run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Stop from main thread
    server.stop();

    serverThread.join();
    EXPECT_FALSE(server.isRunning());
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST_F(ServerAppGameConfigTest, Constructor_DefaultClientTimeout) {
    auto config = std::make_unique<MockGameConfig>();
    config->setInitialized(true);

    // The DEFAULT_CLIENT_TIMEOUT_SECONDS should be used
    ServerApp server(std::move(config), shutdownFlag_, false);
    // Just verify construction doesn't throw
    EXPECT_TRUE(server.hasGameConfig());
}

TEST_F(ServerAppGameConfigTest, MultipleServers_SameShutdownFlag) {
    auto config1 = std::make_unique<MockGameConfig>();
    config1->setInitialized(true);
    GenericServerSettings settings1;
    settings1.port = 4001;
    config1->setServerSettings(settings1);

    auto config2 = std::make_unique<MockGameConfig>();
    config2->setInitialized(true);
    GenericServerSettings settings2;
    settings2.port = 4002;
    config2->setServerSettings(settings2);

    ServerApp server1(std::move(config1), shutdownFlag_, false);
    ServerApp server2(std::move(config2), shutdownFlag_, false);

    EXPECT_TRUE(server1.isRunning());
    EXPECT_TRUE(server2.isRunning());

    // Stopping one stops both (same flag)
    server1.stop();

    EXPECT_FALSE(server1.isRunning());
    EXPECT_FALSE(server2.isRunning());
}
