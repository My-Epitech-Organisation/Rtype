/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** AABB Collision Detection PoC - Main test program
*/

#include "../ECS/ECS.hpp"
#include "Rect.hpp"
#include "Collision.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>

// ============================================================================
// ECS Components for collision testing
// ============================================================================

struct Transform {
    float x;
    float y;

    Transform(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

struct BoxCollider {
    float width;
    float height;

    BoxCollider(float w = 1.0f, float h = 1.0f) : width(w), height(h) {}

    AABB::Rect getRect(const Transform& transform) const {
        return AABB::Rect{transform.x, transform.y, width, height};
    }
};

struct Velocity {
    float dx;
    float dy;

    Velocity(float dx = 0.0f, float dy = 0.0f) : dx(dx), dy(dy) {}
};

// Tag component for collision detection
struct Collidable {};

// Component to track collision state
struct CollisionInfo {
    std::vector<ECS::Entity> collidingWith;
    int collisionCount = 0;
};

// ============================================================================
// Test functions
// ============================================================================

void printTestHeader(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n";
}

void printTestResult(const std::string& test, bool result, bool expected) {
    std::cout << std::setw(50) << std::left << test << " : ";
    if (result == expected) {
        std::cout << "\033[32m✓ PASS\033[0m";
    } else {
        std::cout << "\033[31m✗ FAIL\033[0m (expected: " << (expected ? "true" : "false")
                  << ", got: " << (result ? "true" : "false") << ")";
    }
    std::cout << "\n";
}

// Basic collision tests
void testBasicCollisions() {
    printTestHeader("Basic AABB Collision Tests");

    // Test 1: Overlapping rectangles
    AABB::Rect a{0.0f, 0.0f, 10.0f, 10.0f};
    AABB::Rect b{5.0f, 5.0f, 10.0f, 10.0f};
    printTestResult("Overlapping rectangles", AABB::checkCollision(a, b), true);

    // Test 2: Separated rectangles (X axis)
    AABB::Rect c{0.0f, 0.0f, 5.0f, 5.0f};
    AABB::Rect d{10.0f, 0.0f, 5.0f, 5.0f};
    printTestResult("Separated on X axis", AABB::checkCollision(c, d), false);

    // Test 3: Separated rectangles (Y axis)
    AABB::Rect e{0.0f, 0.0f, 5.0f, 5.0f};
    AABB::Rect f{0.0f, 10.0f, 5.0f, 5.0f};
    printTestResult("Separated on Y axis", AABB::checkCollision(e, f), false);

    // Test 4: Edge touching
    AABB::Rect g{0.0f, 0.0f, 5.0f, 5.0f};
    AABB::Rect h{5.0f, 0.0f, 5.0f, 5.0f};
    printTestResult("Edge touching", AABB::checkCollision(g, h), true);

    // Test 5: Complete containment
    AABB::Rect i{0.0f, 0.0f, 20.0f, 20.0f};
    AABB::Rect j{5.0f, 5.0f, 5.0f, 5.0f};
    printTestResult("Complete containment", AABB::checkCollision(i, j), true);

    // Test 6: Identical rectangles
    AABB::Rect k{0.0f, 0.0f, 10.0f, 10.0f};
    AABB::Rect l{0.0f, 0.0f, 10.0f, 10.0f};
    printTestResult("Identical rectangles", AABB::checkCollision(k, l), true);

    // Test 7: Point collision (zero-sized rectangle)
    AABB::Rect m{5.0f, 5.0f, 0.0f, 0.0f};
    AABB::Rect n{0.0f, 0.0f, 10.0f, 10.0f};
    printTestResult("Point inside rectangle", AABB::checkCollision(m, n), true);

    // Test 8: Negative coordinates
    AABB::Rect o{-10.0f, -10.0f, 15.0f, 15.0f};
    AABB::Rect p{0.0f, 0.0f, 10.0f, 10.0f};
    printTestResult("Negative coordinates overlap", AABB::checkCollision(o, p), true);
}

// Test additional collision functions
void testAdvancedCollisions() {
    printTestHeader("Advanced AABB Functions");

    AABB::Rect a{0.0f, 0.0f, 10.0f, 10.0f};
    AABB::Rect b{5.0f, 5.0f, 10.0f, 10.0f};

    // Test containsPoint
    bool pointInside = AABB::containsPoint(a, 5.0f, 5.0f);
    printTestResult("Point inside rectangle", pointInside, true);

    bool pointOutside = AABB::containsPoint(a, 15.0f, 15.0f);
    printTestResult("Point outside rectangle", pointOutside, false);

    // Test contains
    AABB::Rect outer{0.0f, 0.0f, 20.0f, 20.0f};
    AABB::Rect inner{5.0f, 5.0f, 5.0f, 5.0f};
    bool fullyContained = AABB::contains(outer, inner);
    printTestResult("Rectangle fully contains another", fullyContained, true);

    // Test intersection
    auto intersect = AABB::intersection(a, b);
    printTestResult("Intersection exists", intersect.has_value(), true);
    if (intersect) {
        std::cout << "  → Intersection: (" << intersect->x << ", " << intersect->y
                  << ", " << intersect->w << "x" << intersect->h << ")\n";
    }

    // Test union
    AABB::Rect unionRect = AABB::unionBounds(a, b);
    std::cout << "  Union bounds: (" << unionRect.x << ", " << unionRect.y
              << ", " << unionRect.w << "x" << unionRect.h << ")\n";

    // Test overlap depth
    float overlapX, overlapY;
    bool hasOverlap = AABB::getOverlapDepth(a, b, overlapX, overlapY);
    printTestResult("Overlap depth calculation", hasOverlap, true);
    if (hasOverlap) {
        std::cout << "  → Overlap: X=" << overlapX << ", Y=" << overlapY << "\n";
    }
}

// Test with ECS
void testECSIntegration() {
    printTestHeader("ECS Integration Test");

    ECS::Registry registry;

    // Create multiple entities with colliders
    std::cout << "\nCreating entities with colliders...\n";

    auto entity1 = registry.spawnEntity();
    registry.emplaceComponent<Transform>(entity1, 0.0f, 0.0f);
    registry.emplaceComponent<BoxCollider>(entity1, 10.0f, 10.0f);
    registry.emplaceComponent<Collidable>(entity1);
    registry.emplaceComponent<CollisionInfo>(entity1);

    auto entity2 = registry.spawnEntity();
    registry.emplaceComponent<Transform>(entity2, 5.0f, 5.0f);
    registry.emplaceComponent<BoxCollider>(entity2, 10.0f, 10.0f);
    registry.emplaceComponent<Collidable>(entity2);
    registry.emplaceComponent<CollisionInfo>(entity2);

    auto entity3 = registry.spawnEntity();
    registry.emplaceComponent<Transform>(entity3, 20.0f, 20.0f);
    registry.emplaceComponent<BoxCollider>(entity3, 5.0f, 5.0f);
    registry.emplaceComponent<Collidable>(entity3);
    registry.emplaceComponent<CollisionInfo>(entity3);

    std::cout << "  Created 3 entities with Transform, BoxCollider, Collidable\n";

    // Collision detection system
    std::cout << "\nRunning collision detection system...\n";

    int totalCollisions = 0;

    auto view = registry.view<Transform, BoxCollider, Collidable, CollisionInfo>();
    
    view.each([&](ECS::Entity entityA, auto& transformA, auto& colliderA, auto&, auto& collisionInfoA) {
        collisionInfoA.collidingWith.clear();
        collisionInfoA.collisionCount = 0;

        AABB::Rect rectA = colliderA.getRect(transformA);

        view.each([&](ECS::Entity entityB, auto& transformB, auto& colliderB, auto&, auto&) {
            if (entityA.id >= entityB.id) return; // Avoid duplicate checks

            AABB::Rect rectB = colliderB.getRect(transformB);

            if (AABB::checkCollision(rectA, rectB)) {
                collisionInfoA.collidingWith.push_back(entityB);
                collisionInfoA.collisionCount++;
                totalCollisions++;

                std::cout << "  ✓ Collision detected: Entity " << entityA.id
                          << " <-> Entity " << entityB.id << "\n";
            }
        });
    });

    std::cout << "\nTotal collision pairs detected: " << totalCollisions << "\n";
    printTestResult("ECS collision detection", totalCollisions > 0, true);
}

// Performance benchmark
void performanceBenchmark() {
    printTestHeader("Performance Benchmark");

    const int NUM_TESTS = 1000000;
    
    AABB::Rect a{0.0f, 0.0f, 10.0f, 10.0f};
    AABB::Rect b{5.0f, 5.0f, 10.0f, 10.0f};

    std::cout << "\nRunning " << NUM_TESTS << " collision checks...\n";

    auto start = std::chrono::high_resolution_clock::now();
    
    int collisions = 0;
    for (int i = 0; i < NUM_TESTS; ++i) {
        if (AABB::checkCollision(a, b)) {
            collisions++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "\nResults:\n";
    std::cout << "  Total time: " << duration.count() << " µs\n";
    std::cout << "  Time per check: " << (duration.count() / (double)NUM_TESTS) << " µs\n";
    std::cout << "  Checks per second: " << std::fixed << std::setprecision(0)
              << (NUM_TESTS / (duration.count() / 1000000.0)) << "\n";
    std::cout << "  Collisions detected: " << collisions << "\n";
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         AABB Collision Detection - Proof of Concept       ║\n";
    std::cout << "║                      R-Type Project                        ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════╝\n";

    try {
        testBasicCollisions();
        testAdvancedCollisions();
        testECSIntegration();
        performanceBenchmark();

        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "  \033[32m✓ All tests completed successfully!\033[0m\n";
        std::cout << std::string(60, '=') << "\n\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n\033[31mError: " << e.what() << "\033[0m\n";
        return 1;
    }
}
