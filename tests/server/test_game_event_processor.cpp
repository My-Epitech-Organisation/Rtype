/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_game_event_processor - Unit tests for GameEventProcessor
*/

#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "../../lib/rtype_ecs/src/core/Registry/Registry.hpp"
#include "../../lib/rtype_engine/src/IGameEngine.hpp"
#include "../../src/server/network/ServerNetworkSystem.hpp"
#include "../../src/server/network/NetworkServer.hpp"
#include "../../src/server/serverApp/game/gameEvent/GameEventProcessor.hpp"

using namespace rtype::server;
using namespace rtype::engine;
using namespace ECS;

// ============================================================================
// MOCK GAME ENGINE
// ============================================================================

class MockGameEngine : public IGameEngine {
   public:
    bool initialize() override { return true; }

    void update(float /*deltaTime*/) override {}

    void shutdown() override {}

    void setEventCallback(EventCallback callback) override {
        _callback = std::move(callback);
    }

    std::vector<GameEvent> getPendingEvents() override {
        return _pendingEvents;
    }

    void clearPendingEvents() override {
        _clearPendingEventsCalled = true;
        _pendingEvents.clear();
    }

    std::size_t getEntityCount() const override { return 0; }

    bool isRunning() const override { return true; }

    ProcessedEvent processEvent(const GameEvent& event) override {
        ProcessedEvent processed;
        processed.type = event.type;
        processed.networkId = event.entityNetworkId;
        processed.networkEntityType = event.entityType;
        processed.x = event.x;
        processed.y = event.y;
        processed.vx = event.velocityX;
        processed.vy = event.velocityY;
        processed.valid = _processEventReturnsValid;
        return processed;
    }

    void syncEntityPositions(
        std::function<void(uint32_t, float, float, float, float)> callback) override {
        for (const auto& pos : _entityPositions) {
            callback(pos.networkId, pos.x, pos.y, pos.vx, pos.vy);
        }
    }

    uint32_t spawnProjectile(uint32_t /*playerNetworkId*/, float /*playerX*/,
                             float /*playerY*/) override {
        return 0;  // Mock: not implemented
    }

    void updatePlayerPositions(
        float /*deltaTime*/,
        std::function<void(uint32_t, float, float, float, float)>
        /*positionCallback*/) override {
        // Mock: no-op
    }

    bool setPlayerVelocity(uint32_t /*networkId*/, float /*vx*/,
                           float /*vy*/) override {
        return false;  // Mock: not implemented
    }

    [[nodiscard]] std::optional<PlayerState> getPlayerPosition(
        uint32_t /*networkId*/) const override {
        return std::nullopt;  // Mock: not implemented
    }

    // Test helpers
    void addPendingEvent(const GameEvent& event) {
        _pendingEvents.push_back(event);
    }

    void setProcessEventReturnsValid(bool valid) {
        _processEventReturnsValid = valid;
    }

    bool wasClearPendingEventsCalled() const {
        return _clearPendingEventsCalled;
    }

    struct EntityPosition {
        uint32_t networkId;
        float x, y, vx, vy;
    };

    void addEntityPosition(uint32_t id, float x, float y, float vx, float vy) {
        _entityPositions.push_back({id, x, y, vx, vy});
    }

   private:
    EventCallback _callback;
    std::vector<GameEvent> _pendingEvents;
    bool _processEventReturnsValid = true;
    bool _clearPendingEventsCalled = false;
    std::vector<EntityPosition> _entityPositions;
};

// ============================================================================
// TEST FIXTURE
// ============================================================================

class GameEventProcessorTest : public ::testing::Test {
   protected:
    void SetUp() override {
        registry_ = std::make_shared<Registry>();
        NetworkServer::Config config;
        config.clientTimeout = std::chrono::milliseconds(5000);
        server_ = std::make_shared<NetworkServer>(config);
        networkSystem_ = std::make_shared<ServerNetworkSystem>(registry_, server_);
        gameEngine_ = std::make_shared<MockGameEngine>();
    }

    void TearDown() override {
        networkSystem_.reset();
        server_->stop();
        server_.reset();
        registry_.reset();
        gameEngine_.reset();
    }

    std::shared_ptr<Registry> registry_;
    std::shared_ptr<NetworkServer> server_;
    std::shared_ptr<ServerNetworkSystem> networkSystem_;
    std::shared_ptr<MockGameEngine> gameEngine_;
};

// ============================================================================
// CONSTRUCTOR TESTS
// ============================================================================

TEST_F(GameEventProcessorTest, Constructor_ValidParameters) {
    EXPECT_NO_THROW({
        GameEventProcessor processor(gameEngine_, networkSystem_, false);
    });
}

TEST_F(GameEventProcessorTest, Constructor_VerboseMode) {
    EXPECT_NO_THROW({
        GameEventProcessor processor(gameEngine_, networkSystem_, true);
    });
}

TEST_F(GameEventProcessorTest, Constructor_NullGameEngine) {
    EXPECT_NO_THROW({
        GameEventProcessor processor(nullptr, networkSystem_, false);
    });
}

TEST_F(GameEventProcessorTest, Constructor_NullNetworkSystem) {
    EXPECT_NO_THROW({
        GameEventProcessor processor(gameEngine_, nullptr, false);
    });
}

// ============================================================================
// PROCESS EVENTS TESTS
// ============================================================================

