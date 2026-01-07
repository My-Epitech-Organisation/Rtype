/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_entity_spawner_factory - Comprehensive unit tests for EntitySpawnerFactory
*/

#include <gtest/gtest.h>

#include <memory>
#include <thread>

#include "core/Registry/Registry.hpp"
#include "network/ServerNetworkSystem.hpp"
#include "serverApp/game/entitySpawnerFactory/EntitySpawnerFactory.hpp"
#include "server/shared/IEntitySpawner.hpp"

using namespace rtype::server;

// ============================================================================
// Mock Entity Spawner for Testing
// ============================================================================

class MockEntitySpawner : public IEntitySpawner {
   public:
    MockEntitySpawner(std::shared_ptr<ECS::Registry> /*registry*/,
                     std::shared_ptr<ServerNetworkSystem> /*networkSystem*/,
                     GameEngineOpt /*gameEngine*/,
                     GameConfigOpt /*gameConfig*/)
        : spawnCount_(0) {}

    PlayerSpawnResult spawnPlayer(const PlayerSpawnConfig& /*config*/) override {
        spawnCount_++;
        return PlayerSpawnResult{};
    }

    void destroyPlayer(ECS::Entity /*entity*/) override {}

    bool destroyPlayerByUserId(std::uint32_t /*userId*/) override {
        return true;
    }

    std::optional<ECS::Entity> getPlayerEntity(std::uint32_t /*userId*/) const override {
        return std::nullopt;
    }

    float getPlayerSpeed() const noexcept override {
        return 200.0F;
    }

    std::uint32_t handlePlayerShoot(ECS::Entity /*playerEntity*/,
                                   std::uint32_t /*playerNetworkId*/) override {
        return 0;
    }

    bool canPlayerShoot(ECS::Entity /*playerEntity*/) const override {
        return true;
    }

    std::optional<std::uint32_t> getEntityNetworkId(ECS::Entity /*entity*/) const override {
        return std::nullopt;
    }

    std::optional<EntityPosition> getEntityPosition(ECS::Entity /*entity*/) const override {
        return std::nullopt;
    }

    void updatePlayerVelocity(ECS::Entity /*entity*/, float /*vx*/, float /*vy*/) override {}

    void triggerShootCooldown(ECS::Entity /*entity*/) override {}

    void updateAllPlayersMovement(float /*deltaTime*/,
                                 const PositionUpdateCallback& /*callback*/) override {}

    WorldBounds getWorldBounds() const noexcept override {
        return WorldBounds{0.0F, 1920.0F, 0.0F, 1080.0F};
    }

    std::string getGameId() const noexcept override {
        return "mock";
    }

    int spawnCount_;
};

// ============================================================================
// Test Fixture
// ============================================================================

class EntitySpawnerFactoryTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Clear registry before each test
        EntitySpawnerFactory::clearRegistry();
    }

    void TearDown() override {
        // Clean up after each test
        EntitySpawnerFactory::clearRegistry();
    }

    static std::unique_ptr<IEntitySpawner> createMockSpawner(
        std::shared_ptr<ECS::Registry> registry,
        std::shared_ptr<ServerNetworkSystem> networkSystem,
        GameEngineOpt gameEngine,
        GameConfigOpt gameConfig) {
        return std::make_unique<MockEntitySpawner>(
            std::move(registry), std::move(networkSystem), gameEngine, gameConfig);
    }
};

// ============================================================================
// Registration Tests
// ============================================================================

TEST_F(EntitySpawnerFactoryTest, RegisterSpawnerSuccess) {
    bool result = EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    EXPECT_TRUE(result);
    EXPECT_TRUE(EntitySpawnerFactory::isRegistered("test-game"));
}

TEST_F(EntitySpawnerFactoryTest, RegisterSpawnerEmptyGameId) {
    bool result = EntitySpawnerFactory::registerSpawner("", createMockSpawner);
    EXPECT_FALSE(result);
}

TEST_F(EntitySpawnerFactoryTest, RegisterSpawnerNullCreator) {
    bool result = EntitySpawnerFactory::registerSpawner("test-game", nullptr);
    EXPECT_FALSE(result);
}

TEST_F(EntitySpawnerFactoryTest, RegisterSpawnerDuplicate) {
    bool result1 = EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    EXPECT_TRUE(result1);

    bool result2 = EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    EXPECT_FALSE(result2);
}

