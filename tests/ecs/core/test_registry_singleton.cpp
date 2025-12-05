/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Unit tests for Registry - Singleton Management
*/

#include <gtest/gtest.h>
#include "../../../lib/rtype_ecs/src/ecs/core/Registry/Registry.hpp"
#include <string>
#include <memory>

using namespace ECS;

// ============================================================================
// TEST SINGLETONS
// ============================================================================

struct GameConfig {
    int difficulty = 1;
    float volume = 0.5f;
    std::string playerName = "Player1";

    GameConfig() = default;
    GameConfig(int d, float v, std::string name)
        : difficulty(d), volume(v), playerName(std::move(name)) {}
};

struct RenderSettings {
    int width = 1920;
    int height = 1080;
    bool fullscreen = false;

    RenderSettings() = default;
    RenderSettings(int w, int h, bool fs) : width(w), height(h), fullscreen(fs) {}
};

struct GameTime {
    float deltaTime = 0.016f;
    float totalTime = 0.0f;
};

// Singleton with non-trivial resource (must be copy/move constructible for std::any)
struct AssetCache {
    int cacheSize = 0;

    AssetCache() = default;
    explicit AssetCache(int size) : cacheSize(size) {}
};

// ============================================================================
// TEST FIXTURE
// ============================================================================

class RegistrySingletonTest : public ::testing::Test {
protected:
    Registry registry;
};

// ============================================================================
// SET SINGLETON TESTS
// ============================================================================

TEST_F(RegistrySingletonTest, SetSingleton_BasicType_Success) {
    GameConfig& config = registry.setSingleton<GameConfig>();

    EXPECT_EQ(config.difficulty, 1);
    EXPECT_EQ(config.volume, 0.5f);
    EXPECT_EQ(config.playerName, "Player1");
}

TEST_F(RegistrySingletonTest, SetSingleton_WithArguments) {
    GameConfig& config = registry.setSingleton<GameConfig>(5, 0.8f, "Hero");

    EXPECT_EQ(config.difficulty, 5);
    EXPECT_EQ(config.volume, 0.8f);
    EXPECT_EQ(config.playerName, "Hero");
}

TEST_F(RegistrySingletonTest, SetSingleton_MultipleDifferentTypes) {
    registry.setSingleton<GameConfig>(2, 0.7f, "Test");
    registry.setSingleton<RenderSettings>(1280, 720, true);
    registry.setSingleton<GameTime>();

    EXPECT_TRUE(registry.hasSingleton<GameConfig>());
    EXPECT_TRUE(registry.hasSingleton<RenderSettings>());
    EXPECT_TRUE(registry.hasSingleton<GameTime>());
}

TEST_F(RegistrySingletonTest, SetSingleton_Override_ReplacesOld) {
    registry.setSingleton<GameConfig>(1, 0.5f, "Old");
    registry.setSingleton<GameConfig>(10, 1.0f, "New");

    GameConfig& config = registry.getSingleton<GameConfig>();

    EXPECT_EQ(config.difficulty, 10);
    EXPECT_EQ(config.volume, 1.0f);
    EXPECT_EQ(config.playerName, "New");
}

TEST_F(RegistrySingletonTest, SetSingleton_WithResource) {
    AssetCache& cache = registry.setSingleton<AssetCache>(42);

    EXPECT_EQ(cache.cacheSize, 42);
}

// ============================================================================
// GET SINGLETON TESTS
// ============================================================================

TEST_F(RegistrySingletonTest, GetSingleton_Exists_ReturnsReference) {
    registry.setSingleton<GameConfig>(3, 0.6f, "Player");

    GameConfig& config = registry.getSingleton<GameConfig>();

    EXPECT_EQ(config.difficulty, 3);
}

TEST_F(RegistrySingletonTest, GetSingleton_ModifyReference) {
    registry.setSingleton<GameTime>();

    registry.getSingleton<GameTime>().deltaTime = 0.033f;

    EXPECT_EQ(registry.getSingleton<GameTime>().deltaTime, 0.033f);
}

TEST_F(RegistrySingletonTest, GetSingleton_NotExists_Throws) {
    EXPECT_THROW(
        registry.getSingleton<GameConfig>(),
        std::out_of_range
    );
}

TEST_F(RegistrySingletonTest, GetSingleton_MultipleAccesses_SameInstance) {
    registry.setSingleton<GameConfig>();

    GameConfig& ref1 = registry.getSingleton<GameConfig>();
    GameConfig& ref2 = registry.getSingleton<GameConfig>();

    EXPECT_EQ(&ref1, &ref2);
}

// ============================================================================
// HAS SINGLETON TESTS
// ============================================================================

TEST_F(RegistrySingletonTest, HasSingleton_Exists_ReturnsTrue) {
    registry.setSingleton<GameConfig>();

    EXPECT_TRUE(registry.hasSingleton<GameConfig>());
}

TEST_F(RegistrySingletonTest, HasSingleton_NotExists_ReturnsFalse) {
    EXPECT_FALSE(registry.hasSingleton<GameConfig>());
}

