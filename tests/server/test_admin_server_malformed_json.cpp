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