TEST_F(EntitySpawnerFactoryTest, RegisterMultipleSpawners) {
    bool result1 = EntitySpawnerFactory::registerSpawner("game1", createMockSpawner);
    bool result2 = EntitySpawnerFactory::registerSpawner("game2", createMockSpawner);
    bool result3 = EntitySpawnerFactory::registerSpawner("game3", createMockSpawner);

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
    EXPECT_TRUE(result3);

    EXPECT_TRUE(EntitySpawnerFactory::isRegistered("game1"));
    EXPECT_TRUE(EntitySpawnerFactory::isRegistered("game2"));
    EXPECT_TRUE(EntitySpawnerFactory::isRegistered("game3"));
}

// ============================================================================
// Unregistration Tests
// ============================================================================

TEST_F(EntitySpawnerFactoryTest, UnregisterSpawnerSuccess) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    
    bool result = EntitySpawnerFactory::unregisterSpawner("test-game");
    EXPECT_TRUE(result);
    EXPECT_FALSE(EntitySpawnerFactory::isRegistered("test-game"));
}

TEST_F(EntitySpawnerFactoryTest, UnregisterSpawnerNotFound) {
    bool result = EntitySpawnerFactory::unregisterSpawner("non-existent");
    EXPECT_FALSE(result);
}

TEST_F(EntitySpawnerFactoryTest, UnregisterSpawnerTwice) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    
    bool result1 = EntitySpawnerFactory::unregisterSpawner("test-game");
    EXPECT_TRUE(result1);

    bool result2 = EntitySpawnerFactory::unregisterSpawner("test-game");
    EXPECT_FALSE(result2);
}

// ============================================================================
// Creation Tests
// ============================================================================

TEST_F(EntitySpawnerFactoryTest, CreateSpawnerSuccess) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);

    auto registry = std::make_shared<ECS::Registry>();
    auto spawner = EntitySpawnerFactory::create("test-game", registry, nullptr, std::nullopt, std::nullopt);

    EXPECT_NE(spawner, nullptr);
    EXPECT_EQ(spawner->getGameId(), "mock");
}

TEST_F(EntitySpawnerFactoryTest, CreateSpawnerNotRegistered) {
    auto registry = std::make_shared<ECS::Registry>();
    auto spawner = EntitySpawnerFactory::create("non-existent", registry, nullptr, std::nullopt, std::nullopt);

    EXPECT_EQ(spawner, nullptr);
}

TEST_F(EntitySpawnerFactoryTest, CreateMultipleInstances) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);

    auto registry1 = std::make_shared<ECS::Registry>();
    auto registry2 = std::make_shared<ECS::Registry>();

    auto spawner1 = EntitySpawnerFactory::create("test-game", registry1, nullptr, std::nullopt, std::nullopt);
    auto spawner2 = EntitySpawnerFactory::create("test-game", registry2, nullptr, std::nullopt, std::nullopt);

    EXPECT_NE(spawner1, nullptr);
    EXPECT_NE(spawner2, nullptr);
    EXPECT_NE(spawner1.get(), spawner2.get());
}

// ============================================================================
// Query Tests
// ============================================================================

TEST_F(EntitySpawnerFactoryTest, IsRegisteredTrue) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    EXPECT_TRUE(EntitySpawnerFactory::isRegistered("test-game"));
}

TEST_F(EntitySpawnerFactoryTest, IsRegisteredFalse) {
    EXPECT_FALSE(EntitySpawnerFactory::isRegistered("non-existent"));
}

TEST_F(EntitySpawnerFactoryTest, GetRegisteredSpawnersEmpty) {
    auto spawners = EntitySpawnerFactory::getRegisteredSpawners();
    EXPECT_TRUE(spawners.empty());
}

TEST_F(EntitySpawnerFactoryTest, GetRegisteredSpawnersSingle) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    
    auto spawners = EntitySpawnerFactory::getRegisteredSpawners();
    EXPECT_EQ(spawners.size(), 1);
    EXPECT_EQ(spawners[0], "test-game");
}

TEST_F(EntitySpawnerFactoryTest, GetRegisteredSpawnersMultiple) {
    EntitySpawnerFactory::registerSpawner("game-c", createMockSpawner);
    EntitySpawnerFactory::registerSpawner("game-a", createMockSpawner);
    EntitySpawnerFactory::registerSpawner("game-b", createMockSpawner);

    auto spawners = EntitySpawnerFactory::getRegisteredSpawners();
    EXPECT_EQ(spawners.size(), 3);
    
    // Should be sorted alphabetically
    EXPECT_EQ(spawners[0], "game-a");
    EXPECT_EQ(spawners[1], "game-b");
    EXPECT_EQ(spawners[2], "game-c");
}

