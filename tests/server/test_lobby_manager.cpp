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
