/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_player_input_handler - Unit tests for PlayerInputHandler
*/

#include <gtest/gtest.h>

#include <memory>
#include <optional>

#include "core/Registry/Registry.hpp"
#include "../../src/server/network/ServerNetworkSystem.hpp"
#include "../../src/server/network/NetworkServer.hpp"
#include "../../src/server/serverApp/game/gameStateManager/GameStateManager.hpp"
#include "../../src/server/serverApp/player/playerInputHandler/PlayerInputHandler.hpp"
#include "games/rtype/shared/Components/CooldownComponent.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"
#include "games/rtype/shared/Components/VelocityComponent.hpp"

using namespace rtype::server;
using namespace ECS;

// Mock GameConfig for testing
class MockGameConfig : public IGameConfig {
   public:
    bool initialize(const std::string& /*configDir*/) override {
        _initialized = true;
        return true;
    }

    bool reloadConfiguration() override { return true; }

    bool isInitialized() const noexcept override { return _initialized; }

    GenericServerSettings getServerSettings() const noexcept override {
        return _serverSettings;
    }

    GenericGameplaySettings getGameplaySettings() const noexcept override {
        return _gameplaySettings;
    }

    std::string getSavesPath() const noexcept override { return "/tmp/saves"; }

    bool saveGame(const std::string& /*slotName*/,
                  const std::vector<uint8_t>& /*gameStateData*/) override {
        return true;
    }

    std::vector<uint8_t> loadGame(const std::string& /*slotName*/) override {
        return {};
    }

    std::vector<GenericSaveInfo> listSaves() const override { return {}; }

    bool saveExists(const std::string& /*slotName*/) const override {
        return false;
    }

    bool deleteSave(const std::string& /*slotName*/) override { return false; }

    const std::string& getLastError() const noexcept override {
        return _lastError;
    }

    std::string getGameId() const noexcept override { return "test_game"; }

    void setInitialized(bool init) { _initialized = init; }
    void setPlayerSpeed(float speed) { _gameplaySettings.playerSpeed = speed; }

   private:
    bool _initialized = false;
    std::string _lastError;
    GenericServerSettings _serverSettings;
    GenericGameplaySettings _gameplaySettings;
};

// ============================================================================
// TEST FIXTURE
// ============================================================================

class PlayerInputHandlerTest : public ::testing::Test {
   protected:
    void SetUp() override {
        registry_ = std::make_shared<Registry>();
        NetworkServer::Config config;
        config.clientTimeout = std::chrono::milliseconds(5000);
        server_ = std::make_shared<NetworkServer>(config);
        networkSystem_ = std::make_shared<ServerNetworkSystem>(registry_, server_);
        stateManager_ = std::make_shared<GameStateManager>(1);
    }

    void TearDown() override {
        networkSystem_.reset();
        server_->stop();
        server_.reset();
        registry_.reset();
        stateManager_.reset();
    }

    std::shared_ptr<Registry> registry_;
    std::shared_ptr<NetworkServer> server_;
    std::shared_ptr<ServerNetworkSystem> networkSystem_;
    std::shared_ptr<GameStateManager> stateManager_;
};

// ============================================================================
// CONSTRUCTOR TESTS
// ============================================================================

TEST_F(PlayerInputHandlerTest, Constructor_WithoutGameConfig) {
    EXPECT_NO_THROW({
        PlayerInputHandler handler(registry_, networkSystem_, stateManager_);
    });
}

TEST_F(PlayerInputHandlerTest, Constructor_WithGameConfig) {
    auto gameConfig = std::make_shared<MockGameConfig>();
    gameConfig->setInitialized(true);
    gameConfig->setPlayerSpeed(300.0f);

    EXPECT_NO_THROW({
        PlayerInputHandler handler(registry_, networkSystem_, stateManager_,
                                   gameConfig, false);
    });
}

TEST_F(PlayerInputHandlerTest, Constructor_WithUninitializedGameConfig) {
    auto gameConfig = std::make_shared<MockGameConfig>();
    gameConfig->setInitialized(false);

    EXPECT_NO_THROW({
        PlayerInputHandler handler(registry_, networkSystem_, stateManager_,
                                   gameConfig, false);
    });
}

TEST_F(PlayerInputHandlerTest, Constructor_VerboseMode) {
    EXPECT_NO_THROW({
        PlayerInputHandler handler(registry_, networkSystem_, stateManager_,
                                   nullptr, true);
    });
}

// ============================================================================
// HANDLE INPUT TESTS - STATE MANAGER BRANCH COVERAGE
// ============================================================================

TEST_F(PlayerInputHandlerTest, HandleInput_WhenWaitingForPlayers) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    // Game is in WaitingForPlayers state
    EXPECT_TRUE(stateManager_->isWaiting());
    EXPECT_FALSE(stateManager_->isPlayerReady(1));

    handler.handleInput(1, rtype::network::InputMask::kUp, std::nullopt);

    // Player should be marked as ready
    EXPECT_TRUE(stateManager_->isPlayerReady(1));
}