// ============================================================================
// Clear Registry Tests
// ============================================================================

TEST_F(EntitySpawnerFactoryTest, ClearRegistryEmpty) {
    EntitySpawnerFactory::clearRegistry();
    
    auto spawners = EntitySpawnerFactory::getRegisteredSpawners();
    EXPECT_TRUE(spawners.empty());
}

TEST_F(EntitySpawnerFactoryTest, ClearRegistryWithEntries) {
    EntitySpawnerFactory::registerSpawner("game1", createMockSpawner);
    EntitySpawnerFactory::registerSpawner("game2", createMockSpawner);
    EntitySpawnerFactory::registerSpawner("game3", createMockSpawner);

    EntitySpawnerFactory::clearRegistry();

    auto spawners = EntitySpawnerFactory::getRegisteredSpawners();
    EXPECT_TRUE(spawners.empty());
    EXPECT_FALSE(EntitySpawnerFactory::isRegistered("game1"));
    EXPECT_FALSE(EntitySpawnerFactory::isRegistered("game2"));
    EXPECT_FALSE(EntitySpawnerFactory::isRegistered("game3"));
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(EntitySpawnerFactoryTest, ConcurrentRegistration) {
    const int numThreads = 10;
    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i]() {
            std::string gameId = "game-" + std::to_string(i);
            EntitySpawnerFactory::registerSpawner(gameId, createMockSpawner);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto spawners = EntitySpawnerFactory::getRegisteredSpawners();
    EXPECT_EQ(spawners.size(), numThreads);
}

TEST_F(EntitySpawnerFactoryTest, ConcurrentRegistrationAndQuery) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);

    const int numThreads = 10;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&successCount]() {
            if (EntitySpawnerFactory::isRegistered("test-game")) {
                successCount++;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(successCount, numThreads);
}

TEST_F(EntitySpawnerFactoryTest, ConcurrentCreateAndUnregister) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);

    std::thread createThread([this]() {
        for (int i = 0; i < 100; ++i) {
            auto registry = std::make_shared<ECS::Registry>();
            auto spawner = EntitySpawnerFactory::create("test-game", registry, nullptr, std::nullopt, std::nullopt);
            // spawner may be null if unregistered concurrently
        }
    });

    std::thread unregisterThread([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        EntitySpawnerFactory::unregisterSpawner("test-game");
    });

    createThread.join();
    unregisterThread.join();

    // Should not crash - that's the important part
    SUCCEED();
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(EntitySpawnerFactoryTest, RegisterWithSpecialCharacters) {
    bool result = EntitySpawnerFactory::registerSpawner("game-with-dashes_and_underscores", createMockSpawner);
    EXPECT_TRUE(result);
    EXPECT_TRUE(EntitySpawnerFactory::isRegistered("game-with-dashes_and_underscores"));
}

TEST_F(EntitySpawnerFactoryTest, RegisterWithLongGameId) {
    std::string longGameId(1000, 'a');
    bool result = EntitySpawnerFactory::registerSpawner(longGameId, createMockSpawner);
    EXPECT_TRUE(result);
    EXPECT_TRUE(EntitySpawnerFactory::isRegistered(longGameId));
}

TEST_F(EntitySpawnerFactoryTest, UnregisterAfterClear) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    EntitySpawnerFactory::clearRegistry();
    
    bool result = EntitySpawnerFactory::unregisterSpawner("test-game");
    EXPECT_FALSE(result);
}

TEST_F(EntitySpawnerFactoryTest, CreateAfterUnregister) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    EntitySpawnerFactory::unregisterSpawner("test-game");

    auto registry = std::make_shared<ECS::Registry>();
    auto spawner = EntitySpawnerFactory::create("test-game", registry, nullptr, std::nullopt, std::nullopt);

    EXPECT_EQ(spawner, nullptr);
}

TEST_F(EntitySpawnerFactoryTest, RegisterAfterUnregister) {
    EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    EntitySpawnerFactory::unregisterSpawner("test-game");
    
    bool result = EntitySpawnerFactory::registerSpawner("test-game", createMockSpawner);
    EXPECT_TRUE(result);
    EXPECT_TRUE(EntitySpawnerFactory::isRegistered("test-game"));
}

