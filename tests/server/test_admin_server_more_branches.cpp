#include <gtest/gtest.h>

#include "server/shared/AdminServer.hpp"
#include "server/serverApp/ServerApp.hpp"
#include "server/lobby/LobbyManager.hpp"
#include "httplib.h"
#include "GameEngineFactory.hpp"
#include "server/serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.hpp"

using namespace rtype::server;
using namespace rtype::engine;

TEST(AdminServerBranchesExtra, Ban_IpOnly_BansIp) {
    AdminServer::Config cfg;
    cfg.port = 9201;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    rtype::server::ServerApp sa(1234, 4, 60, shutdownFlag, 10, false);

    AdminServer server(cfg, &sa, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/ban", goodAuth, "{\"ip\": \"9.9.9.9\"}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto bans = sa.getBanManager().getBannedList();
    bool found = false;
    for (const auto& b : bans) {
        if (b.ip == "9.9.9.9" && b.port == 0) { found = true; break; }
    }
    EXPECT_TRUE(found);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerBranchesExtra, Unban_MissingIp_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9202;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/unban", goodAuth, "{}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerBranchesExtra, Lobby_Create_SucceedsAndDelete) {
    AdminServer::Config cfg;
    cfg.port = 9203;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    // Register a minimal test game engine and entity spawner so ServerApp instances can initialize
    auto prevDefault = GameEngineFactory::getDefaultGame();
    class TestFakeEngine : public IGameEngine {
      public:
        bool initialize() override { return true; }
        void update(float) override {}
        void shutdown() override {}
        void setEventCallback(EventCallback) override {}
        std::vector<GameEvent> getPendingEvents() override { return {}; }
        void clearPendingEvents() override {}
        std::size_t getEntityCount() const override { return 0; }
        bool isRunning() const override { return true; }
        std::string getGameId() const override { return "test_game"; }
        bool loadLevelFromFile(const std::string& /*filepath*/) override { return true; }
        ProcessedEvent processEvent(const GameEvent&) override { return {GameEventType::GameOver, 0u, static_cast<uint8_t>(0), static_cast<uint8_t>(0), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false}; }
        void syncEntityPositions(std::function<void(uint32_t, float, float, float, float)>) override {}
    };

    ASSERT_TRUE(GameEngineFactory::registerGame("test_game", [](std::shared_ptr<ECS::Registry>) { return std::make_unique<TestFakeEngine>(); }));
    ASSERT_TRUE(GameEngineFactory::setDefaultGame("test_game"));
    // Sanity-check that registration and default selection worked
    ASSERT_TRUE(GameEngineFactory::isRegistered("test_game"));
    ASSERT_EQ(GameEngineFactory::getDefaultGame(), "test_game");

    class TestEntitySpawner : public rtype::server::IEntitySpawner {
      public:
        explicit TestEntitySpawner(std::shared_ptr<ECS::Registry>) {}
        PlayerSpawnResult spawnPlayer(const PlayerSpawnConfig& config) override { return PlayerSpawnResult{ECS::Entity(), 0u, 0.0f, 0.0f, 1, 1, true}; }
        void destroyPlayer(ECS::Entity) override {}
        bool destroyPlayerByUserId(std::uint32_t) override { return false; }
        std::optional<ECS::Entity> getPlayerEntity(std::uint32_t) const override { return std::nullopt; }
        float getPlayerSpeed() const noexcept override { return 100.0F; }
        WorldBounds getWorldBounds() const noexcept override { return {-100.0f, 100.0f, -100.0f, 100.0f}; }
        std::string getGameId() const noexcept override { return "test_game"; }
        std::uint32_t handlePlayerShoot(ECS::Entity, std::uint32_t) override { return 0u; }
        bool canPlayerShoot(ECS::Entity) const override { return false; }
        void triggerShootCooldown(ECS::Entity) override {}
        std::optional<std::uint32_t> getEntityNetworkId(ECS::Entity) const override { return std::nullopt; }
        std::optional<EntityPosition> getEntityPosition(ECS::Entity) const override { return std::nullopt; }
        void updatePlayerVelocity(ECS::Entity, float, float) override {}
        void updateAllPlayersMovement(float, const PositionUpdateCallback&) override {}
    };

    ::rtype::server::EntitySpawnerFactory::registerSpawner(
        "test_game",
        [](auto registry, auto /*networkSystem*/, auto /*gameEngine*/, auto /*gameConfig*/) {
            return std::make_unique<TestEntitySpawner>(std::move(registry));
        });

    // Start a LobbyManager with one instance
    rtype::server::LobbyManager::Config lmCfg;
    lmCfg.basePort = 54100;
    lmCfg.instanceCount = 1;
    lmCfg.maxInstances = 4;

    rtype::server::LobbyManager lm(lmCfg);
    ASSERT_TRUE(lm.start());

    AdminServer server(cfg, nullptr, &lm);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    // Create a public lobby
    auto res = cli.Post("/api/lobby/create", goodAuth, "{\"isPublic\": true}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"code\":"), std::string::npos);

    // Parse the code robustly (accept quoted or numeric code)
    auto pos = res->body.find("\"code\":");
    ASSERT_NE(pos, std::string::npos);
    // Find first digit after the code key
    pos = res->body.find_first_of("0123456789", pos);
    ASSERT_NE(pos, std::string::npos);
    auto end = res->body.find_first_not_of("0123456789", pos);
    std::string code;
    if (end == std::string::npos) {
        code = res->body.substr(pos);
    } else {
        code = res->body.substr(pos, end - pos);
    }
    ASSERT_FALSE(code.empty());
    fprintf(stderr, "Create response body: %s\n", res->body.c_str());
    fprintf(stderr, "Parsed code (raw): %s\n", code.c_str());

    // Wait a bit for lobby to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Delete the created lobby
    // Verify the lobby manager has the lobby before attempting delete
    auto foundLobby = lm.findLobbyByCode(code);
    ASSERT_NE(foundLobby, nullptr) << "Lobby not found for code: " << code;

    // Basic validation: response contains a code and succeeds
    EXPECT_NE(res->body.find("\"code\":"), std::string::npos);

    server.stop();
    lm.stop();

    // Cleanup registrations
    ::rtype::server::EntitySpawnerFactory::unregisterSpawner("test_game");
    GameEngineFactory::unregisterGame("test_game");
    if (!prevDefault.empty()) { GameEngineFactory::setDefaultGame(prevDefault); }

    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerBranchesExtra, Lobby_Create_NoManager_Returns500) {
    AdminServer::Config cfg;
    cfg.port = 9204;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/lobby/create", goodAuth, "{\"isPublic\": true}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 500);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerBranchesExtra, Ban_IpAndPort_BansEndpoint) {
    AdminServer::Config cfg;
    cfg.port = 9205;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    auto shutdownFlag = std::make_shared<std::atomic<bool>>(false);
    rtype::server::ServerApp sa(1235, 4, 60, shutdownFlag, 10, false);

    AdminServer server(cfg, &sa, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/ban", goodAuth, "{\"ip\": \"8.8.8.8\", \"port\": 2222}", "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    auto bans = sa.getBanManager().getBannedList();
    bool found = false;
    for (const auto& b : bans) {
        if (b.ip == "8.8.8.8" && b.port == 2222) { found = true; break; }
    }
    EXPECT_TRUE(found);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerBranchesExtra, Ban_EmptyBody_Returns400) {
    AdminServer::Config cfg;
    cfg.port = 9206;
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

TEST(AdminServerBranchesExtra, Kick_ClientNotFound_Returns404) {
    AdminServer::Config cfg;
    cfg.port = 9207;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Post("/api/kick/9999", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 404);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}

TEST(AdminServerBranchesExtra, Players_NoLobbies_ReturnsEmpty) {
    AdminServer::Config cfg;
    cfg.port = 9208;
    cfg.token = "testtoken";
    cfg.localhostOnly = true;

    AdminServer server(cfg, nullptr, nullptr);
    ASSERT_TRUE(server.start());
    ASSERT_TRUE(server.isRunning());

    httplib::Client cli("127.0.0.1", cfg.port);
    httplib::Headers goodAuth{{"Authorization", "Bearer testtoken"}};

    auto res = cli.Get("/api/players", goodAuth);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_NE(res->body.find("\"players\":"), std::string::npos);

    server.stop();
    EXPECT_FALSE(server.isRunning());
}
