/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_integration
*/

#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>

#include "../../src/common/SafeQueue/SafeQueue.hpp"
#include "../../src/common/Types.hpp"
#include "../../src/games/rtype/shared/Components.hpp"
#include "../../src/games/rtype/shared/Systems/Movements/MovementSystem.hpp"
#include "../../src/engine/ecs/ECS.hpp"

using namespace rtype;
using namespace rtype::games::rtype::shared;

// ============================================================================
// Integration Tests - Testing component interactions
// ============================================================================

TEST(IntegrationTest, MovementSystemWithNetworkComponents) {
    // Test that movement system works with network-synced components
    ECS::Registry registry;
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f, 0.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 10.0f, 5.0f);
    registry.emplaceComponent<NetworkIdComponent>(entity, 12345);

    MovementSystem movementSystem;
    const float deltaTime = 0.016f;  // ~60 FPS

    // Update movement
    movementSystem.update(registry, deltaTime);

    // Verify movement occurred
    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_NEAR(transform.x, 0.16f, 0.001f);
    EXPECT_NEAR(transform.y, 0.08f, 0.001f);

    // Network ID should remain unchanged
    auto& netId = registry.getComponent<NetworkIdComponent>(entity);
    EXPECT_EQ(netId.networkId, 12345u);

    registry.killEntity(entity);
}

TEST(IntegrationTest, SafeQueueThreadSafety) {
    SafeQueue<int> queue;

    // Test basic operations
    queue.push(1);
    queue.push(2);
    queue.push(3);

    EXPECT_EQ(queue.size(), 3);

    auto item1 = queue.pop();
    ASSERT_TRUE(item1.has_value());
    EXPECT_EQ(*item1, 1);

    auto item2 = queue.pop();
    ASSERT_TRUE(item2.has_value());
    EXPECT_EQ(*item2, 2);

    EXPECT_EQ(queue.size(), 1);
}

TEST(IntegrationTest, ComponentStateSynchronization) {
    // Test that components maintain state correctly for network sync
    ECS::Registry registry;
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<TransformComponent>(entity, 100.0f, 200.0f, 90.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 15.0f, -10.0f);
    registry.emplaceComponent<NetworkIdComponent>(entity, 999);

    MovementSystem movementSystem;
    // Simulate multiple movement updates
    const float deltaTime = 0.1f;
    for (int i = 0; i < 10; ++i) {
        movementSystem.update(registry, deltaTime);
    }

    // Verify final position
    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_NEAR(transform.x, 100.0f + 15.0f * 10 * deltaTime, 0.001f);
    EXPECT_NEAR(transform.y, 200.0f + (-10.0f) * 10 * deltaTime, 0.001f);

    // Components should maintain their relationships
    auto& netId = registry.getComponent<NetworkIdComponent>(entity);
    EXPECT_EQ(netId.networkId, 999u);
    auto& velocity = registry.getComponent<VelocityComponent>(entity);
    EXPECT_FLOAT_EQ(velocity.vx, 15.0f);
    EXPECT_FLOAT_EQ(velocity.vy, -10.0f);

    registry.killEntity(entity);
}

// ============================================================================
// Performance Tests
// ============================================================================

TEST(PerformanceTest, MovementSystem_HighFrequency) {
    ECS::Registry registry;
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<TransformComponent>(entity, 0.0f, 0.0f, 0.0f);
    registry.emplaceComponent<VelocityComponent>(entity, 1.0f, 1.0f);

    MovementSystem movementSystem;
    const int iterations = 10000;
    const float deltaTime = 1.0f / 60.0f;  // 60 FPS

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; ++i) {
        movementSystem.update(registry, deltaTime);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should complete in reasonable time (less than 100ms for 10k iterations)
    EXPECT_LT(duration.count(), 100);

    // Verify final position is correct (allow for floating point precision)
    auto& transform = registry.getComponent<TransformComponent>(entity);
    EXPECT_NEAR(transform.x, iterations * deltaTime, 0.01f);
    EXPECT_NEAR(transform.y, iterations * deltaTime, 0.01f);

    registry.killEntity(entity);
}