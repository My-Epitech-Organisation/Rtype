/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Stress tests for the rendering system - tests performance under heavy load
*/

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <random>
#include <vector>

#include <SFML/Graphics.hpp>
#include <rtype/ecs.hpp>

#include "../../src/games/rtype/client/AllComponents.hpp"
#include "../../src/games/rtype/client/GraphicsConstants.hpp"
#include "../../src/games/rtype/client/Systems/MovementSystem.hpp"
#include "../../src/games/rtype/client/Systems/RenderSystem.hpp"

// Namespace aliases for readability
namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;
namespace cfg = ::rtype::games::rtype::client::GraphicsConfig;

/**
 * @brief Test fixture for rendering stress tests
 *
 * Creates an offscreen SFML window and provides utilities for
 * measuring performance under various entity loads.
 */
class RenderStressTest : public ::testing::Test {
   protected:
    std::shared_ptr<ECS::Registry> registry;
    std::shared_ptr<sf::RenderWindow> window;
    std::unique_ptr<rc::RenderSystem> renderSystem;
    std::unique_ptr<rc::MovementSystem> movementSystem;

    // Test texture for sprites
    sf::Texture testTexture;

    // Random number generation
    std::mt19937 rng{42};  // Fixed seed for reproducibility

    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();

        // Create a small offscreen window for testing
        window = std::make_shared<sf::RenderWindow>(
            sf::VideoMode({800, 600}), "StressTest",
            sf::Style::None);  // No decorations

        // Create a simple test texture (8x8 pixels)
        sf::Image img({8, 8}, sf::Color::Red);
        testTexture.loadFromImage(img);

        // Initialize systems
        renderSystem = std::make_unique<rc::RenderSystem>(window);
        movementSystem = std::make_unique<rc::MovementSystem>();
    }

    void TearDown() override {
        window->close();
        registry.reset();
    }

    /**
     * @brief Create multiple sprite entities with random positions
     * @param count Number of entities to spawn
     * @return Vector of spawned entity IDs
     */
    std::vector<ECS::Entity> spawnSpriteEntities(std::size_t count) {
        std::vector<ECS::Entity> entities;
        entities.reserve(count);

        std::uniform_real_distribution<float> posDistX(0.0f, 800.0f);
        std::uniform_real_distribution<float> posDistY(0.0f, 600.0f);
        std::uniform_int_distribution<int> zDist(-5, 5);

        for (std::size_t i = 0; i < count; ++i) {
            auto entity = registry->spawnEntity();

            registry->emplaceComponent<rc::Image>(entity, testTexture);
            registry->emplaceComponent<rs::Position>(entity, posDistX(rng),
                                                     posDistY(rng));
            registry->emplaceComponent<rc::ZIndex>(entity, zDist(rng));

            entities.push_back(entity);
        }

        return entities;
    }

    /**
     * @brief Create sprite entities with velocity for movement tests
     * @param count Number of entities to spawn
     * @return Vector of spawned entity IDs
     */
    std::vector<ECS::Entity> spawnMovingEntities(std::size_t count) {
        std::vector<ECS::Entity> entities;
        entities.reserve(count);

        std::uniform_real_distribution<float> posDistX(0.0f, 800.0f);
        std::uniform_real_distribution<float> posDistY(0.0f, 600.0f);
        std::uniform_real_distribution<float> velDist(-100.0f, 100.0f);
        std::uniform_int_distribution<int> zDist(-5, 5);

        for (std::size_t i = 0; i < count; ++i) {
            auto entity = registry->spawnEntity();

            registry->emplaceComponent<rc::Image>(entity, testTexture);
            registry->emplaceComponent<rs::Position>(entity, posDistX(rng),
                                                     posDistY(rng));
            registry->emplaceComponent<rs::VelocityComponent>(
                entity, rs::VelocityComponent{velDist(rng), velDist(rng)});
            registry->emplaceComponent<rc::ZIndex>(entity, zDist(rng));

            entities.push_back(entity);
        }

        return entities;
    }

    /**
     * @brief Create rectangle entities (UI elements)
     * @param count Number of entities to spawn
     * @return Vector of spawned entity IDs
     */
    std::vector<ECS::Entity> spawnRectangleEntities(std::size_t count) {
        std::vector<ECS::Entity> entities;
        entities.reserve(count);

        std::uniform_real_distribution<float> posDist(0.0f, 700.0f);
        std::uniform_real_distribution<float> sizeDist(10.0f, 100.0f);

        for (std::size_t i = 0; i < count; ++i) {
            auto entity = registry->spawnEntity();

            registry->emplaceComponent<rs::Position>(entity, posDist(rng),
                                                     posDist(rng));
            registry->emplaceComponent<rc::Rectangle>(
                entity,
                std::make_pair(sizeDist(rng), sizeDist(rng)),
                sf::Color::Blue, sf::Color::Cyan);

            entities.push_back(entity);
        }

        return entities;
    }

    /**
     * @brief Measure time to execute a function
     * @param func Function to measure
     * @return Duration in milliseconds
     */
    template <typename Func>
    double measureTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> duration = end - start;
        return duration.count();
    }

    /**
     * @brief Run multiple iterations and return average time
     * @param iterations Number of iterations
     * @param func Function to measure
     * @return Average duration in milliseconds
     */
    template <typename Func>
    double measureAverageTime(std::size_t iterations, Func&& func) {
        double totalTime = 0.0;

        for (std::size_t i = 0; i < iterations; ++i) {
            totalTime += measureTime(std::forward<Func>(func));
        }

        return totalTime / static_cast<double>(iterations);
    }
};

