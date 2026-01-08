#include <gtest/gtest.h>

#include "server/serverApp/ServerApp.hpp"
#include "server/shared/IGameConfig.hpp"

using namespace rtype::server;

class FakeGameConfig : public IGameConfig {
public:
    GenericServerSettings settings;
    bool initialized;
    bool reloadSuc;
    uint16_t portOnReload{0};

    FakeGameConfig(bool init, bool reload, uint16_t initialPort = 4000, uint16_t newPort = 0)
        : initialized(init), reloadSuc(reload), portOnReload(newPort) {
        settings.port = initialPort;
        settings.maxPlayers = 4;
        settings.tickRate = 60;
    }

    bool initialize(const std::string& /*configDir*/) override { return initialized; }
    bool reloadConfiguration() override {
        if (portOnReload != 0) settings.port = portOnReload;
        return reloadSuc;
    }
    bool isInitialized() const noexcept override { return initialized; }
    GenericServerSettings getServerSettings() const noexcept override { return settings; }
    GenericGameplaySettings getGameplaySettings() const noexcept override { return GenericGameplaySettings(); }
    std::string getSavesPath() const noexcept override { return ""; }
    bool saveGame(const std::string&, const std::vector<uint8_t>&) override { return false; }
    std::vector<uint8_t> loadGame(const std::string&) override { return {}; }
    std::vector<GenericSaveInfo> listSaves() const override { return {}; }
    bool saveExists(const std::string&) const override { return false; }
    bool deleteSave(const std::string&) override { return false; }
    const std::string& getLastError() const noexcept override { static std::string s; return s; }
    std::string getGameId() const noexcept override { return "testgame"; }
};

TEST(ServerAppReloadTest, Reload_NoGameConfig_ReturnsFalse) {
    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    ServerApp sa(nullptr, shutdownFlag, false);
    EXPECT_FALSE(sa.reloadConfiguration());
}

TEST(ServerAppReloadTest, Reload_GameConfigNotInitialized_ReturnsFalse) {
    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    auto cfg = std::make_unique<FakeGameConfig>(false, true);
    ServerApp sa(std::move(cfg), shutdownFlag, false);
    EXPECT_FALSE(sa.reloadConfiguration());
}

TEST(ServerAppReloadTest, Reload_ReloadFails_ReturnsFalse) {
    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    auto cfg = std::make_unique<FakeGameConfig>(true, false);
    ServerApp sa(std::move(cfg), shutdownFlag, false);
    EXPECT_FALSE(sa.reloadConfiguration());
}

TEST(ServerAppReloadTest, Reload_Succeeds_PortChange_LogsAndReturnsTrue) {
    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    // initial port 4000, on reload change to 5000
    auto cfg = std::make_unique<FakeGameConfig>(true, true, 4000, 5000);
    ServerApp sa(std::move(cfg), shutdownFlag, false);
    EXPECT_TRUE(sa.reloadConfiguration());
}
