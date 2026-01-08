#include <gtest/gtest.h>

#include <memory>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>
#include "games/rtype/server/GameEngine.hpp"

#include "server/serverApp/ServerApp.hpp"
#include "server/serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.hpp"

using namespace rtype::server;
using namespace rtype::engine;

// Minimal mock engine used for initialize success path
class TestMockGameEngine : public AGameEngine {
   public:
    explicit TestMockGameEngine(std::shared_ptr<ECS::Registry> registry)
        : _registry(std::move(registry)) {}

    bool initialize() override {
        _isRunning = true;
        return true;
    }

    void update(float /*deltaTime*/) override {}
    void shutdown() override { _isRunning = false; }
    ProcessedEvent processEvent(const GameEvent& event) override {
        ProcessedEvent r{}; r.valid = true; r.type = event.type; return r;
    }
    void syncEntityPositions(std::function<void(uint32_t, float, float, float, float)>) override {}
    [[nodiscard]] std::string getGameId() const override { return "test_game"; }

   private:
    std::shared_ptr<ECS::Registry> _registry;
};

TEST(ServerAppInitialize, NoRegisteredGames_ReturnsFalse) {
    // Ensure no engines are registered
    GameEngineFactory::clearRegistry();

    auto shutdownFlag = std::make_shared<std::atomic<bool>>(true);
    ServerApp server(8100, 4, 60, shutdownFlag, 10, false);

    // run() will call initialize() internally and then exit immediately because shutdownFlag is true
    EXPECT_FALSE(server.run());

    // Restore the default RType registration for other tests
    rtype::games::rtype::server::registerRTypeGameEngine();
}

TEST(ServerAppInitialize, WithMockGameEngine_ReturnsTrue) {
    // Register a small mock engine and make it available as "test_game"
    GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<TestMockGameEngine>(std::move(registry));
        });

    // Provide a simple game config that reports the test game id and is initialized
    class TestGameConfig : public rtype::server::IGameConfig {
      public:
        bool initialize(const std::string& /*configDir*/) override { return true; }
        bool reloadConfiguration() override { return true; }
        bool isInitialized() const noexcept override { return true; }
        rtype::server::GenericServerSettings getServerSettings() const noexcept override { return {8101, 4, 60, ""}; }
        rtype::server::GenericGameplaySettings getGameplaySettings() const noexcept override { return {"normal", 3, 200.0F, 1.0F}; }
        std::string getSavesPath() const noexcept override { return ""; }
        bool saveGame(const std::string& /*slotName*/, const std::vector<uint8_t>& /*data*/) override { return false; }
        std::vector<uint8_t> loadGame(const std::string& /*slotName*/) override { return {}; }
        std::vector<rtype::server::GenericSaveInfo> listSaves() const override { return {}; }
        bool saveExists(const std::string& /*slotName*/) const override { return false; }
        bool deleteSave(const std::string& /*slotName*/) override { return false; }
        const std::string& getLastError() const noexcept override { static std::string s; return s; }
        std::string getGameId() const noexcept override { return "test_game"; }
    };

    auto shutdownFlag = std::make_shared<std::atomic<bool>>(true);
    auto gameConfig = std::make_unique<TestGameConfig>();

    // Register a minimal entity spawner for the test game
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

    ServerApp server(std::move(gameConfig), shutdownFlag, false);

    // run() will call initialize() internally and then exit immediately because shutdownFlag is true
    EXPECT_TRUE(server.run());

    // Cleanup registration
    GameEngineFactory::unregisterGame("test_game");
    ::rtype::server::EntitySpawnerFactory::unregisterSpawner("test_game");
}
