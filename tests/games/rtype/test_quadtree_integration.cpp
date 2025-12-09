/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** test_quadtree_integration - Functional tests for QuadTree collision
** optimization
*/

#include <gtest/gtest.h>

#include <chrono>
#include <random>

#include "../../../lib/rtype_ecs/src/ECS.hpp"
#include "../../../src/games/rtype/shared/Components.hpp"
#include "../../../src/games/rtype/shared/Systems/Collision/AABB.hpp"
#include "../../../src/games/rtype/shared/Systems/Collision/QuadTreeSystem.hpp"

using namespace rtype::games::rtype::shared;
using namespace rtype::games::rtype::shared::collision;

// =============================================================================
// Integration Tests: QuadTree with Collision Detection
// =============================================================================

class QuadTreeCollisionIntegrationTest : public ::testing::Test {
   protected:
    void SetUp() override {
        registry = std::make_unique<ECS::Registry>();
        quadTreeSystem =
            std::make_unique<QuadTreeSystem>(Rect{0, 0, 1920, 1080}, 10, 5);
    }

    void TearDown() override {
        registry.reset();
        quadTreeSystem.reset();
    }

    ECS::Entity createProjectile(float x, float y) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<TransformComponent>(entity, x, y, 0.0F);
        registry->emplaceComponent<BoundingBoxComponent>(entity, 10.0F, 10.0F);
        registry->emplaceComponent<ProjectileTag>(entity);
        return entity;
    }

    ECS::Entity createEnemy(float x, float y) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<TransformComponent>(entity, x, y, 0.0F);
        registry->emplaceComponent<BoundingBoxComponent>(entity, 32.0F, 32.0F);
        registry->emplaceComponent<EnemyTag>(entity);
        return entity;
    }

    ECS::Entity createPlayer(float x, float y) {
        auto entity = registry->spawnEntity();
        registry->emplaceComponent<TransformComponent>(entity, x, y, 0.0F);
        registry->emplaceComponent<BoundingBoxComponent>(entity, 48.0F, 48.0F);
        registry->emplaceComponent<PlayerTag>(entity);
        return entity;
    }

    // Performs actual AABB collision check
    bool checkActualCollision(ECS::Entity a, ECS::Entity b) {
        const auto& transformA = registry->getComponent<TransformComponent>(a);
        const auto& bboxA = registry->getComponent<BoundingBoxComponent>(a);
        const auto& transformB = registry->getComponent<TransformComponent>(b);
        const auto& bboxB = registry->getComponent<BoundingBoxComponent>(b);
        return overlaps(transformA, bboxA, transformB, bboxB);
    }

    std::unique_ptr<ECS::Registry> registry;
    std::unique_ptr<QuadTreeSystem> quadTreeSystem;
};

// =============================================================================
// Functional Tests: Real Game Scenarios
// =============================================================================

TEST_F(QuadTreeCollisionIntegrationTest, ProjectileHitsEnemy) {
    auto projectile = createProjectile(100.0F, 100.0F);
    auto enemy = createEnemy(105.0F, 105.0F);

    quadTreeSystem->update(*registry, 0.016F);
    auto pairs = quadTreeSystem->queryCollisionPairs(*registry);

    // Should detect potential collision
    EXPECT_FALSE(pairs.empty());

    // Verify with actual AABB check
    bool hasCollision = false;
    for (const auto& pair : pairs) {
        if ((pair.entityA.id == projectile.id &&
             pair.entityB.id == enemy.id) ||
            (pair.entityA.id == enemy.id && pair.entityB.id == projectile.id)) {
            hasCollision = checkActualCollision(pair.entityA, pair.entityB);
            break;
        }
    }
    EXPECT_TRUE(hasCollision);
}

TEST_F(QuadTreeCollisionIntegrationTest, ProjectileMissesEnemy) {
    auto projectile = createProjectile(100.0F, 100.0F);
    auto enemy = createEnemy(500.0F, 500.0F);

    quadTreeSystem->update(*registry, 0.016F);
    auto pairs = quadTreeSystem->queryCollisionPairs(*registry);

    // Should not have collision pair or AABB check should fail
    bool hasCollision = false;
    for (const auto& pair : pairs) {
        if ((pair.entityA.id == projectile.id &&
             pair.entityB.id == enemy.id) ||
            (pair.entityA.id == enemy.id && pair.entityB.id == projectile.id)) {
            hasCollision = checkActualCollision(pair.entityA, pair.entityB);
        }
    }
    EXPECT_FALSE(hasCollision);
}

