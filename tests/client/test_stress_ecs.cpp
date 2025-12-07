/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Stress tests for ECS component operations - tests performance of
** component add/remove/query operations under heavy load
*/

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <random>
#include <vector>
#include <algorithm>
#include <numeric>

#include <SFML/Graphics.hpp>
#include <ecs/ECS.hpp>

#include "../../src/games/rtype/client/AllComponents.hpp"

// Namespace aliases
namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

/**
 * @brief Test fixture for ECS component stress tests
 *
 * Tests the performance of adding, removing, and querying components
 * on large numbers of entities.
 */
class ECSComponentStressTest : public ::testing::Test {
   protected:
    std::shared_ptr<ECS::Registry> registry;
    sf::Texture testTexture;
    sf::Font testFont;

    std::mt19937 rng{12345};

    void SetUp() override {
        registry = std::make_shared<ECS::Registry>();

        // Create test texture
        sf::Image img({8, 8}, sf::Color::Green);
        testTexture.loadFromImage(img);
    }

    void TearDown() override {
        registry.reset();
    }

    template <typename Func>
    double measureTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> duration = end - start;
        return duration.count();
    }

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
// Component Addition Stress Tests
// =============================================================================

TEST_F(ECSComponentStressTest, AddSingleComponent_1000Entities) {
    constexpr std::size_t COUNT = 1000;
    std::vector<ECS::Entity> entities;
    entities.reserve(COUNT);

    // First spawn all entities
    for (std::size_t i = 0; i < COUNT; ++i) {
        entities.push_back(registry->spawnEntity());
    }

    double time = measureTime([this, &entities]() {
        for (auto entity : entities) {
            registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
        }
    });

    std::cout << "[PERF] Add Position to " << COUNT << " entities: "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 100.0) << "Adding components too slow";
}

TEST_F(ECSComponentStressTest, AddMultipleComponents_1000Entities) {
    constexpr std::size_t COUNT = 1000;
    std::vector<ECS::Entity> entities;
    entities.reserve(COUNT);

    for (std::size_t i = 0; i < COUNT; ++i) {
        entities.push_back(registry->spawnEntity());
    }

    double time = measureTime([this, &entities]() {
        for (auto entity : entities) {
            registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
            registry->emplaceComponent<rs::VelocityComponent>(
                entity, rs::VelocityComponent{1.0f, 1.0f});
            registry->emplaceComponent<rc::ZIndex>(entity, 0);
        }
    });

    std::cout << "[PERF] Add 3 components to " << COUNT << " entities: "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 200.0) << "Adding multiple components too slow";
}

TEST_F(ECSComponentStressTest, AddHeavyComponent_1000Entities) {
    constexpr std::size_t COUNT = 1000;
    std::vector<ECS::Entity> entities;
    entities.reserve(COUNT);

    for (std::size_t i = 0; i < COUNT; ++i) {
        entities.push_back(registry->spawnEntity());
    }

    // Image component contains SFML Sprite which is heavier
    double time = measureTime([this, &entities]() {
        for (auto entity : entities) {
            registry->emplaceComponent<rc::Image>(entity, testTexture);
        }
    });

    std::cout << "[PERF] Add Image component to " << COUNT << " entities: "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 500.0) << "Adding heavy components too slow";
}

TEST_F(ECSComponentStressTest, AddComponents_5000Entities_AllTypes) {
    constexpr std::size_t COUNT = 5000;
    std::vector<ECS::Entity> entities;
    entities.reserve(COUNT);

    double spawnTime = measureTime([this, &entities, COUNT]() {
        for (std::size_t i = 0; i < COUNT; ++i) {
            entities.push_back(registry->spawnEntity());
        }
    });

    double componentTime = measureTime([this, &entities]() {
        for (auto entity : entities) {
            registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
            registry->emplaceComponent<rs::VelocityComponent>(
                entity, rs::VelocityComponent{0.0f, 0.0f});
            registry->emplaceComponent<rc::ZIndex>(entity, 0);
            registry->emplaceComponent<rc::Image>(entity, testTexture);
        }
    });

    std::cout << "[PERF] Spawn " << COUNT << " entities: "
              << spawnTime << " ms" << std::endl;
    std::cout << "[PERF] Add 4 components to all: "
              << componentTime << " ms" << std::endl;

    EXPECT_LT(componentTime, 2000.0) << "Bulk component addition too slow";
}

