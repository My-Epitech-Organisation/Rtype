#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "server/lobby/LobbyManager.hpp"
#include "server/main.hpp"
#include "httplib.h"
#include <filesystem>

using namespace rtype::server;

TEST(AdminServerLobbyAPI, Delete_LastInstance_NoForce_Returns409) {
    AdminServer::Config cfg;
    cfg.port = 9250;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    LobbyManager::Config lmCfg;
    lmCfg.basePort = 56000;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto lobbies = lm.getAllLobbies();
    ASSERT_EQ(lobbies.size(), 1u);
    auto code = lobbies[0]->getCode();

    AdminServer admin(cfg, nullptr, &lm);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post((std::string("/api/lobby/") + code + "/delete").c_str(), goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 409);

    // Lobby should still exist
    auto after = lm.getAllLobbies();
    EXPECT_EQ(after.size(), 1u);

    admin.stop();
    lm.stop();
}

TEST(AdminServerLobbyAPI, Delete_LastInstance_Force_RequestsShutdownAndPreservesLobby) {
    AdminServer::Config cfg;
    cfg.port = 9251;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    LobbyManager::Config lmCfg;
    lmCfg.basePort = 56100;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto lobbies = lm.getAllLobbies();
    ASSERT_EQ(lobbies.size(), 1u);
    auto code = lobbies[0]->getCode();

    // Ensure shutdown flag is clear at test start
    ServerSignals::shutdown()->store(false);

    AdminServer admin(cfg, nullptr, &lm);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post((std::string("/api/lobby/") + code + "/delete?force=1").c_str(), goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    // Lobby should still exist (we preserved it)
    auto after = lm.getAllLobbies();
    EXPECT_EQ(after.size(), 1u);

    // And the server shutdown flag should be set
    EXPECT_TRUE(ServerSignals::shutdown()->load());

    admin.stop();
    lm.stop();
}
