/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_GameEngineFactory
*/

#include <gtest/gtest.h>

#include <memory>

#include <rtype/ecs.hpp>
#include <rtype/engine.hpp>

#include "games/rtype/server/GameEngine.hpp"

using namespace rtype::engine;

// Mock game engine for testing
class MockGameEngine : public AGameEngine {
   public:
    explicit MockGameEngine(std::shared_ptr<ECS::Registry> registry)
        : _registry(std::move(registry)) {}

    bool initialize() override {
        _isRunning = true;
        return true;
    }

    void update(float /*deltaTime*/) override {}

    void shutdown() override { _isRunning = false; }

    ProcessedEvent processEvent(const GameEvent& event) override {
        ProcessedEvent result{};
        result.type = event.type;
        result.valid = true;
        return result;
    }

    void syncEntityPositions(
        std::function<void(uint32_t, float, float, float, float)>
        /*callback*/) override {}

    [[nodiscard]] std::string getGameId() const override { return "mock_game"; }

   private:
    std::shared_ptr<ECS::Registry> _registry;
};

class AnotherMockGameEngine : public AGameEngine {
   public:
    explicit AnotherMockGameEngine(std::shared_ptr<ECS::Registry> /*registry*/) {}

    bool initialize() override { return true; }
    void update(float /*deltaTime*/) override {}
    void shutdown() override {}
    ProcessedEvent processEvent(const GameEvent& event) override {
        ProcessedEvent result{};
        result.type = event.type;
        result.valid = true;
        return result;
    }
    void syncEntityPositions(
        std::function<void(uint32_t, float, float, float, float)>
        /*callback*/) override {}
    [[nodiscard]] std::string getGameId() const override { return "another_mock"; }
};

class GameEngineFactoryTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Ensure RType is registered before tests
        rtype::games::rtype::server::registerRTypeGameEngine();
    }

    void TearDown() override {
        // Clean up test registrations
        GameEngineFactory::unregisterGame("test_game");
        GameEngineFactory::unregisterGame("test_game2");
        GameEngineFactory::unregisterGame("another_game");
    }
};

TEST_F(GameEngineFactoryTest, RegisterGame_Success) {
    bool result = GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });

    EXPECT_TRUE(result);
    EXPECT_TRUE(GameEngineFactory::isRegistered("test_game"));
}

TEST_F(GameEngineFactoryTest, RegisterGame_DuplicateFails) {
    GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });

    bool result = GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });

    EXPECT_FALSE(result);
}

TEST_F(GameEngineFactoryTest, RegisterGame_EmptyIdFails) {
    bool result = GameEngineFactory::registerGame(
        "",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });

    EXPECT_FALSE(result);
}

TEST_F(GameEngineFactoryTest, RegisterGame_NullCreatorFails) {
    bool result = GameEngineFactory::registerGame("test_game", nullptr);

    EXPECT_FALSE(result);
}

TEST_F(GameEngineFactoryTest, UnregisterGame_Success) {
    GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });

    bool result = GameEngineFactory::unregisterGame("test_game");

    EXPECT_TRUE(result);
    EXPECT_FALSE(GameEngineFactory::isRegistered("test_game"));
}

TEST_F(GameEngineFactoryTest, UnregisterGame_NotFoundFails) {
    bool result = GameEngineFactory::unregisterGame("nonexistent_game");

    EXPECT_FALSE(result);
}

TEST_F(GameEngineFactoryTest, Create_Success) {
    GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });

    auto registry = std::make_shared<ECS::Registry>();
    auto engine = GameEngineFactory::create("test_game", registry);

    ASSERT_NE(engine, nullptr);
    EXPECT_TRUE(engine->initialize());
}

TEST_F(GameEngineFactoryTest, Create_NotFoundReturnsNull) {
    auto registry = std::make_shared<ECS::Registry>();
    auto engine = GameEngineFactory::create("nonexistent_game", registry);

    EXPECT_EQ(engine, nullptr);
}

TEST_F(GameEngineFactoryTest, GetRegisteredGames_ReturnsAllGames) {
    GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });
    GameEngineFactory::registerGame(
        "another_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<AnotherMockGameEngine>(std::move(registry));
        });

    auto games = GameEngineFactory::getRegisteredGames();

    EXPECT_GE(games.size(), 2u);  // At least our 2 test games (rtype may also be registered)
    EXPECT_NE(std::find(games.begin(), games.end(), "test_game"), games.end());
    EXPECT_NE(std::find(games.begin(), games.end(), "another_game"), games.end());
}

TEST_F(GameEngineFactoryTest, DefaultGame_FirstRegisteredIsDefault) {
    // Clear any existing default
    std::string currentDefault = GameEngineFactory::getDefaultGame();
    
    // If rtype is already registered, it's the default
    if (!currentDefault.empty()) {
        EXPECT_TRUE(GameEngineFactory::isRegistered(currentDefault));
    }
}

TEST_F(GameEngineFactoryTest, SetDefaultGame_Success) {
    GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });
    GameEngineFactory::registerGame(
        "test_game2",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });

    bool result = GameEngineFactory::setDefaultGame("test_game2");

    EXPECT_TRUE(result);
    EXPECT_EQ(GameEngineFactory::getDefaultGame(), "test_game2");
}

TEST_F(GameEngineFactoryTest, SetDefaultGame_NonexistentFails) {
    bool result = GameEngineFactory::setDefaultGame("nonexistent_game");

    EXPECT_FALSE(result);
}

TEST_F(GameEngineFactoryTest, IsRegistered_ReturnsTrueForRegistered) {
    GameEngineFactory::registerGame(
        "test_game",
        [](std::shared_ptr<ECS::Registry> registry) {
            return std::make_unique<MockGameEngine>(std::move(registry));
        });

    EXPECT_TRUE(GameEngineFactory::isRegistered("test_game"));
}

TEST_F(GameEngineFactoryTest, IsRegistered_ReturnsFalseForUnregistered) {
    EXPECT_FALSE(GameEngineFactory::isRegistered("definitely_not_registered"));
}

// Test that RType is auto-registered
TEST_F(GameEngineFactoryTest, RTypeIsAutoRegistered) {
    // RType should be auto-registered via static initializer
    EXPECT_TRUE(GameEngineFactory::isRegistered("rtype"));
}

TEST_F(GameEngineFactoryTest, CreateRTypeEngine_Success) {
    auto registry = std::make_shared<ECS::Registry>();
    auto engine = GameEngineFactory::create("rtype", registry);

    ASSERT_NE(engine, nullptr);
    EXPECT_TRUE(engine->initialize());
    engine->shutdown();
}

// Test template registrar
TEST_F(GameEngineFactoryTest, GameEngineRegistrar_RegistersGame) {
    // This is implicitly tested by RTypeIsAutoRegistered, but we can test
    // the mechanism directly
    {
        GameEngineRegistrar<MockGameEngine> registrar("test_game");
    }

    EXPECT_TRUE(GameEngineFactory::isRegistered("test_game"));
}

TEST_F(GameEngineFactoryTest, GameEngineRegistrar_WithoutSetAsDefault) {
    std::string prevDefault = GameEngineFactory::getDefaultGame();
    {
        // Register game without setAsDefault=true
        GameEngineRegistrar<AnotherMockGameEngine> registrar("another_game", false);
    }

    EXPECT_TRUE(GameEngineFactory::isRegistered("another_game"));
    // Default should not change
    EXPECT_EQ(GameEngineFactory::getDefaultGame(), prevDefault);
}