TEST_F(QuadTreeCollisionIntegrationTest, MultipleProjectilesMultipleEnemies) {
    // Create a line of projectiles
    std::vector<ECS::Entity> projectiles;
    for (int i = 0; i < 5; ++i) {
        projectiles.push_back(
            createProjectile(100.0F + i * 200.0F, 540.0F));
    }

    // Create a line of enemies (some will collide)
    std::vector<ECS::Entity> enemies;
    for (int i = 0; i < 5; ++i) {
        enemies.push_back(
            createEnemy(110.0F + i * 200.0F, 545.0F));  // Slightly offset
    }

    quadTreeSystem->update(*registry, 0.016F);
    auto pairs = quadTreeSystem->queryCollisionPairs(*registry);

    // Count actual collisions
    int actualCollisions = 0;
    for (const auto& pair : pairs) {
        if (checkActualCollision(pair.entityA, pair.entityB)) {
            actualCollisions++;
        }
    }

    // All 5 projectile-enemy pairs should collide
    EXPECT_EQ(actualCollisions, 5);
}

TEST_F(QuadTreeCollisionIntegrationTest, PlayerAvoidsEnemies) {
    auto player = createPlayer(960.0F, 540.0F);  // Center of screen

    // Create enemies in a circle around the player (not touching)
    const float radius = 200.0F;
    for (int i = 0; i < 8; ++i) {
        float angle = i * 3.14159F / 4.0F;
        float ex = 960.0F + radius * std::cos(angle);
        float ey = 540.0F + radius * std::sin(angle);
        createEnemy(ex, ey);
    }

    quadTreeSystem->update(*registry, 0.016F);
    auto pairs = quadTreeSystem->queryCollisionPairs(*registry);

    // No collision should occur with player
    bool playerCollision = false;
    for (const auto& pair : pairs) {
        if (pair.entityA.id == player.id || pair.entityB.id == player.id) {
            if (checkActualCollision(pair.entityA, pair.entityB)) {
                playerCollision = true;
                break;
            }
        }
    }
    EXPECT_FALSE(playerCollision);
}

TEST_F(QuadTreeCollisionIntegrationTest, DenseEnemyFormation) {
    // Create a dense 10x10 grid of enemies
    std::vector<ECS::Entity> enemies;
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            enemies.push_back(
                createEnemy(100.0F + i * 40.0F, 100.0F + j * 40.0F));
        }
    }

    // Fire a projectile into the formation
    auto projectile = createProjectile(120.0F, 120.0F);

    quadTreeSystem->update(*registry, 0.016F);
    auto pairs = quadTreeSystem->queryCollisionPairs(*registry);

    // Should find collision with at least one enemy
    bool hitEnemy = false;
    for (const auto& pair : pairs) {
        if (pair.entityA.id == projectile.id ||
            pair.entityB.id == projectile.id) {
            if (checkActualCollision(pair.entityA, pair.entityB)) {
                hitEnemy = true;
                break;
            }
        }
    }
    EXPECT_TRUE(hitEnemy);
}

// =============================================================================
// Performance Comparison Tests
// =============================================================================

