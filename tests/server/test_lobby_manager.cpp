#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "server/lobby/LobbyManager.hpp"

using namespace rtype::server;

TEST(LobbyManagerTest, CreateAndDeleteLobbies) {
    LobbyManager::Config cfg;
    cfg.basePort = 43000; // avoid low ports
    cfg.instanceCount = 1; // must be at least 1
    cfg.maxInstances = 4;

    LobbyManager manager(cfg);

    // Create private lobby
    auto code1 = manager.createLobby(true);
    EXPECT_FALSE(code1.empty());
    // private codes should contain non-digit chars (alphanumeric)
    bool hasAlpha = std::any_of(code1.begin(), code1.end(), [](char c){ return !std::isdigit((unsigned char)c); });
    EXPECT_TRUE(hasAlpha);

    // Create public lobby
    auto code2 = manager.createLobby(false);
    EXPECT_FALSE(code2.empty());
    // public code should be numeric and exactly 6 digits (zero-padded)
    bool allDigits = std::all_of(code2.begin(), code2.end(), [](char c){ return std::isdigit((unsigned char)c); });
    EXPECT_TRUE(allDigits);
    EXPECT_EQ(code2.size(), 6);

    // Delete both
    EXPECT_TRUE(manager.deleteLobby(code1));
    EXPECT_TRUE(manager.deleteLobby(code2));
}

TEST(LobbyManagerTest, FindVerifyAndActiveList) {
    LobbyManager::Config cfg;
    cfg.basePort = 43100; // avoid low ports used by other tests
    cfg.instanceCount = 1;
    cfg.maxInstances = 4;

    LobbyManager manager(cfg);

    auto code = manager.createLobby(true);
    ASSERT_FALSE(code.empty());

    // Give the lobby a brief moment to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    Lobby* l = manager.findLobbyByCode(code);
    ASSERT_NE(l, nullptr);

    EXPECT_TRUE(manager.verifyLobbyCode(code, l->getPort()));

    auto active = manager.getActiveLobbyList();
    EXPECT_FALSE(active.empty());

    bool found = std::any_of(active.begin(), active.end(), [&](const LobbyManager::LobbyInfo& info){ return info.code == code; });
    EXPECT_TRUE(found);

    // Clean up
    EXPECT_TRUE(manager.deleteLobby(code));
    EXPECT_EQ(manager.findLobbyByCode(code), nullptr);
}

TEST(LobbyManagerTest, CreateUpToMaxAndRejectExtra) {
    LobbyManager::Config cfg;
    cfg.basePort = 43200;
    cfg.instanceCount = 1;
    cfg.maxInstances = 2;

    LobbyManager manager(cfg);

    auto c1 = manager.createLobby(true);
    ASSERT_FALSE(c1.empty());

    auto c2 = manager.createLobby(true);
    ASSERT_FALSE(c2.empty());

    // Third should be rejected due to maxInstances
    auto c3 = manager.createLobby(true);
    EXPECT_EQ(c3, "");

    // Deleting non-existent lobby returns false
    EXPECT_FALSE(manager.deleteLobby("NOPE"));

    // Clean up
    EXPECT_TRUE(manager.deleteLobby(c1));
    EXPECT_TRUE(manager.deleteLobby(c2));
}

TEST(LobbyManagerTest, VerifyLobbyCodeWrongPort) {
    LobbyManager::Config cfg;
    cfg.basePort = 43300;
    cfg.instanceCount = 1;
    cfg.maxInstances = 2;

    LobbyManager manager(cfg);

    auto code = manager.createLobby(true);
    ASSERT_FALSE(code.empty());

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    Lobby* l = manager.findLobbyByCode(code);
    ASSERT_NE(l, nullptr);

    // Correct code, correct port should verify
    EXPECT_TRUE(manager.verifyLobbyCode(code, l->getPort()));

    // Correct code, wrong port should NOT verify
    EXPECT_FALSE(manager.verifyLobbyCode(code, l->getPort() + 100));

    // Wrong code should NOT verify
    EXPECT_FALSE(manager.verifyLobbyCode("BADCODE", l->getPort()));

    EXPECT_TRUE(manager.deleteLobby(code));
}

TEST(LobbyManagerTest, FindNonExistentLobby) {
    LobbyManager::Config cfg;
    cfg.basePort = 43400;
    cfg.instanceCount = 1;
    cfg.maxInstances = 2;

    LobbyManager manager(cfg);

    // Finding a non-existent lobby should return nullptr
    EXPECT_EQ(manager.findLobbyByCode("NOTFOUND"), nullptr);
    EXPECT_EQ(manager.findLobbyByCode(""), nullptr);
    EXPECT_EQ(manager.findLobbyByCode("123456"), nullptr);
}

TEST(LobbyManagerTest, InvalidConfigThrows) {
    // Zero instanceCount should throw
    LobbyManager::Config cfg1;
    cfg1.basePort = 43500;
    cfg1.instanceCount = 0;
    cfg1.maxInstances = 4;
    EXPECT_THROW((LobbyManager{cfg1}), std::invalid_argument);

    // instanceCount > maxInstances should throw
    LobbyManager::Config cfg2;
    cfg2.basePort = 43600;
    cfg2.instanceCount = 10;
    cfg2.maxInstances = 5;
    EXPECT_THROW((LobbyManager{cfg2}), std::invalid_argument);
}

TEST(LobbyManagerTest, GetBanManager) {
    LobbyManager::Config cfg;
    cfg.basePort = 43700;
    cfg.instanceCount = 1;
    cfg.maxInstances = 2;

    LobbyManager manager(cfg);

    auto banManager = manager.getBanManager();
    ASSERT_NE(banManager, nullptr);

    // Use the ban manager to ban an IP
    banManager->banIp("1.2.3.4", "testPlayer", "testReason");
    EXPECT_TRUE(banManager->isIpBanned("1.2.3.4"));

    // Clear the bans
    banManager->clearAllBans();
    EXPECT_FALSE(banManager->isIpBanned("1.2.3.4"));
}

TEST(LobbyManagerTest, MultiplePublicLobbyCodes) {
    LobbyManager::Config cfg;
    cfg.basePort = 43800;
    cfg.instanceCount = 1;
    cfg.maxInstances = 10;

    LobbyManager manager(cfg);

    std::vector<std::string> codes;
    for (int i = 0; i < 5; ++i) {
        auto code = manager.createLobby(false);  // public lobbies
        ASSERT_FALSE(code.empty());
        // All public codes should be 6-digit numeric
        EXPECT_EQ(code.size(), 6);
        bool allDigits = std::all_of(code.begin(), code.end(), 
            [](char c) { return std::isdigit(static_cast<unsigned char>(c)); });
        EXPECT_TRUE(allDigits);
        codes.push_back(code);
    }

    // All codes should be unique
    std::sort(codes.begin(), codes.end());
    auto last = std::unique(codes.begin(), codes.end());
    EXPECT_EQ(last, codes.end());

    // Clean up
    for (const auto& code : codes) {
        EXPECT_TRUE(manager.deleteLobby(code));
    }
}

TEST(LobbyManagerTest, DeleteSameLobbyTwice) {
    LobbyManager::Config cfg;
    cfg.basePort = 43900;
    cfg.instanceCount = 1;
    cfg.maxInstances = 2;

    LobbyManager manager(cfg);

    auto code = manager.createLobby(true);
    ASSERT_FALSE(code.empty());

    // First delete should succeed
    EXPECT_TRUE(manager.deleteLobby(code));

    // Second delete should fail (already deleted)
    EXPECT_FALSE(manager.deleteLobby(code));
}