TEST_F(PlayerInputHandlerTest, HandleInput_WhenPaused) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->forceStart();
    stateManager_->pause();

    EXPECT_TRUE(stateManager_->isPaused());
    EXPECT_FALSE(stateManager_->isPlayerReady(1));

    handler.handleInput(1, rtype::network::InputMask::kUp, std::nullopt);

    // Player should be marked as ready when paused
    EXPECT_TRUE(stateManager_->isPlayerReady(1));
}

TEST_F(PlayerInputHandlerTest, HandleInput_PlayerAlreadyReady) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->playerReady(1);
    EXPECT_TRUE(stateManager_->isPlayerReady(1));

    // Call again - should not cause issues
    handler.handleInput(1, rtype::network::InputMask::kUp, std::nullopt);
}

TEST_F(PlayerInputHandlerTest, HandleInput_WhenNotPlaying_NoEntity) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    // stateManager is waiting, not playing
    EXPECT_FALSE(stateManager_->isPlaying());

    // No entity provided - should return early
    handler.handleInput(1, rtype::network::InputMask::kUp, std::nullopt);
}

TEST_F(PlayerInputHandlerTest, HandleInput_WhenPlaying_NoEntity) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->forceStart();
    EXPECT_TRUE(stateManager_->isPlaying());

    // No entity provided - should return early
    handler.handleInput(1, rtype::network::InputMask::kUp, std::nullopt);
}

TEST_F(PlayerInputHandlerTest, HandleInput_EntityNotAlive) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->forceStart();

    ECS::Entity entity = registry_->spawnEntity();
    registry_->killEntity(entity);

    handler.handleInput(1, rtype::network::InputMask::kUp, entity);
}

// ============================================================================
// MOVEMENT BRANCH COVERAGE TESTS
// ============================================================================

TEST_F(PlayerInputHandlerTest, ProcessMovement_AllDirections) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);

    // Test Up
    handler.handleInput(1, rtype::network::InputMask::kUp, entity);
    auto& vel = registry_->getComponent<Velocity>(entity);
    EXPECT_LT(vel.vy, 0.0f);  // Up = negative y

    // Test Down
    handler.handleInput(1, rtype::network::InputMask::kDown, entity);
    EXPECT_GT(vel.vy, 0.0f);  // Down = positive y

    // Test Left
    handler.handleInput(1, rtype::network::InputMask::kLeft, entity);
    EXPECT_LT(vel.vx, 0.0f);  // Left = negative x

    // Test Right
    handler.handleInput(1, rtype::network::InputMask::kRight, entity);
    EXPECT_GT(vel.vx, 0.0f);  // Right = positive x
}

TEST_F(PlayerInputHandlerTest, ProcessMovement_DiagonalMovement) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);

    // Test Up + Right
    std::uint8_t mask = rtype::network::InputMask::kUp | rtype::network::InputMask::kRight;
    handler.handleInput(1, mask, entity);

    auto& vel = registry_->getComponent<Velocity>(entity);
    EXPECT_LT(vel.vy, 0.0f);  // Up
    EXPECT_GT(vel.vx, 0.0f);  // Right
}

TEST_F(PlayerInputHandlerTest, ProcessMovement_OppositeDirectionsCancel) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);

    // Test Up + Down (should cancel out)
    std::uint8_t mask = rtype::network::InputMask::kUp | rtype::network::InputMask::kDown;
    handler.handleInput(1, mask, entity);

    auto& vel = registry_->getComponent<Velocity>(entity);
    EXPECT_EQ(vel.vy, 0.0f);  // Cancelled

    // Test Left + Right
    mask = rtype::network::InputMask::kLeft | rtype::network::InputMask::kRight;
    handler.handleInput(1, mask, entity);
    EXPECT_EQ(vel.vx, 0.0f);  // Cancelled
}

TEST_F(PlayerInputHandlerTest, ProcessMovement_NoVelocityComponent) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    // No velocity component

    // Should not crash
    EXPECT_NO_THROW({
        handler.handleInput(1, rtype::network::InputMask::kUp, entity);
    });
}

TEST_F(PlayerInputHandlerTest, ProcessMovement_NoInput) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 50.0f, 50.0f);

    handler.handleInput(1, rtype::network::InputMask::kNone, entity);

    auto& vel = registry_->getComponent<Velocity>(entity);
    EXPECT_EQ(vel.vx, 0.0f);
    EXPECT_EQ(vel.vy, 0.0f);
}

// ============================================================================
// SHOOT BRANCH COVERAGE TESTS
// ============================================================================

TEST_F(PlayerInputHandlerTest, ProcessShoot_NoPositionComponent) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_, nullptr, true);

    stateManager_->forceStart();

    using Velocity = rtype::games::rtype::shared::VelocityComponent;
    using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);
    registry_->emplaceComponent<ShootCooldown>(entity, 0.3f);

    handler.handleInput(1, rtype::network::InputMask::kShoot, entity);
}