TEST_F(RegistrySingletonTest, HasSingleton_AfterRemove_ReturnsFalse) {
    registry.setSingleton<GameConfig>();
    registry.removeSingleton<GameConfig>();

    EXPECT_FALSE(registry.hasSingleton<GameConfig>());
}

TEST_F(RegistrySingletonTest, HasSingleton_DifferentTypes_Independent) {
    registry.setSingleton<GameConfig>();

    EXPECT_TRUE(registry.hasSingleton<GameConfig>());
    EXPECT_FALSE(registry.hasSingleton<RenderSettings>());
    EXPECT_FALSE(registry.hasSingleton<GameTime>());
}

// ============================================================================
// REMOVE SINGLETON TESTS
// ============================================================================

TEST_F(RegistrySingletonTest, RemoveSingleton_Exists_Removes) {
    registry.setSingleton<GameConfig>();

    registry.removeSingleton<GameConfig>();

    EXPECT_FALSE(registry.hasSingleton<GameConfig>());
}

TEST_F(RegistrySingletonTest, RemoveSingleton_NotExists_NoEffect) {
    // Should not throw
    registry.removeSingleton<GameConfig>();

    EXPECT_FALSE(registry.hasSingleton<GameConfig>());
}

TEST_F(RegistrySingletonTest, RemoveSingleton_KeepsOtherSingletons) {
    registry.setSingleton<GameConfig>();
    registry.setSingleton<RenderSettings>();
    registry.setSingleton<GameTime>();

    registry.removeSingleton<RenderSettings>();

    EXPECT_TRUE(registry.hasSingleton<GameConfig>());
    EXPECT_FALSE(registry.hasSingleton<RenderSettings>());
    EXPECT_TRUE(registry.hasSingleton<GameTime>());
}

TEST_F(RegistrySingletonTest, RemoveSingleton_Resource_ProperlyDestructs) {
    registry.setSingleton<AssetCache>(999);

    // Should properly destruct
    registry.removeSingleton<AssetCache>();

    EXPECT_FALSE(registry.hasSingleton<AssetCache>());
}

TEST_F(RegistrySingletonTest, RemoveSingleton_ThenReCreate) {
    registry.setSingleton<GameConfig>(1, 0.5f, "First");
    registry.removeSingleton<GameConfig>();
    registry.setSingleton<GameConfig>(2, 0.8f, "Second");

    GameConfig& config = registry.getSingleton<GameConfig>();

    EXPECT_EQ(config.difficulty, 2);
    EXPECT_EQ(config.playerName, "Second");
}

// ============================================================================
// SINGLETON LIFECYCLE TESTS
// ============================================================================

TEST_F(RegistrySingletonTest, Singleton_PersistsAcrossEntityOperations) {
    registry.setSingleton<GameConfig>(5, 0.9f, "Persistent");

    // Create and destroy some entities
    for (int i = 0; i < 100; ++i) {
        Entity e = registry.spawnEntity();
        if (i % 2 == 0) {
            registry.killEntity(e);
        }
    }

    // Singleton should still exist and be unchanged
    GameConfig& config = registry.getSingleton<GameConfig>();
    EXPECT_EQ(config.difficulty, 5);
    EXPECT_EQ(config.playerName, "Persistent");
}

TEST_F(RegistrySingletonTest, Singleton_IndependentOfComponents) {
    Entity e = registry.spawnEntity();
    registry.setSingleton<GameConfig>();

    // Adding component of same type shouldn't affect singleton
    struct GameConfig_Component {
        int value = 42;
    };
    registry.emplaceComponent<GameConfig_Component>(e);

    EXPECT_TRUE(registry.hasSingleton<GameConfig>());
    EXPECT_TRUE(registry.hasComponent<GameConfig_Component>(e));

    // Different types, different storage
    EXPECT_EQ(registry.getSingleton<GameConfig>().difficulty, 1);
    EXPECT_EQ(registry.getComponent<GameConfig_Component>(e).value, 42);
}

// ============================================================================
// PRIMITIVE TYPE SINGLETONS
// ============================================================================

TEST_F(RegistrySingletonTest, Singleton_IntType) {
    registry.setSingleton<int>(42);

    EXPECT_TRUE(registry.hasSingleton<int>());
    EXPECT_EQ(registry.getSingleton<int>(), 42);

    registry.getSingleton<int>() = 100;
    EXPECT_EQ(registry.getSingleton<int>(), 100);
}

TEST_F(RegistrySingletonTest, Singleton_StringType) {
    registry.setSingleton<std::string>("Hello World");

    EXPECT_TRUE(registry.hasSingleton<std::string>());
    EXPECT_EQ(registry.getSingleton<std::string>(), "Hello World");
}

TEST_F(RegistrySingletonTest, Singleton_FloatType) {
    registry.setSingleton<float>(3.14159f);

    EXPECT_TRUE(registry.hasSingleton<float>());
    EXPECT_FLOAT_EQ(registry.getSingleton<float>(), 3.14159f);
}