TEST_F(GameEventProcessorTest, ProcessEvents_NoEvents) {
    GameEventProcessor processor(gameEngine_, networkSystem_, false);

    EXPECT_NO_THROW({
        processor.processEvents();
    });

    EXPECT_TRUE(gameEngine_->wasClearPendingEventsCalled());
}

TEST_F(GameEventProcessorTest, ProcessEvents_NullGameEngine) {
    GameEventProcessor processor(nullptr, networkSystem_, false);

    EXPECT_NO_THROW({
        processor.processEvents();
    });
}

TEST_F(GameEventProcessorTest, ProcessEvents_NullNetworkSystem) {
    GameEventProcessor processor(gameEngine_, nullptr, false);

    EXPECT_NO_THROW({
        processor.processEvents();
    });
}

TEST_F(GameEventProcessorTest, ProcessEvents_EntitySpawned) {
    GameEventProcessor processor(gameEngine_, networkSystem_, true);

    GameEvent event;
    event.type = GameEventType::EntitySpawned;
    event.entityNetworkId = 1;
    event.entityType = 0;  // Player
    event.x = 100.0f;
    event.y = 200.0f;

    gameEngine_->addPendingEvent(event);

    EXPECT_NO_THROW({
        processor.processEvents();
    });
}

TEST_F(GameEventProcessorTest, ProcessEvents_EntityDestroyed) {
    GameEventProcessor processor(gameEngine_, networkSystem_, true);

    // First register an entity
    ECS::Entity entity = registry_->spawnEntity();
    networkSystem_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 100.0f, 200.0f);

    GameEvent event;
    event.type = GameEventType::EntityDestroyed;
    event.entityNetworkId = 1;

    gameEngine_->addPendingEvent(event);

    EXPECT_NO_THROW({
        processor.processEvents();
    });
}

TEST_F(GameEventProcessorTest, ProcessEvents_EntityUpdated) {
    GameEventProcessor processor(gameEngine_, networkSystem_, false);

    // Register an entity
    ECS::Entity entity = registry_->spawnEntity();
    networkSystem_->registerNetworkedEntity(entity, 1,
        ServerNetworkSystem::EntityType::Player, 100.0f, 200.0f);

    GameEvent event;
    event.type = GameEventType::EntityUpdated;
    event.entityNetworkId = 1;
    event.x = 150.0f;
    event.y = 250.0f;
    event.velocityX = 10.0f;
    event.velocityY = 20.0f;

    gameEngine_->addPendingEvent(event);

    EXPECT_NO_THROW({
        processor.processEvents();
    });
}

TEST_F(GameEventProcessorTest, ProcessEvents_EntityHealthChanged) {
    GameEventProcessor processor(gameEngine_, networkSystem_, true);

    GameEvent event;
    event.type = GameEventType::EntityHealthChanged;
    event.entityNetworkId = 1;
    event.healthCurrent = 2;
    event.healthMax = 3;

    gameEngine_->addPendingEvent(event);

    EXPECT_NO_THROW({
        processor.processEvents();
    });
}

TEST_F(GameEventProcessorTest, ProcessEvents_InvalidEvent) {
    GameEventProcessor processor(gameEngine_, networkSystem_, true);

    gameEngine_->setProcessEventReturnsValid(false);

    GameEvent event;
    event.type = GameEventType::EntitySpawned;
    event.entityNetworkId = 1;

    gameEngine_->addPendingEvent(event);

    EXPECT_NO_THROW({
        processor.processEvents();
    });
}

TEST_F(GameEventProcessorTest, ProcessEvents_MultipleEvents) {
    GameEventProcessor processor(gameEngine_, networkSystem_, false);

    GameEvent event1;
    event1.type = GameEventType::EntitySpawned;
    event1.entityNetworkId = 1;
    event1.x = 100.0f;
    event1.y = 100.0f;

    GameEvent event2;
    event2.type = GameEventType::EntityUpdated;
    event2.entityNetworkId = 1;
    event2.x = 150.0f;
    event2.y = 150.0f;

    GameEvent event3;
    event3.type = GameEventType::EntityDestroyed;
    event3.entityNetworkId = 1;

    gameEngine_->addPendingEvent(event1);
    gameEngine_->addPendingEvent(event2);
    gameEngine_->addPendingEvent(event3);

    EXPECT_NO_THROW({
        processor.processEvents();
    });
}

// ============================================================================
// SYNC ENTITY POSITIONS TESTS
// ============================================================================

TEST_F(GameEventProcessorTest, SyncEntityPositions_NoEntities) {
    GameEventProcessor processor(gameEngine_, networkSystem_, false);

    EXPECT_NO_THROW({
        processor.syncEntityPositions();
    });
}

TEST_F(GameEventProcessorTest, SyncEntityPositions_NullGameEngine) {
    GameEventProcessor processor(nullptr, networkSystem_, false);

    EXPECT_NO_THROW({
        processor.syncEntityPositions();
    });
}

TEST_F(GameEventProcessorTest, SyncEntityPositions_NullNetworkSystem) {
    GameEventProcessor processor(gameEngine_, nullptr, false);

    EXPECT_NO_THROW({
        processor.syncEntityPositions();
    });
}

TEST_F(GameEventProcessorTest, SyncEntityPositions_WithEntities) {
    GameEventProcessor processor(gameEngine_, networkSystem_, false);

    gameEngine_->addEntityPosition(1, 100.0f, 200.0f, 10.0f, 20.0f);
    gameEngine_->addEntityPosition(2, 300.0f, 400.0f, 30.0f, 40.0f);

    EXPECT_NO_THROW({
        processor.syncEntityPositions();
    });
}