TEST_F(PlayerInputHandlerTest, ProcessShoot_NoShootCooldownComponent) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_, nullptr, true);

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);

    handler.handleInput(1, rtype::network::InputMask::kShoot, entity);
}

TEST_F(PlayerInputHandlerTest, ProcessShoot_CooldownNotReady) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_, nullptr, true);

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;
    using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);
    auto& cooldown = registry_->emplaceComponent<ShootCooldown>(entity, 0.3f);
    cooldown.triggerCooldown();  // Put on cooldown

    handler.handleInput(1, rtype::network::InputMask::kShoot, entity);
}

TEST_F(PlayerInputHandlerTest, ProcessShoot_NoShootCallback) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);
    // No shoot callback set

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;
    using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);
    registry_->emplaceComponent<ShootCooldown>(entity, 0.3f);

    handler.handleInput(1, rtype::network::InputMask::kShoot, entity);
}

TEST_F(PlayerInputHandlerTest, ProcessShoot_WithShootCallback) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    bool callbackCalled = false;
    uint32_t receivedNetworkId = 0;
    float receivedX = 0;
    float receivedY = 0;

    handler.setShootCallback([&](uint32_t networkId, float x, float y) -> uint32_t {
        callbackCalled = true;
        receivedNetworkId = networkId;
        receivedX = x;
        receivedY = y;
        return 100;  // Return projectile ID
    });

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;
    using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 150.0f, 200.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);
    registry_->emplaceComponent<ShootCooldown>(entity, 0.3f);

    networkSystem_->registerNetworkedEntity(entity, 42,
        ServerNetworkSystem::EntityType::Player, 150.0f, 200.0f);

    handler.handleInput(1, rtype::network::InputMask::kShoot, entity);

    EXPECT_TRUE(callbackCalled);
    EXPECT_EQ(receivedNetworkId, 42u);
    EXPECT_FLOAT_EQ(receivedX, 150.0f);
    EXPECT_FLOAT_EQ(receivedY, 200.0f);
}

TEST_F(PlayerInputHandlerTest, ProcessShoot_CallbackReturnsZero) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    handler.setShootCallback([](uint32_t, float, float) -> uint32_t {
        return 0;  // Failure case
    });

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;
    using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 150.0f, 200.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);
    auto& cooldown = registry_->emplaceComponent<ShootCooldown>(entity, 0.3f);

    networkSystem_->registerNetworkedEntity(entity, 42,
        ServerNetworkSystem::EntityType::Player, 150.0f, 200.0f);

    handler.handleInput(1, rtype::network::InputMask::kShoot, entity);

    // Cooldown should NOT be triggered if projectile spawn failed
    EXPECT_TRUE(cooldown.canShoot());
}

TEST_F(PlayerInputHandlerTest, ProcessShoot_EntityNotRegistered) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_, nullptr, true);

    handler.setShootCallback([](uint32_t, float, float) -> uint32_t {
        return 100;
    });

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;
    using ShootCooldown = rtype::games::rtype::shared::ShootCooldownComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 150.0f, 200.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);
    registry_->emplaceComponent<ShootCooldown>(entity, 0.3f);
    // Not registered with network system

    handler.handleInput(1, rtype::network::InputMask::kShoot, entity);
}

// ============================================================================
// SET PLAYER SPEED TESTS
// ============================================================================

TEST_F(PlayerInputHandlerTest, SetPlayerSpeed) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_);

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);

    handler.setPlayerSpeed(500.0f);

    handler.handleInput(1, rtype::network::InputMask::kUp, entity);

    auto& vel = registry_->getComponent<Velocity>(entity);
    EXPECT_FLOAT_EQ(vel.vy, -500.0f);
}

// ============================================================================
// VERBOSE MODE TESTS
// ============================================================================

TEST_F(PlayerInputHandlerTest, VerboseMode_LogsInput) {
    PlayerInputHandler handler(registry_, networkSystem_, stateManager_, nullptr, true);

    stateManager_->forceStart();

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);

    // Should log but not crash
    EXPECT_NO_THROW({
        handler.handleInput(1, rtype::network::InputMask::kUp, entity);
    });
}

// ============================================================================
// NULL STATE MANAGER TESTS
// ============================================================================

TEST_F(PlayerInputHandlerTest, HandleInput_NullStateManager) {
    PlayerInputHandler handler(registry_, networkSystem_, nullptr);

    using Position = rtype::games::rtype::shared::TransformComponent;
    using Velocity = rtype::games::rtype::shared::VelocityComponent;

    ECS::Entity entity = registry_->spawnEntity();
    registry_->emplaceComponent<Position>(entity, 100.0f, 100.0f);
    registry_->emplaceComponent<Velocity>(entity, 0.0f, 0.0f);

    // Should handle gracefully with null state manager
    EXPECT_NO_THROW({
        handler.handleInput(1, rtype::network::InputMask::kUp, entity);
    });
}
