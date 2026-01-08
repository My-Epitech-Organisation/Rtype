#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "server/serverApp/ServerApp.hpp"
#include "httplib.h"

using namespace rtype::server;

TEST(AdminServerTest, AuthAndBasicEndpoints) {
    AdminServer::Config cfg;
    cfg.port = 9091;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);

    // Unauthorized access without token
    auto res = cli.Get("/api/metrics");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 401);

    // With wrong token
    httplib::Headers badAuth{{"Authorization", "Bearer wrong"}};
    res = cli.Get("/api/metrics", badAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 401);

    // With correct token
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};
    res = cli.Get("/api/metrics", goodAuth);
    ASSERT_NE(res, nullptr);
    // ServerApp is null in this test, should return 500
    EXPECT_EQ(res->status, 500);

    // /api/bans with auth should return 200 and empty list
    res = cli.Get("/api/bans", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"bans\":"), std::string::npos);

    // Create lobby should return 500 when no manager
    res = cli.Post("/api/lobby/create", goodAuth, "{\"isPublic\": true}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 500);

    // Delete lobby should return 500 when no manager
    res = cli.Post("/api/lobby/ABC123/delete", goodAuth, "", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 500);

    // Reset metrics should return 200
    res = cli.Post("/api/metrics/reset", goodAuth, "", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerTest, BanAndUnbanIp) {
    AdminServer::Config cfg;
    cfg.port = 9092;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    // Create a ServerApp instance (ban manager available)
    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    rtype::server::ServerApp sa(1234, 4, 60, shutdownFlag, 10, false);

    AdminServer server(cfg, &sa, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    // Ban by ip
    auto res = cli.Post("/api/ban", goodAuth, "{\"ip\": \"1.2.3.4\"}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto bans = sa.getBanManager().getBannedList();
    bool found = false;
    for (const auto& b : bans) {
        if (b.ip == "1.2.3.4") { found = true; break; }
    }
    EXPECT_TRUE(found);

    // Unban missing ip should fail
    res = cli.Post("/api/unban", goodAuth, "{ }", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    // Unban by ip
    res = cli.Post("/api/unban", goodAuth, "{\"ip\": \"1.2.3.4\"}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    bans = sa.getBanManager().getBannedList();
    EXPECT_TRUE(bans.empty());

    server.stop();
    EXPECT_FALSE(server.isRunning());
}