TEST_F(QuadTreeCollisionIntegrationTest, PerformanceWithManyEntities) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> xDist(50.0F, 1870.0F);
    std::uniform_real_distribution<float> yDist(50.0F, 1030.0F);

    // Create 200 entities randomly distributed
    for (int i = 0; i < 200; ++i) {
        createEnemy(xDist(gen), yDist(gen));
    }

    // Measure QuadTree approach
    auto start = std::chrono::high_resolution_clock::now();

    quadTreeSystem->update(*registry, 0.016F);
    auto pairs = quadTreeSystem->queryCollisionPairs(*registry);

    // Perform actual collision checks on pairs
    int collisions = 0;
    for (const auto& pair : pairs) {
        if (checkActualCollision(pair.entityA, pair.entityB)) {
            collisions++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto quadTreeTime =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    // QuadTree should complete in reasonable time (< 50ms)
    EXPECT_LT(quadTreeTime.count(), 50000);

    // Log for informational purposes
    std::cout << "QuadTree collision detection with 200 entities: "
              << quadTreeTime.count() << " microseconds, "
              << "pairs checked: " << pairs.size()
              << ", actual collisions: " << collisions << std::endl;
}

TEST_F(QuadTreeCollisionIntegrationTest, QueryNearbyForTargeting) {
    // Scenario: Player wants to find nearby enemies for auto-targeting

    auto player = createPlayer(960.0F, 540.0F);

    // Create enemies at various distances
    createEnemy(1000.0F, 540.0F);   // Close
    createEnemy(1050.0F, 540.0F);   // Medium
    createEnemy(1500.0F, 540.0F);   // Far
    createEnemy(200.0F, 200.0F);    // Very far (diagonal)

    quadTreeSystem->update(*registry, 0.016F);

    // Query for enemies within targeting range
    auto nearby = quadTreeSystem->queryNearby(960.0F, 540.0F, 150.0F);

    // Should find 2 close enemies
    int enemyCount = 0;
    for (const auto& entity : nearby) {
        if (registry->hasComponent<EnemyTag>(entity)) {
            enemyCount++;
        }
    }

    EXPECT_EQ(enemyCount, 2);
}

// =============================================================================
// Edge Cases and Stress Tests
// =============================================================================

TEST_F(QuadTreeCollisionIntegrationTest, EntitiesOnBoundary) {
    // Create entities exactly on world boundaries
    createEnemy(0.0F, 0.0F);
    createEnemy(1920.0F, 0.0F);
    createEnemy(0.0F, 1080.0F);
    createEnemy(1920.0F, 1080.0F);

    quadTreeSystem->update(*registry, 0.016F);

    // These may or may not be inserted depending on bounds handling
    // Just verify no crash
    EXPECT_NO_THROW(quadTreeSystem->queryCollisionPairs(*registry));
}

TEST_F(QuadTreeCollisionIntegrationTest, OverlappingEntitiesAtSamePosition) {
    // Multiple entities at exact same position
    for (int i = 0; i < 5; ++i) {
        createEnemy(500.0F, 500.0F);
    }

    quadTreeSystem->update(*registry, 0.016F);
    auto pairs = quadTreeSystem->queryCollisionPairs(*registry);

    // Should find all pairs (5 choose 2 = 10 pairs)
    // All should be actual collisions
    int actualCollisions = 0;
    for (const auto& pair : pairs) {
        if (checkActualCollision(pair.entityA, pair.entityB)) {
            actualCollisions++;
        }
    }
    EXPECT_EQ(actualCollisions, 10);
}

TEST_F(QuadTreeCollisionIntegrationTest, RapidEntityCreationAndDeletion) {
    // Simulate game scenario with rapid spawning and destruction

    for (int frame = 0; frame < 10; ++frame) {
        // Spawn new enemies
        for (int i = 0; i < 10; ++i) {
            createEnemy(static_cast<float>(frame * 100 + i * 10),
                        static_cast<float>(frame * 50));
        }

        // Update quadtree
        quadTreeSystem->update(*registry, 0.016F);
        auto pairs = quadTreeSystem->queryCollisionPairs(*registry);

        // Should handle each frame without issues
        EXPECT_NO_THROW({
            for (const auto& pair : pairs) {
                checkActualCollision(pair.entityA, pair.entityB);
            }
        });
    }
}

TEST_F(QuadTreeCollisionIntegrationTest, QueryEmptyRegion) {
    // Create entities only in one corner
    for (int i = 0; i < 10; ++i) {
        createEnemy(100.0F + i * 20.0F, 100.0F + i * 20.0F);
    }

    quadTreeSystem->update(*registry, 0.016F);

    // Query opposite corner
    auto nearby = quadTreeSystem->queryNearby(Rect{1500, 800, 200, 200});

    EXPECT_TRUE(nearby.empty());
}

TEST_F(QuadTreeCollisionIntegrationTest, VerifyNoFalseNegatives) {
    // This test ensures QuadTree doesn't miss collisions

    // Create a known collision scenario
    std::vector<std::pair<ECS::Entity, ECS::Entity>> knownCollisions;

    auto e1 = createEnemy(100.0F, 100.0F);
    auto e2 = createEnemy(110.0F, 110.0F);  // Overlaps with e1
    knownCollisions.push_back({e1, e2});

    auto e3 = createEnemy(500.0F, 500.0F);
    auto e4 = createEnemy(505.0F, 505.0F);  // Overlaps with e3
    knownCollisions.push_back({e3, e4});

    quadTreeSystem->update(*registry, 0.016F);
    auto pairs = quadTreeSystem->queryCollisionPairs(*registry);

    // Verify all known collisions are detected
    for (const auto& known : knownCollisions) {
        bool found = false;
        for (const auto& pair : pairs) {
            if ((pair.entityA.id == known.first.id &&
                 pair.entityB.id == known.second.id) ||
                (pair.entityA.id == known.second.id &&
                 pair.entityB.id == known.first.id)) {
                if (checkActualCollision(pair.entityA, pair.entityB)) {
                    found = true;
                    break;
                }
            }
        }
        EXPECT_TRUE(found) << "Known collision was not detected by QuadTree";
    }
}

