#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "server/lobby/LobbyManager.hpp"
#include "server/serverApp/ServerApp.hpp"
#include "httplib.h"
#include <filesystem>

using namespace rtype::server;

TEST(AdminServerLobbyAPI, Create_WithManager_ReturnsCode) {
    AdminServer::Config cfg;
    cfg.port = 9220;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    LobbyManager::Config lmCfg;
    lmCfg.basePort = 54700;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    AdminServer admin(cfg, nullptr, &lm);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/lobby/create", goodAuth, R"({"isPublic":true})", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"success\":true"), std::string::npos);
    EXPECT_NE(res->body.find("\"code\":"), std::string::npos);

    // Created code should appear in /api/lobbies
    res = cli.Get("/api/lobbies", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"isPublic\":true"), std::string::npos);

    admin.stop();
    lm.stop();
}

TEST(AdminServerLobbyAPI, Lobbies_NoManager_Empty) {
    AdminServer::Config cfg;
    cfg.port = 9224;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer admin(cfg, nullptr, nullptr);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Get("/api/lobbies", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"lobbies\":[]"), std::string::npos);

    // players list for a code should be empty
    res = cli.Get("/api/lobbies/NOPE/players", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"players\":[]"), std::string::npos);

    admin.stop();
}

TEST(AdminServerLobbyAPI, Metrics_NoLobbyManager_ReturnsBase) {
    AdminServer::Config cfg;
    cfg.port = 9225;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    ServerApp baseSa(8300, 4, 60, shutdownFlag, 10, false);
    {
        auto& baseMetrics = const_cast<ServerMetrics&>(baseSa.getMetrics());
        baseMetrics.packetsReceived.store(13, std::memory_order_relaxed);
        baseMetrics.packetsSent.store(7, std::memory_order_relaxed);
    }

    AdminServer admin(cfg, &baseSa, nullptr);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Get("/api/metrics", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"packetsReceived\":13"), std::string::npos);
    EXPECT_NE(res->body.find("\"packetsSent\":7"), std::string::npos);

    admin.stop();
}

TEST(AdminServerLobbyAPI, Create_NoManager_Returns500) {
    AdminServer::Config cfg;
    cfg.port = 9221;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer admin(cfg, nullptr, nullptr);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/lobby/create", goodAuth, R"({"isPublic":false})", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 500);

    admin.stop();
}

TEST(AdminServerAdminPage, ServesHtmlDashboard) {
    AdminServer::Config cfg;
    cfg.port = 9231;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    // Temporarily change CWD to repo root so AdminServer can find assets/admin.html reliably
    std::filesystem::path origCwd = std::filesystem::current_path();
    auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    std::filesystem::current_path(repoRoot);

    AdminServer admin(cfg, nullptr, nullptr);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);

    // Retry briefly to avoid race where server starts but asset isn't ready yet
    // Use the server-generated admin credentials to form a Basic auth header.
    auto user = admin.getAdminUserForTests();
    auto pass = admin.getAdminPassForTests();
    auto toEncode = user + ":" + pass;
    auto base64_encode = [](const std::string &in) {
        static const std::string b64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        int val=0, valb=-6;
        for (unsigned char c : in) {
            val = (val<<8) + c;
            valb += 8;
            while (valb>=0) {
                out.push_back(b64_chars[(val>>valb)&0x3F]);
                valb-=6;
            }
        }
        if (valb>-6) out.push_back(b64_chars[((val<<8)>>(valb+8))&0x3F]);
        while (out.size()%4) out.push_back('=');
        return out;
    };
    httplib::Headers auth{{"Authorization", std::string("Basic ") + base64_encode(toEncode)}};

    decltype(cli.Get("/admin", auth)) res{};
    for (int i = 0; i < 50; ++i) {
        res = cli.Get("/admin", auth);
        if (res && res->status == 200 && res->body.find("<html") != std::string::npos) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_TRUE(res);
    // The asset file exists in repo; should return HTML
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("<html"), std::string::npos);

    admin.stop();
    std::filesystem::current_path(origCwd);

}

TEST(AdminServerMetrics, AggregatesLobbyMetrics) {
    AdminServer::Config cfg;
    cfg.port = 9232;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    // Base server metrics
    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    ServerApp baseSa(8400, 4, 60, shutdownFlag, 10, false);
    {
        auto& baseMetrics = const_cast<ServerMetrics&>(baseSa.getMetrics());
        baseMetrics.packetsReceived.store(2, std::memory_order_relaxed);
        baseMetrics.packetsSent.store(3, std::memory_order_relaxed);
    }

    // Lobby manager with one lobby whose server has metrics
    LobbyManager::Config lmCfg;
    lmCfg.basePort = 55300;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto lobbies = lm.getAllLobbies();
    ASSERT_GE(lobbies.size(), 1u);
    auto* lobby = lobbies[0];
    ASSERT_NE(lobby, nullptr);
    auto* ls = lobby->getServerApp();
    ASSERT_NE(ls, nullptr);

    // Set lobby metrics
    const_cast<ServerMetrics&>(ls->getMetrics()).packetsReceived.store(5, std::memory_order_relaxed);

    AdminServer admin(cfg, &baseSa, &lm);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Get("/api/metrics", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    // Should include aggregated packetsReceived = base (2) + lobby (5) => 7
    EXPECT_NE(res->body.find("\"packetsReceived\":7"), std::string::npos);

    admin.stop();
    lm.stop();
}

TEST(AdminServerLobbyAPI, Delete_ExistingAndNotFound) {
    AdminServer::Config cfg;
    cfg.port = 9222;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    LobbyManager::Config lmCfg;
    lmCfg.basePort = 54800;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Create a lobby via manager API
    auto code = lm.createLobby(false);
    ASSERT_FALSE(code.empty());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    AdminServer admin(cfg, nullptr, &lm);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    std::string path = std::string{"/api/lobby/"} + code + "/delete";
    auto res = cli.Post(path.c_str(), goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    // Deleting again should return 404
    res = cli.Post(path.c_str(), goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);

    admin.stop();
    lm.stop();
}

TEST(AdminServerLobbyAPI, Bans_GetAndUnban_WithManager) {
    AdminServer::Config cfg;
    cfg.port = 9223;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    LobbyManager::Config lmCfg;
    lmCfg.basePort = 54900;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto lobbies = lm.getAllLobbies();
    ASSERT_GE(lobbies.size(), 1u);
    auto* lobby = lobbies[0];
    ASSERT_NE(lobby, nullptr);
    auto* ls = lobby->getServerApp();
    ASSERT_NE(ls, nullptr);

    // Ban an IP on the lobby server
    ls->getBanManager().banIp("123.123.123.123", "", "Testing ban");
    ASSERT_TRUE(ls->getBanManager().isIpBanned("123.123.123.123"));

    AdminServer admin(cfg, nullptr, &lm);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    // GET /api/bans should list the ban
    auto res = cli.Get("/api/bans", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("123.123.123.123"), std::string::npos);

    // Unban via admin API
    res = cli.Post("/api/unban", goodAuth, R"({"ip": "123.123.123.123"})", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    EXPECT_FALSE(ls->getBanManager().isIpBanned("123.123.123.123"));

    admin.stop();
    lm.stop();
}