// =============================================================================
// Component Query Stress Tests
// =============================================================================

TEST_F(ECSComponentStressTest, QuerySingleComponent_1000Entities) {
    constexpr std::size_t COUNT = 1000;

    // Setup: create entities with Position
    for (std::size_t i = 0; i < COUNT; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(entity,
            static_cast<float>(i), static_cast<float>(i));
    }

    std::size_t iterationCount = 0;
    double time = measureTime([this, &iterationCount]() {
        registry->view<rs::Position>().each(
            [&iterationCount](auto entity, auto& pos) {
                ++iterationCount;
                pos.x += 1.0f;  // Modify to prevent optimization
            });
    });

    std::cout << "[PERF] Query Position (" << COUNT << " entities): "
              << time << " ms" << std::endl;

    EXPECT_EQ(iterationCount, COUNT);
    EXPECT_LT(time, 10.0) << "Single component query too slow";
}

TEST_F(ECSComponentStressTest, QueryMultipleComponents_1000Entities) {
    constexpr std::size_t COUNT = 1000;

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
        registry->emplaceComponent<rs::VelocityComponent>(
            entity, rs::VelocityComponent{1.0f, 1.0f});
        registry->emplaceComponent<rc::ZIndex>(entity, 0);
    }

    std::size_t iterationCount = 0;
    double time = measureTime([this, &iterationCount]() {
        registry->view<rs::Position, rs::VelocityComponent, rc::ZIndex>().each(
            [&iterationCount](auto entity, auto& pos, auto& vel, auto& z) {
                ++iterationCount;
                pos.x += vel.vx;
                pos.y += vel.vy;
            });
    });

    std::cout << "[PERF] Query 3 components (" << COUNT << " entities): "
              << time << " ms" << std::endl;

    EXPECT_EQ(iterationCount, COUNT);
    EXPECT_LT(time, 20.0) << "Multi-component query too slow";
}

TEST_F(ECSComponentStressTest, QueryWithFilter_5000Entities) {
    constexpr std::size_t COUNT = 5000;
    constexpr std::size_t MATCHING = 2500;  // Half will match

    std::uniform_int_distribution<int> dist(0, 1);

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);

        // Only half get velocity
        if (i < MATCHING) {
            registry->emplaceComponent<rs::VelocityComponent>(
                entity, rs::VelocityComponent{1.0f, 1.0f});
        }
    }

    std::size_t matchCount = 0;
    double time = measureTime([this, &matchCount]() {
        registry->view<rs::Position, rs::VelocityComponent>().each(
            [&matchCount](auto entity, auto& pos, auto& vel) {
                ++matchCount;
            });
    });

    std::cout << "[PERF] Filtered query (" << matchCount << "/" << COUNT
              << " match): " << time << " ms" << std::endl;

    EXPECT_EQ(matchCount, MATCHING);
    EXPECT_LT(time, 20.0) << "Filtered query too slow";
}

TEST_F(ECSComponentStressTest, RepeatedQueries_100Iterations) {
    constexpr std::size_t COUNT = 1000;
    constexpr std::size_t ITERATIONS = 100;

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
        registry->emplaceComponent<rs::VelocityComponent>(
            entity, rs::VelocityComponent{1.0f, 1.0f});
    }

    double avgTime = measureAverageTime(ITERATIONS, [this]() {
        registry->view<rs::Position, rs::VelocityComponent>().each(
            [](auto entity, auto& pos, auto& vel) {
                pos.x += vel.vx;
                pos.y += vel.vy;
            });
    });

    std::cout << "[PERF] Avg query time (" << ITERATIONS << " iterations): "
              << avgTime << " ms" << std::endl;

    EXPECT_LT(avgTime, 5.0) << "Repeated queries too slow";
}

// =============================================================================
// Component Removal Stress Tests
// =============================================================================

