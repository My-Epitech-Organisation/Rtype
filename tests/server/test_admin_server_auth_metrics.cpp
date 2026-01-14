#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "server/serverApp/ServerApp.hpp"
#include "server/lobby/LobbyManager.hpp"
#include "httplib.h"

using namespace rtype::server;

TEST(AdminServerAuthMetrics, Auth_Unauthorized_NoToken_Returns401) {
    AdminServer::Config cfg;
    cfg.port = 9205;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);

    auto res = cli.Get("/api/lobbies");
    ASSERT_NE(res, nullptr);
    // Without credentials, even localhost requests should be unauthorized
    EXPECT_EQ(res->status, 401);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerAuthMetrics, Metrics_NoServerApp_Returns500) {
    AdminServer::Config cfg;
    cfg.port = 9206;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Get("/api/metrics", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 500);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerAuthMetrics, GetBans_FromLobbyManager) {
    AdminServer::Config cfg;
    cfg.port = 9207;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    rtype::server::LobbyManager::Config lmCfg;
    lmCfg.basePort = 54200;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    rtype::server::LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());

    lm.getBanManager()->banIp("8.8.8.8", "", "testban");

    AdminServer server(cfg, nullptr, &lm);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Get("/api/bans", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("8.8.8.8"), std::string::npos);

    server.stop();
    lm.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerAuthMetrics, Unban_IpOnly_RemovesBan) {
    AdminServer::Config cfg;
    cfg.port = 9208;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    rtype::server::LobbyManager::Config lmCfg;
    lmCfg.basePort = 54300;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    rtype::server::LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());

    lm.getBanManager()->banIp("7.7.7.7", "", "testban");

    AdminServer server(cfg, nullptr, &lm);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/unban", goodAuth, "{\"ip\": \"7.7.7.7\"}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto bans = lm.getBanManager()->getBannedList();
    bool found = false;
    for (const auto& b : bans) {
        if (b.ip == "7.7.7.7") { found = true; break; }
    }
    EXPECT_FALSE(found);

    server.stop();
    lm.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerAuthMetrics, Players_NoLobbies_ReturnsEmpty) {
    AdminServer::Config cfg;
    cfg.port = 9209;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    rtype::server::LobbyManager::Config lmCfg;
    lmCfg.basePort = 54400;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    rtype::server::LobbyManager lm(lmCfg);
    // Do not start the manager so getAllLobbies will return empty

    AdminServer server(cfg, nullptr, &lm);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Get("/api/players", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"players\":[]"), std::string::npos);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}
