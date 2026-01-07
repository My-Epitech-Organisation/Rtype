/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ServerApp conditional branch coverage tests
*/

#include <gtest/gtest.h>

#include "server/serverApp/ServerApp.hpp"
#include "server/shared/IGameConfig.hpp"

#include <atomic>
#include <memory>

using namespace rtype::server;

// Test various parameter combinations for branch coverage
TEST(ServerAppBranchCoverage, MinimalValidPort) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(1024, 4, 60, shutdown, 30, false);
    
    EXPECT_TRUE(app.isRunning()); // isRunning returns !shutdownFlag
}

TEST(ServerAppBranchCoverage, MaximalValidPort) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(65535, 4, 60, shutdown, 30, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, VerboseMode) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 30, true);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, NonVerboseMode) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 30, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, SinglePlayer) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 1, 60, shutdown, 30, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, ManyPlayers) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 16, 60, shutdown, 30, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, LowTickRate) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 10, shutdown, 30, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, HighTickRate) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 120, shutdown, 30, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, ShortTimeout) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 5, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, LongTimeout) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 300, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, DifferentShutdownFlagInstances) {
    {
        auto shutdown1 = std::make_shared<std::atomic<bool>>(false);
        ServerApp app(4242, 4, 60, shutdown1, 30, false);
    }
    {
        auto shutdown2 = std::make_shared<std::atomic<bool>>(true);
        ServerApp app(4242, 4, 60, shutdown2, 30, false);
    }
}

// Test getter methods in different contexts
TEST(ServerAppBranchCoverage, GettersBeforeInitialization) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 8, 60, shutdown, 30, false);
    
    EXPECT_EQ(app.getConnectedClientCount(), 0u);
    EXPECT_TRUE(app.getConnectedClientIds().empty());
}

TEST(ServerAppBranchCoverage, GetClientInfoWithNoClients) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 30, false);
    
    auto info = app.getClientInfo(12345);
    EXPECT_FALSE(info.has_value());
}

TEST(ServerAppBranchCoverage, MultipleGetterCalls) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 30, false);
    
    // Call getters multiple times to exercise branches
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(app.getConnectedClientCount(), 0u);
        EXPECT_TRUE(app.getConnectedClientIds().empty());
        EXPECT_TRUE(app.isRunning());
    }
}

// Test stop behavior in different states
TEST(ServerAppBranchCoverage, StopBeforeStart) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 30, false);
    
    app.stop(); // Should not crash
    EXPECT_FALSE(app.isRunning()); // After stop, shutdown flag is true
}

TEST(ServerAppBranchCoverage, MultipleStops) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 30, false);
    
    app.stop();
    app.stop();
    app.stop();
    EXPECT_FALSE(app.isRunning());
}

// Test different parameter edge cases
TEST(ServerAppBranchCoverage, EdgeCaseTickRate) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 1, shutdown, 30, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, EdgeCaseMaxPlayers) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 2, 60, shutdown, 30, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, EdgeCaseTimeout) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 1, false);
    
    EXPECT_TRUE(app.isRunning());
}

// Test constructor with combinations
TEST(ServerAppBranchCoverage, CombinationLowTickRateVerbose) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 20, shutdown, 30, true);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, CombinationHighTickRateManyPlayers) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 12, 100, shutdown, 60, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, CombinationAllExtreme) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(65535, 1, 1, shutdown, 1, true);
    
    EXPECT_TRUE(app.isRunning());
}

// Mock game config implementation
class MockGameConfig : public IGameConfig {
private:
    bool initialized_;
    std::string gameId_;
    GenericServerSettings settings_;
    std::string lastError_;

public:
    MockGameConfig(bool init, const std::string& id = "test", uint16_t port = 4000, 
                   uint32_t tick = 60, size_t maxPlayers = 4)
        : initialized_(init), gameId_(id) {
        settings_.port = port;
        settings_.tickRate = tick;
        settings_.maxPlayers = maxPlayers;
    }

    bool initialize(const std::string&) override { return true; }
    bool reloadConfiguration() override { return true; }
    bool isInitialized() const noexcept override { return initialized_; }
    
    GenericServerSettings getServerSettings() const noexcept override { return settings_; }
    GenericGameplaySettings getGameplaySettings() const noexcept override { return {}; }
    std::string getSavesPath() const noexcept override { return ""; }
    
    bool saveGame(const std::string&, const std::vector<uint8_t>&) override { return false; }
    std::vector<uint8_t> loadGame(const std::string&) override { return {}; }
    std::vector<GenericSaveInfo> listSaves() const override { return {}; }
    bool saveExists(const std::string&) const override { return false; }
    bool deleteSave(const std::string&) override { return false; }
    
    const std::string& getLastError() const noexcept override { return lastError_; }
    std::string getGameId() const noexcept override { return gameId_; }
};

TEST(ServerAppBranchCoverage, GameConfigConstructorInitialized) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    auto config = std::make_unique<MockGameConfig>(true, "rtype", 4242, 60, 8);
    ServerApp app(std::move(config), shutdown, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, GameConfigConstructorNotInitialized) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    auto config = std::make_unique<MockGameConfig>(false);
    ServerApp app(std::move(config), shutdown, false);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, GameConfigConstructorVerbose) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    auto config = std::make_unique<MockGameConfig>(true, "test", 5000, 120, 16);
    ServerApp app(std::move(config), shutdown, true);
    
    EXPECT_TRUE(app.isRunning());
}

TEST(ServerAppBranchCoverage, GameConfigConstructorDifferentValues) {
    {
        auto shutdown = std::make_shared<std::atomic<bool>>(false);
        auto config = std::make_unique<MockGameConfig>(true, "game1", 3000, 30, 2);
        ServerApp app(std::move(config), shutdown, false);
    }
    {
        auto shutdown = std::make_shared<std::atomic<bool>>(false);
        auto config = std::make_unique<MockGameConfig>(true, "game2", 7000, 90, 10);
        ServerApp app(std::move(config), shutdown, true);
    }
}

// Test isRunning in various contexts
TEST(ServerAppBranchCoverage, IsRunningMultipleCalls) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 30, false);
    
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(app.isRunning());
    }
}

// Test destructor with different states
TEST(ServerAppBranchCoverage, DestructorAfterConstruction) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    {
        ServerApp app(4242, 4, 60, shutdown, 30, false);
        // Destructor will be called here
    }
    EXPECT_FALSE(shutdown->load());
}

TEST(ServerAppBranchCoverage, DestructorAfterStop) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    {
        ServerApp app(4242, 4, 60, shutdown, 30, false);
        app.stop();
        // Destructor will be called here
    }
}

// Test with various client manager states
TEST(ServerAppBranchCoverage, GetConnectedClientsRepeated) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 30, false);
    
    // Multiple calls to exercise all branches
    EXPECT_EQ(app.getConnectedClientCount(), 0u);
    EXPECT_EQ(app.getConnectedClientCount(), 0u);
    EXPECT_EQ(app.getConnectedClientCount(), 0u);
}

TEST(ServerAppBranchCoverage, GetConnectedClientIdsRepeated) {
    auto shutdown = std::make_shared<std::atomic<bool>>(false);
    ServerApp app(4242, 4, 60, shutdown, 30, false);
    
    auto ids1 = app.getConnectedClientIds();
    auto ids2 = app.getConnectedClientIds();
    auto ids3 = app.getConnectedClientIds();
    
    EXPECT_TRUE(ids1.empty());
    EXPECT_TRUE(ids2.empty());
    EXPECT_TRUE(ids3.empty());
}
