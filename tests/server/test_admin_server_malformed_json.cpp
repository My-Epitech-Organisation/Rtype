#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "httplib.h"

using namespace rtype::server;

TEST(AdminServerMalformedJson, Unban_MalformedJson_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9401;
    cfg.token = "testtoken";
    cfg.localhostOnly = false; // allow token or local

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/unban", goodAuth, "not a json", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
}

TEST(AdminServerMalformedJson, Ban_MalformedJson_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9402;
    cfg.token = "testtoken";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/ban", goodAuth, "[not json]", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
}

TEST(AdminServerMalformedJson, LobbyCreate_MalformedJson_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9403;
    cfg.token = "testtoken";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/lobby/create", goodAuth, "{not json}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
}

TEST(AdminServerMalformedJson, Unban_EmptyBody_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9404;
    cfg.token = "testtoken";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/unban", goodAuth, "", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
}

TEST(AdminServerMalformedJson, Ban_EmptyBody_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9405;
    cfg.token = "testtoken";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/ban", goodAuth, "", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
}

TEST(AdminServerMalformedJson, LobbyCreate_EmptyBody_Returns500_NoManager) {
    AdminServer::Config cfg;
    cfg.port = 9406;
    cfg.token = "testtoken";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    // Empty body is valid for lobby/create (uses default), returns 500 because no lobby manager
    auto res = cli.Post("/api/lobby/create", goodAuth, "", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 500);  // No lobby manager available

    server.stop();
}

TEST(AdminServerMalformedJson, Unban_ValidJsonMissingIp_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9407;
    cfg.token = "testtoken";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    // Valid JSON but missing required "ip" field
    auto res = cli.Post("/api/unban", goodAuth, R"({"notip": "value"})", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
}

TEST(AdminServerMalformedJson, Ban_ValidJsonMissingIp_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9408;
    cfg.token = "testtoken";
    cfg.localhostOnly = false;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    // Valid JSON but missing required "ip" field
    auto res = cli.Post("/api/ban", goodAuth, R"({"notip": "value"})", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
}