TEST_F(ECSComponentStressTest, RemoveComponent_1000Entities) {
    constexpr std::size_t COUNT = 1000;
    std::vector<ECS::Entity> entities;
    entities.reserve(COUNT);

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
        registry->emplaceComponent<rs::VelocityComponent>(
            entity, rs::VelocityComponent{0.0f, 0.0f});
        entities.push_back(entity);
    }

    double time = measureTime([this, &entities]() {
        for (auto entity : entities) {
            registry->removeComponent<rs::VelocityComponent>(entity);
        }
    });

    std::cout << "[PERF] Remove component from " << COUNT << " entities: "
              << time << " ms" << std::endl;

    // Verify removal
    std::size_t remaining = 0;
    registry->view<rs::VelocityComponent>().each(
        [&remaining](auto, auto&) { ++remaining; });

    EXPECT_EQ(remaining, 0);
    EXPECT_LT(time, 100.0) << "Component removal too slow";
}

TEST_F(ECSComponentStressTest, EntityDestruction_1000Entities) {
    constexpr std::size_t COUNT = 1000;
    std::vector<ECS::Entity> entities;
    entities.reserve(COUNT);

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
        registry->emplaceComponent<rs::VelocityComponent>(
            entity, rs::VelocityComponent{0.0f, 0.0f});
        registry->emplaceComponent<rc::ZIndex>(entity, 0);
        entities.push_back(entity);
    }

    double time = measureTime([this, &entities]() {
        for (auto entity : entities) {
            registry->killEntity(entity);
        }
    });

    std::cout << "[PERF] Destroy " << COUNT << " entities: "
              << time << " ms" << std::endl;

    EXPECT_LT(time, 200.0) << "Entity destruction too slow";
}

// =============================================================================
// hasComponent Performance Tests
// =============================================================================

TEST_F(ECSComponentStressTest, HasComponent_10000Checks) {
    constexpr std::size_t COUNT = 1000;
    constexpr std::size_t CHECKS = 10000;
    std::vector<ECS::Entity> entities;
    entities.reserve(COUNT);

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
        if (i % 2 == 0) {
            registry->emplaceComponent<rs::VelocityComponent>(
                entity, rs::VelocityComponent{0.0f, 0.0f});
        }
        entities.push_back(entity);
    }

    std::uniform_int_distribution<std::size_t> dist(0, COUNT - 1);
    std::size_t hasCount = 0;

    double time = measureTime([this, &entities, &dist, &hasCount, CHECKS]() {
        for (std::size_t i = 0; i < CHECKS; ++i) {
            auto entity = entities[dist(rng)];
            if (registry->hasComponent<rs::VelocityComponent>(entity)) {
                ++hasCount;
            }
        }
    });

    std::cout << "[PERF] " << CHECKS << " hasComponent checks: "
              << time << " ms (found " << hasCount << ")" << std::endl;

    EXPECT_LT(time, 50.0) << "hasComponent checks too slow";
}

// =============================================================================
// Component Modification Stress Tests
// =============================================================================

TEST_F(ECSComponentStressTest, ModifyComponents_1000Entities_100Iterations) {
    constexpr std::size_t COUNT = 1000;
    constexpr std::size_t ITERATIONS = 100;

    for (std::size_t i = 0; i < COUNT; ++i) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(entity, 0.0f, 0.0f);
        registry->emplaceComponent<rs::VelocityComponent>(
            entity, rs::VelocityComponent{1.0f, 2.0f});
    }

    double avgTime = measureAverageTime(ITERATIONS, [this]() {
        registry->view<rs::Position, rs::VelocityComponent>().each(
            [](auto entity, auto& pos, auto& vel) {
                pos.x += vel.vx * 0.016f;
                pos.y += vel.vy * 0.016f;
            });
    });

    std::cout << "[PERF] Avg modification time: " << avgTime << " ms"
              << std::endl;

    EXPECT_LT(avgTime, 5.0) << "Component modification too slow";
}

// =============================================================================
// Archetype Fragmentation Tests
// =============================================================================

