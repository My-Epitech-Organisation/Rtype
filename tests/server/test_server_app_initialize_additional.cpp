#include <gtest/gtest.h>

#include "server/serverApp/ServerApp.hpp"
#include "GameEngineFactory.hpp"
#include "server/serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.hpp"
#include <atomic>
#include <memory>

using namespace rtype;
using namespace rtype::server;
using namespace rtype::engine;

// Minimal fake engine that always succeeds initialization
class FakeEngineOk : public IGameEngine {
  public:
    bool initialize() override { return true; }
    void update(float) override {}
    void shutdown() override {}
    void setEventCallback(EventCallback) override {}
    std::vector<GameEvent> getPendingEvents() override { return {}; }
    void clearPendingEvents() override {}
    std::size_t getEntityCount() const override { return 0; }
    bool isRunning() const override { return true; }
    std::string getGameId() const override { return "fakeOk"; }
    bool loadLevelFromFile(const std::string& /*filepath*/) override { return true; }
    ProcessedEvent processEvent(const GameEvent&) override { return {GameEventType::GameOver, 0u, static_cast<uint8_t>(0), static_cast<uint8_t>(0), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false}; }
    void syncEntityPositions(std::function<void(uint32_t, float, float, float, float)>) override {}
};

// Minimal fake engine that fails initialization
class FakeEngineFail : public IGameEngine {
  public:
    bool initialize() override { return false; }
    void update(float) override {}
    void shutdown() override {}
    void setEventCallback(EventCallback) override {}
    std::vector<GameEvent> getPendingEvents() override { return {}; }
    void clearPendingEvents() override {}
    std::size_t getEntityCount() const override { return 0; }
    bool isRunning() const override { return false; }
    std::string getGameId() const override { return "fakeFail"; }
    bool loadLevelFromFile(const std::string& /*filepath*/) override { return true; }
    ProcessedEvent processEvent(const GameEvent&) override { return {GameEventType::GameOver, 0u, static_cast<uint8_t>(0), static_cast<uint8_t>(0), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false}; }
    void syncEntityPositions(std::function<void(uint32_t, float, float, float, float)>) override {}
};

TEST(ServerAppInitializeAdditional, Initialize_SucceedsWithRegisteredEngine) {
    // Register a small mock engine and make it available as "test_game"
    GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<FakeEngineOk>();
        });

    // Provide a simple game config that reports the test game id and is initialized
    class TestGameConfig : public rtype::server::IGameConfig {
      public:
        bool initialize(const std::string& /*configDir*/) override { return true; }
        bool reloadConfiguration() override { return true; }
        bool isInitialized() const noexcept override { return true; }
        rtype::server::GenericServerSettings getServerSettings() const noexcept override { return {9001, 4, 60, ""}; }
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

TEST(ServerAppInitializeAdditional, Initialize_FailsWhenEngineInitializeFails) {
    auto prevDefault = GameEngineFactory::getDefaultGame();

    ASSERT_TRUE(GameEngineFactory::registerGame("fakeFail", [](std::shared_ptr<ECS::Registry>){ return std::make_unique<FakeEngineFail>(); }));
    ASSERT_TRUE(GameEngineFactory::setDefaultGame("fakeFail"));

    ServerApp sa(9002, 4, 60, std::make_shared<std::atomic<bool>>(false), 10, false);
    EXPECT_FALSE(sa.run());

    // Cleanup
    ASSERT_TRUE(GameEngineFactory::unregisterGame("fakeFail"));
    if (!prevDefault.empty()) {
        EXPECT_TRUE(GameEngineFactory::setDefaultGame(prevDefault));
    }
}