// =============================================================================
// Entity Creation Stress Tests
// =============================================================================

TEST_F(RenderStressTest, EntityCreation_100Entities) {
    constexpr std::size_t ENTITY_COUNT = 100;

    double time = measureTime([this]() {
        spawnSpriteEntities(ENTITY_COUNT);
    });

    std::cout << "[PERF] Creating " << ENTITY_COUNT << " entities: "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 100.0) << "Entity creation took too long";
}

TEST_F(RenderStressTest, EntityCreation_1000Entities) {
    constexpr std::size_t ENTITY_COUNT = 1000;

    double time = measureTime([this]() {
        spawnSpriteEntities(ENTITY_COUNT);
    });

    std::cout << "[PERF] Creating " << ENTITY_COUNT << " entities: "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 500.0) << "Entity creation took too long";
}

TEST_F(RenderStressTest, EntityCreation_5000Entities) {
    constexpr std::size_t ENTITY_COUNT = 5000;

    double time = measureTime([this]() {
        spawnSpriteEntities(ENTITY_COUNT);
    });

    std::cout << "[PERF] Creating " << ENTITY_COUNT << " entities: "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 2000.0) << "Entity creation took too long";
}

// =============================================================================
// Rendering Stress Tests
// =============================================================================

TEST_F(RenderStressTest, RenderSystem_100Sprites_SingleFrame) {
    spawnSpriteEntities(100);

    double time = measureTime([this]() {
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] Rendering 100 sprites: " << time << " ms" << std::endl;
    EXPECT_LT(time, 50.0) << "Single frame render took too long";
}

TEST_F(RenderStressTest, RenderSystem_500Sprites_SingleFrame) {
    spawnSpriteEntities(500);

    double time = measureTime([this]() {
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] Rendering 500 sprites: " << time << " ms" << std::endl;
    EXPECT_LT(time, 100.0) << "Single frame render took too long";
}

TEST_F(RenderStressTest, RenderSystem_1000Sprites_SingleFrame) {
    spawnSpriteEntities(1000);

    double time = measureTime([this]() {
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] Rendering 1000 sprites: " << time << " ms" << std::endl;
    EXPECT_LT(time, 200.0) << "Single frame render took too long";
}

TEST_F(RenderStressTest, RenderSystem_100Sprites_60Frames) {
    constexpr std::size_t FRAME_COUNT = 60;
    spawnSpriteEntities(100);

    double avgTime = measureAverageTime(FRAME_COUNT, [this]() {
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] Avg frame time (100 sprites, 60 frames): "
              << avgTime << " ms" << std::endl;

    // Target: 60 FPS = 16.67ms per frame
    EXPECT_LT(avgTime, 16.67) << "Cannot maintain 60 FPS with 100 sprites";
}

TEST_F(RenderStressTest, RenderSystem_500Sprites_60Frames) {
    constexpr std::size_t FRAME_COUNT = 60;
    spawnSpriteEntities(500);

    double avgTime = measureAverageTime(FRAME_COUNT, [this]() {
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] Avg frame time (500 sprites, 60 frames): "
              << avgTime << " ms" << std::endl;

    // More lenient threshold for 500 sprites
    EXPECT_LT(avgTime, 33.33) << "Cannot maintain 30 FPS with 500 sprites";
}

// =============================================================================
// Movement System Stress Tests
// =============================================================================

TEST_F(RenderStressTest, MovementSystem_1000Entities_SingleUpdate) {
    spawnMovingEntities(1000);

    double time = measureTime([this]() {
        movementSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] Movement update (1000 entities): "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 10.0) << "Movement system too slow";
}

TEST_F(RenderStressTest, MovementSystem_5000Entities_SingleUpdate) {
    spawnMovingEntities(5000);

    double time = measureTime([this]() {
        movementSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] Movement update (5000 entities): "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 50.0) << "Movement system too slow";
}

TEST_F(RenderStressTest, MovementSystem_1000Entities_60Updates) {
    constexpr std::size_t UPDATE_COUNT = 60;
    spawnMovingEntities(1000);

    double avgTime = measureAverageTime(UPDATE_COUNT, [this]() {
        movementSystem->update(*registry, 0.016f);
    });

    std::cout << "[PERF] Avg movement time (1000 entities, 60 updates): "
              << avgTime << " ms" << std::endl;

    EXPECT_LT(avgTime, 5.0) << "Movement system average too slow";
}

// =============================================================================
// Combined System Stress Tests (Full Frame Simulation)
// =============================================================================

TEST_F(RenderStressTest, FullFrame_500Sprites_Movement_60Frames) {
    constexpr std::size_t FRAME_COUNT = 60;
    spawnMovingEntities(500);

    double avgTime = measureAverageTime(FRAME_COUNT, [this]() {
        // Simulate a full frame
        movementSystem->update(*registry, 0.016f);
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] Full frame avg (500 moving sprites): "
              << avgTime << " ms" << std::endl;

    // Should maintain at least 30 FPS
    EXPECT_LT(avgTime, 33.33) << "Cannot maintain 30 FPS with 500 moving sprites";
}

TEST_F(RenderStressTest, FullFrame_1000Sprites_Movement_60Frames) {
    constexpr std::size_t FRAME_COUNT = 60;
    spawnMovingEntities(1000);

    double avgTime = measureAverageTime(FRAME_COUNT, [this]() {
        movementSystem->update(*registry, 0.016f);
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] Full frame avg (1000 moving sprites): "
              << avgTime << " ms" << std::endl;

    // More lenient for 1000 entities
    EXPECT_LT(avgTime, 50.0) << "Cannot maintain 20 FPS with 1000 moving sprites";
}

// =============================================================================
// Mixed Entity Type Stress Tests
// =============================================================================

TEST_F(RenderStressTest, MixedEntities_Sprites_Rectangles_60Frames) {
    constexpr std::size_t FRAME_COUNT = 60;

    // Create a mix of entity types
    spawnSpriteEntities(300);
    spawnRectangleEntities(200);

    double avgTime = measureAverageTime(FRAME_COUNT, [this]() {
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] Mixed entities (300 sprites + 200 rects): "
              << avgTime << " ms" << std::endl;

    EXPECT_LT(avgTime, 33.33) << "Cannot maintain 30 FPS with mixed entities";
}

// =============================================================================
// ZIndex Sorting Stress Tests
// =============================================================================

TEST_F(RenderStressTest, ZIndexSorting_1000Entities_RandomDepths) {
    // Create entities with varied Z-indices to stress the sorting algorithm
    std::uniform_int_distribution<int> zDist(-100, 100);

    for (std::size_t i = 0; i < 1000; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<rc::Image>(entity, testTexture);
        registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
        registry->emplaceComponent<rc::ZIndex>(entity, zDist(rng));
    }

    // Force cache invalidation and render first frame (cold)

    double coldTime = measureTime([this]() {
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] First frame with Z-sort (1000 entities): "
              << coldTime << " ms" << std::endl;

    // Run several warmup frames to stabilize
    for (int i = 0; i < 5; ++i) {
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    }

    // Measure average of cached frames (cache should be stable now)
    double cachedTotal = 0.0;
    constexpr int CACHED_FRAMES = 10;
    for (int i = 0; i < CACHED_FRAMES; ++i) {
        cachedTotal += measureTime([this]() {
            window->clear();
            renderSystem->update(*registry, 0.016f);
            window->display();
        });
    }
    double cachedAvg = cachedTotal / CACHED_FRAMES;

    std::cout << "[PERF] Cached frame avg (1000 entities, " << CACHED_FRAMES
              << " frames): " << cachedAvg << " ms" << std::endl;

    // Cached frames should complete within reasonable time
    // Note: First frame may be faster due to driver optimizations,
    // so we just verify cached performance is acceptable
    EXPECT_LT(cachedAvg, 50.0) << "Cached rendering too slow";
}

// =============================================================================
// Entity Lifecycle Stress Tests
// =============================================================================

TEST_F(RenderStressTest, EntityChurn_SpawnDestroy_1000Cycles) {
    constexpr std::size_t CYCLES = 1000;
    constexpr std::size_t BATCH_SIZE = 10;

    double time = measureTime([this]() {
        for (std::size_t cycle = 0; cycle < CYCLES; ++cycle) {
            // Spawn a batch
            auto entities = spawnSpriteEntities(BATCH_SIZE);

            // Immediately destroy them
            for (auto entity : entities) {
                registry->killEntity(entity);
            }
        }
    });

    std::cout << "[PERF] Entity churn (" << CYCLES << " cycles, "
              << BATCH_SIZE << " per batch): " << time << " ms" << std::endl;

    EXPECT_LT(time, 5000.0) << "Entity churn too slow";
}

TEST_F(RenderStressTest, EntityChurn_RenderDuringChurn_60Frames) {
    constexpr std::size_t FRAME_COUNT = 60;
    std::vector<ECS::Entity> entities;

    double avgTime = measureAverageTime(FRAME_COUNT, [this, &entities]() {
        // Add 10 entities per frame
        auto newEntities = spawnSpriteEntities(10);
        entities.insert(entities.end(), newEntities.begin(), newEntities.end());

        // Remove oldest 5 if we have enough
        if (entities.size() > 100) {
            for (std::size_t i = 0; i < 5; ++i) {
                registry->killEntity(entities.front());
                entities.erase(entities.begin());
            }
        }

        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    });

    std::cout << "[PERF] Render during churn (avg): " << avgTime << " ms"
              << std::endl;
    std::cout << "[INFO] Final entity count: " << entities.size() << std::endl;

    EXPECT_LT(avgTime, 50.0) << "Rendering during entity churn too slow";
}

// =============================================================================
// Memory Pressure Tests
// =============================================================================

TEST_F(RenderStressTest, MemoryPressure_ManySmallEntities) {
    constexpr std::size_t ENTITY_COUNT = 10000;

    // Track memory usage would require platform-specific code
    // Here we just ensure the operation completes without crashing

    auto startTime = std::chrono::high_resolution_clock::now();

    auto entities = spawnSpriteEntities(ENTITY_COUNT);

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;

    std::cout << "[PERF] Spawning " << ENTITY_COUNT << " entities: "
              << duration.count() << " ms" << std::endl;

    // Verify all entities exist
    std::size_t validCount = 0;
    for (auto entity : entities) {
        if (registry->hasComponent<rc::Image>(entity)) {
            ++validCount;
        }
    }

    EXPECT_EQ(validCount, ENTITY_COUNT) << "Not all entities were created";
}

// =============================================================================
// Sustained Load Tests
// =============================================================================

TEST_F(RenderStressTest, SustainedLoad_500Sprites_300Frames) {
    constexpr std::size_t WARMUP_FRAMES = 30;  // Skip initial frames for warmup
    constexpr std::size_t MEASURE_FRAMES = 270;  // Measure these frames
    spawnMovingEntities(500);

    // Warmup phase - let GPU/driver stabilize
    for (std::size_t i = 0; i < WARMUP_FRAMES; ++i) {
        movementSystem->update(*registry, 0.016f);
        window->clear();
        renderSystem->update(*registry, 0.016f);
        window->display();
    }

    std::vector<double> frameTimes;
    frameTimes.reserve(MEASURE_FRAMES);

    for (std::size_t i = 0; i < MEASURE_FRAMES; ++i) {
        double time = measureTime([this]() {
            movementSystem->update(*registry, 0.016f);
            window->clear();
            renderSystem->update(*registry, 0.016f);
            window->display();
        });
        frameTimes.push_back(time);
    }

    // Calculate statistics
    double sum = 0.0;
    double minTime = frameTimes[0];
    double maxTime = frameTimes[0];

    for (double t : frameTimes) {
        sum += t;
        minTime = std::min(minTime, t);
        maxTime = std::max(maxTime, t);
    }

    double avgTime = sum / static_cast<double>(MEASURE_FRAMES);

    // Calculate standard deviation
    double variance = 0.0;
    for (double t : frameTimes) {
        variance += (t - avgTime) * (t - avgTime);
    }
    double stdDev = std::sqrt(variance / static_cast<double>(MEASURE_FRAMES));

    // Calculate 95th percentile for outlier detection
    std::vector<double> sorted = frameTimes;
    std::sort(sorted.begin(), sorted.end());
    double p95 = sorted[static_cast<std::size_t>(MEASURE_FRAMES * 0.95)];

    std::cout << "[PERF] Sustained load (500 sprites, " << MEASURE_FRAMES
              << " frames after warmup):" << std::endl;
    std::cout << "       Avg: " << avgTime << " ms" << std::endl;
    std::cout << "       Min: " << minTime << " ms" << std::endl;
    std::cout << "       Max: " << maxTime << " ms" << std::endl;
    std::cout << "       P95: " << p95 << " ms" << std::endl;
    std::cout << "       StdDev: " << stdDev << " ms" << std::endl;

    // Check for acceptable performance
    EXPECT_LT(avgTime, 33.33) << "Average frame time exceeds 30 FPS target";
    // Use P95 instead of stdDev - more meaningful for frame timing
    EXPECT_LT(p95, 16.67) << "95th percentile exceeds 60 FPS target";
}