TEST_F(ECSComponentStressTest, FragmentedArchetypes_ManyComponentCombinations) {
    constexpr std::size_t ENTITIES_PER_TYPE = 100;

    // Create entities with different component combinations
    // This simulates real-world scenarios where entities have varied components

    // Type 1: Position only
    for (std::size_t i = 0; i < ENTITIES_PER_TYPE; ++i) {
        auto e = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(e, 0.0f, 0.0f);
    }

    // Type 2: Position + Velocity
    for (std::size_t i = 0; i < ENTITIES_PER_TYPE; ++i) {
        auto e = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(e, 0.0f, 0.0f);
        registry->emplaceComponent<rs::VelocityComponent>(
            e, rs::VelocityComponent{0.0f, 0.0f});
    }

    // Type 3: Position + ZIndex
    for (std::size_t i = 0; i < ENTITIES_PER_TYPE; ++i) {
        auto e = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(e, 0.0f, 0.0f);
        registry->emplaceComponent<rc::ZIndex>(e, 0);
    }

    // Type 4: Position + Velocity + ZIndex
    for (std::size_t i = 0; i < ENTITIES_PER_TYPE; ++i) {
        auto e = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(e, 0.0f, 0.0f);
        registry->emplaceComponent<rs::VelocityComponent>(
            e, rs::VelocityComponent{0.0f, 0.0f});
        registry->emplaceComponent<rc::ZIndex>(e, 0);
    }

    // Type 5: All components including Image
    for (std::size_t i = 0; i < ENTITIES_PER_TYPE; ++i) {
        auto e = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(e, 0.0f, 0.0f);
        registry->emplaceComponent<rs::VelocityComponent>(
            e, rs::VelocityComponent{0.0f, 0.0f});
        registry->emplaceComponent<rc::ZIndex>(e, 0);
        registry->emplaceComponent<rc::Image>(e, testTexture);
    }

    // Query that spans multiple archetypes
    std::size_t count = 0;
    double time = measureTime([this, &count]() {
        registry->view<rs::Position>().each([&count](auto, auto&) {
            ++count;
        });
    });

    std::cout << "[PERF] Query across fragmented archetypes: "
              << time << " ms (found " << count << ")" << std::endl;

    EXPECT_EQ(count, ENTITIES_PER_TYPE * 5);
    EXPECT_LT(time, 20.0) << "Fragmented query too slow";
}

// =============================================================================
// Concurrent-like Access Pattern Tests
// =============================================================================

TEST_F(ECSComponentStressTest, InterleavedOperations_SimulateGameLoop) {
    constexpr std::size_t FRAMES = 100;
    constexpr std::size_t INITIAL_ENTITIES = 500;
    constexpr std::size_t SPAWN_PER_FRAME = 5;
    constexpr std::size_t KILL_PER_FRAME = 3;

    std::vector<ECS::Entity> entities;

    // Initial spawn
    for (std::size_t i = 0; i < INITIAL_ENTITIES; ++i) {
        auto e = registry->spawnEntity();
        registry->emplaceComponent<rs::Position>(e, 0.0f, 0.0f);
        registry->emplaceComponent<rs::VelocityComponent>(
            e, rs::VelocityComponent{1.0f, 1.0f});
        entities.push_back(e);
    }

    double totalTime = 0.0;

    for (std::size_t frame = 0; frame < FRAMES; ++frame) {
        double frameTime = measureTime([this, &entities,
                                        SPAWN_PER_FRAME, KILL_PER_FRAME]() {
            // 1. Spawn new entities
            for (std::size_t i = 0; i < SPAWN_PER_FRAME; ++i) {
                auto e = registry->spawnEntity();
                registry->emplaceComponent<rs::Position>(e, 0.0f, 0.0f);
                registry->emplaceComponent<rs::VelocityComponent>(
                    e, rs::VelocityComponent{1.0f, 1.0f});
                entities.push_back(e);
            }

            // 2. Update all entities
            registry->view<rs::Position, rs::VelocityComponent>().each(
                [](auto, auto& pos, auto& vel) {
                    pos.x += vel.vx * 0.016f;
                    pos.y += vel.vy * 0.016f;
                });

            // 3. Kill some entities
            for (std::size_t i = 0; i < KILL_PER_FRAME && !entities.empty(); ++i) {
                registry->killEntity(entities.back());
                entities.pop_back();
            }
        });

        totalTime += frameTime;
    }

    double avgFrameTime = totalTime / static_cast<double>(FRAMES);

    std::cout << "[PERF] Simulated game loop (" << FRAMES << " frames):"
              << std::endl;
    std::cout << "       Total: " << totalTime << " ms" << std::endl;
    std::cout << "       Avg frame: " << avgFrameTime << " ms" << std::endl;
    std::cout << "       Final entities: " << entities.size() << std::endl;

    EXPECT_LT(avgFrameTime, 10.0) << "Game loop simulation too slow";
}
