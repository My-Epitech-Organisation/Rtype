#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "httplib.h"

using namespace rtype::server;

TEST(AdminServerAuthLocalhost, LocalhostAllowedWhenTokenConfigured) {
    AdminServer::Config cfg;
    cfg.port = 9311;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    // Make request WITHOUT Authorization header from localhost
    httplib::Client cli("127.0.0.1", cfg.port);
    auto res = cli.Get("/api/lobbies");
    ASSERT_NE(res, nullptr);
    // Should be allowed (empty list or 200).
    EXPECT_EQ(res->status, 200);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}
