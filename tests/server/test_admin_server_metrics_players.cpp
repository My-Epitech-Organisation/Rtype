#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "server/serverApp/ServerApp.hpp"
#include "server/lobby/LobbyManager.hpp"
#include "httplib.h"

using namespace rtype::server;

TEST(AdminServerMetricsPlayers, MetricsAggregation_SumsAcrossLobbies) {
    AdminServer::Config cfg;
    cfg.port = 9210;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    // Base server app to provide base metrics
    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    ServerApp baseSa(8200, 4, 60, shutdownFlag, 10, false);

    // Create a lobby manager with one instance
    LobbyManager::Config lmCfg;
    lmCfg.basePort = 54200;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());

    // Wait briefly for lobby to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Set some metrics on base server and lobby server app
    {
        auto& baseMetrics = const_cast<ServerMetrics&>(baseSa.getMetrics());
        baseMetrics.packetsReceived.store(10, std::memory_order_relaxed);
        baseMetrics.packetsSent.store(5, std::memory_order_relaxed);
        rtype::server::MetricsSnapshot snap{};
        snap.playerCount = 0;
        snap.packetsReceived = 10;
        baseMetrics.addSnapshot(snap);
    }

    auto lobbies = lm.getAllLobbies();
    ASSERT_GE(lobbies.size(), 1u);
    auto* lobby = lobbies[0];
    ASSERT_NE(lobby, nullptr);

    // Lobby's ServerApp should be available after start
    auto* ls = lobby->getServerApp();
    ASSERT_NE(ls, nullptr);
    const_cast<ServerMetrics&>(ls->getMetrics()).packetsReceived.store(100, std::memory_order_relaxed);
    const_cast<ServerMetrics&>(ls->getMetrics()).packetsSent.store(50, std::memory_order_relaxed);

    AdminServer admin(cfg, &baseSa, &lm);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Get("/api/metrics", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    // Should include aggregated packetsReceived = 110
    EXPECT_NE(res->body.find("\"packetsReceived\":110"), std::string::npos);
    EXPECT_NE(res->body.find("\"packetsSent\":55"), std::string::npos);

    admin.stop();
    lm.stop();
}

TEST(AdminServerMetricsPlayers, Players_WithOneClient_ReturnsList) {
    AdminServer::Config cfg;
    cfg.port = 9211;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    LobbyManager::Config lmCfg;
    lmCfg.basePort = 54300;
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

    // Register a client via public API
    rtype::Endpoint ep{"1.2.3.4", 2222};
    auto clientId = ls->getClientManager().handleNewConnection(ep);
    ASSERT_NE(clientId, rtype::server::ClientManager::INVALID_CLIENT_ID);

    AdminServer admin(cfg, nullptr, &lm);
    ASSERT_TRUE(admin.start());
    ASSERT_TRUE(admin.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Get("/api/players", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    // Should include the client id and an "ip" field
    EXPECT_NE(res->body.find(std::string("\"id\":") + std::to_string(clientId)), std::string::npos);
    EXPECT_NE(res->body.find("\"ip\":"), std::string::npos);

    admin.stop();
    lm.stop();
}
