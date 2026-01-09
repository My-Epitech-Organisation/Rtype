#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "server/serverApp/ServerApp.hpp"
#include "server/lobby/LobbyManager.hpp"
#include "httplib.h"

using namespace rtype::server;

TEST(AdminServerBranches, Ban_EmptyBody_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9191;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/ban", goodAuth, "{}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerBranches, Ban_ClientIdNotFound_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9192;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    rtype::server::ServerApp sa(1234, 4, 60, shutdownFlag, 10, false);

    AdminServer server(cfg, &sa, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/ban", goodAuth, "{\"clientId\": 99999}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerBranches, Ban_IpAndPort_BansEndpoint) {
    AdminServer::Config cfg;
    cfg.port = 9193;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    rtype::server::ServerApp sa(1234, 4, 60, shutdownFlag, 10, false);

    AdminServer server(cfg, &sa, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/ban", goodAuth, "{\"ip\": \"9.9.9.9\", \"port\": 4321}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto bans = sa.getBanManager().getBannedList();
    bool found = false;
    for (const auto& b : bans) {
        if (b.ip == "9.9.9.9" && b.port == 4321) { found = true; break; }
    }
    EXPECT_TRUE(found);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerBranches, Lobbies_ListIncludesPublicAndPrivate) {
    AdminServer::Config cfg;
    cfg.port = 9291;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    // Start a LobbyManager with one instance
    rtype::server::LobbyManager::Config lmCfg;
    lmCfg.basePort = 54000; // high ephemeral port range
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    rtype::server::LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());

    AdminServer server(cfg, nullptr, &lm);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    // Initial call should contain at least one lobby
    auto res = cli.Get("/api/lobbies", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"lobbies\":"), std::string::npos);

    // Create a public lobby (numeric code)
    auto publicCode = lm.createLobby(false);
    ASSERT_FALSE(publicCode.empty());

    // Wait a bit for the lobby to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    res = cli.Get("/api/lobbies", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    // Expect at least one public lobby (isPublic:true) and one private lobby (isPublic:false)
    EXPECT_NE(res->body.find("\"isPublic\":true"), std::string::npos);
    EXPECT_NE(res->body.find("\"isPublic\":false"), std::string::npos);

    server.stop();
    lm.stop();
    EXPECT_FALSE(server.isRunning());
}
