/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** QuadTree Spatial Partitioning PoC - Main test program
*/

#include "../ECS/ECS.hpp"
#include "QuadTree.hpp"
#include "Rect.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>
#include <algorithm>

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

    QuadTree::Rect getRect(const Transform& transform) const {
        return QuadTree::Rect{transform.x, transform.y, width, height};
    }
};

struct EntityData {
    size_t id;
    EntityData(size_t id = 0) : id(id) {}
};

// ============================================================================
// Utility functions
// ============================================================================

void printTestHeader(const std::string& title) {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(70, '=') << "\n";
}

void printTestResult(const std::string& test, bool result, bool expected) {
    std::cout << std::setw(55) << std::left << test << " : ";
    if (result == expected) {
        std::cout << "\033[32m✓ PASS\033[0m";
    } else {
        std::cout << "\033[31m✗ FAIL\033[0m (expected: " << (expected ? "true" : "false")
                  << ", got: " << (result ? "true" : "false") << ")";
    }
    std::cout << "\n";
}

// ============================================================================
// Basic QuadTree tests
// ============================================================================

void testBasicOperations() {
    printTestHeader("Basic QuadTree Operations");

    QuadTree::QuadTree<int> tree(QuadTree::Rect{0.0f, 0.0f, 100.0f, 100.0f}, 4, 5);

    // Test 1: Insert objects
    bool inserted1 = tree.insert(QuadTree::Object<int>{QuadTree::Rect{10.0f, 10.0f, 5.0f, 5.0f}, 1});
    printTestResult("Insert object 1", inserted1, true);

    bool inserted2 = tree.insert(QuadTree::Object<int>{QuadTree::Rect{50.0f, 50.0f, 10.0f, 10.0f}, 2});
    printTestResult("Insert object 2", inserted2, true);

    // Test 2: Insert out of bounds
    bool inserted3 = tree.insert(QuadTree::Object<int>{QuadTree::Rect{-10.0f, -10.0f, 5.0f, 5.0f}, 3});
    printTestResult("Insert out of bounds", inserted3, false);

    // Test 3: Check size
    std::cout << "  Tree contains " << tree.totalSize() << " objects\n";

    // Test 4: Query objects in range
    std::vector<QuadTree::Object<int>> found;
    tree.query(QuadTree::Rect{0.0f, 0.0f, 20.0f, 20.0f}, found);
    printTestResult("Query range contains object 1", found.size() == 1 && found[0].data == 1, true);

    // Test 5: Query all objects
    found.clear();
    tree.queryAll(found);
    printTestResult("Query all returns 2 objects", found.size() == 2, true);

    // Test 6: Clear tree
    tree.clear();
    found.clear();
    tree.queryAll(found);
    printTestResult("Clear removes all objects", found.size() == 0, true);
}

void testSubdivision() {
    printTestHeader("QuadTree Subdivision");

    QuadTree::QuadTree<int> tree(QuadTree::Rect{0.0f, 0.0f, 100.0f, 100.0f}, 2, 5);

    std::cout << "\nInserting objects to trigger subdivision...\n";
    
    // Insert enough objects to trigger subdivision
    tree.insert(QuadTree::Object<int>{QuadTree::Rect{10.0f, 10.0f, 5.0f, 5.0f}, 1});
    std::cout << "  After 1 object: Divided = " << tree.isDivided() << "\n";
    
    tree.insert(QuadTree::Object<int>{QuadTree::Rect{15.0f, 15.0f, 5.0f, 5.0f}, 2});
    std::cout << "  After 2 objects: Divided = " << tree.isDivided() << "\n";
    
    tree.insert(QuadTree::Object<int>{QuadTree::Rect{20.0f, 20.0f, 5.0f, 5.0f}, 3});
    std::cout << "  After 3 objects: Divided = " << tree.isDivided() << "\n";

    printTestResult("Tree subdivided after exceeding capacity", tree.isDivided(), true);
    
    std::cout << "  Total nodes in tree: " << tree.getNodeCount() << "\n";
    std::cout << "  Total objects: " << tree.totalSize() << "\n";
}

void testEdgeCases() {
    printTestHeader("QuadTree Edge Cases");

    QuadTree::QuadTree<int> tree(QuadTree::Rect{0.0f, 0.0f, 100.0f, 100.0f}, 4, 5);

    // Test 1: Object spanning multiple quadrants
    bool inserted = tree.insert(QuadTree::Object<int>{QuadTree::Rect{40.0f, 40.0f, 20.0f, 20.0f}, 1});
    printTestResult("Insert object spanning quadrants", inserted, true);

    // Test 2: Very small object
    inserted = tree.insert(QuadTree::Object<int>{QuadTree::Rect{25.0f, 25.0f, 0.1f, 0.1f}, 2});
    printTestResult("Insert very small object", inserted, true);

    // Test 3: Object at boundary
    inserted = tree.insert(QuadTree::Object<int>{QuadTree::Rect{0.0f, 0.0f, 10.0f, 10.0f}, 3});
    printTestResult("Insert object at boundary", inserted, true);

    // Test 4: Query with no results
    std::vector<QuadTree::Object<int>> found;
    tree.query(QuadTree::Rect{90.0f, 90.0f, 5.0f, 5.0f}, found);
    printTestResult("Query with no results", found.empty(), true);
}

// ============================================================================
// Performance benchmarks
// ============================================================================

void benchmarkInsertion() {
    printTestHeader("Insertion Performance Benchmark");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0.0f, 9000.0f);
    std::uniform_real_distribution<float> sizeDist(5.0f, 50.0f);

    std::vector<size_t> objectCounts = {100, 500, 1000, 5000, 10000};

    for (size_t count : objectCounts) {
        QuadTree::QuadTree<int> tree(QuadTree::Rect{0.0f, 0.0f, 10000.0f, 10000.0f}, 10, 8);

        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < count; ++i) {
            float x = posDist(gen);
            float y = posDist(gen);
            float w = sizeDist(gen);
            float h = sizeDist(gen);
            tree.insert(QuadTree::Object<int>{QuadTree::Rect{x, y, w, h}, static_cast<int>(i)});
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "  " << std::setw(6) << count << " objects: "
                  << std::setw(10) << duration.count() << " µs"
                  << " (" << std::setw(8) << std::fixed << std::setprecision(3)
                  << (duration.count() / static_cast<double>(count)) << " µs/obj)"
                  << " | Nodes: " << tree.getNodeCount()
                  << "\n";
    }
}

void benchmarkQuery() {
    printTestHeader("Query Performance Benchmark");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0.0f, 9000.0f);
    std::uniform_real_distribution<float> sizeDist(5.0f, 50.0f);

    const size_t objectCount = 10000;
    const size_t queryCount = 1000;

    QuadTree::QuadTree<int> tree(QuadTree::Rect{0.0f, 0.0f, 10000.0f, 10000.0f}, 10, 8);

    // Populate tree
    std::cout << "\nPopulating tree with " << objectCount << " objects...\n";
    for (size_t i = 0; i < objectCount; ++i) {
        float x = posDist(gen);
        float y = posDist(gen);
        float w = sizeDist(gen);
        float h = sizeDist(gen);
        tree.insert(QuadTree::Object<int>{QuadTree::Rect{x, y, w, h}, static_cast<int>(i)});
    }

    std::cout << "Tree structure: " << tree.getNodeCount() << " nodes\n";

    // Test different query sizes
    std::vector<float> querySizes = {50.0f, 100.0f, 200.0f, 500.0f, 1000.0f};

    for (float querySize : querySizes) {
        std::vector<QuadTree::Object<int>> found;
        size_t totalFound = 0;

        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < queryCount; ++i) {
            float x = posDist(gen);
            float y = posDist(gen);
            found.clear();
            tree.query(QuadTree::Rect{x, y, querySize, querySize}, found);
            totalFound += found.size();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "  Query size " << std::setw(6) << querySize << "x" << querySize << ": "
                  << std::setw(10) << duration.count() << " µs"
                  << " (" << std::setw(8) << std::fixed << std::setprecision(3)
                  << (duration.count() / static_cast<double>(queryCount)) << " µs/query)"
                  << " | Avg found: " << std::setw(6) << std::fixed << std::setprecision(1)
                  << (totalFound / static_cast<double>(queryCount))
                  << "\n";
    }
}

void benchmarkBruteForce() {
    printTestHeader("Brute Force Collision Detection Benchmark");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0.0f, 9000.0f);
    std::uniform_real_distribution<float> sizeDist(5.0f, 50.0f);

    std::vector<size_t> objectCounts = {100, 500, 1000, 2000, 5000};

    for (size_t count : objectCounts) {
        std::vector<QuadTree::Rect> objects;
        objects.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            float x = posDist(gen);
            float y = posDist(gen);
            float w = sizeDist(gen);
            float h = sizeDist(gen);
            objects.emplace_back(x, y, w, h);
        }

        size_t collisions = 0;
        auto start = std::chrono::high_resolution_clock::now();

        // Brute force: check all pairs
        for (size_t i = 0; i < objects.size(); ++i) {
            for (size_t j = i + 1; j < objects.size(); ++j) {
                if (objects[i].intersects(objects[j])) {
                    collisions++;
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        size_t comparisons = (count * (count - 1)) / 2;

        std::cout << "  " << std::setw(5) << count << " objects: "
                  << std::setw(10) << duration.count() << " µs"
                  << " | Comparisons: " << std::setw(10) << comparisons
                  << " | Collisions: " << std::setw(6) << collisions
                  << "\n";
    }
}

void benchmarkQuadTreeVsBruteForce() {
    printTestHeader("QuadTree vs Brute Force Comparison");

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(0.0f, 9000.0f);
    std::uniform_real_distribution<float> sizeDist(5.0f, 50.0f);

    std::vector<size_t> objectCounts = {100, 500, 1000, 2000, 5000};

    std::cout << "\n" << std::setw(8) << "Objects"
              << " | " << std::setw(15) << "QuadTree (µs)"
              << " | " << std::setw(15) << "Brute Force (µs)"
              << " | " << std::setw(10) << "Speedup"
              << " | " << std::setw(10) << "QT Collis"
              << " | " << std::setw(10) << "BF Collis"
              << "\n";
    std::cout << std::string(95, '-') << "\n";

    for (size_t count : objectCounts) {
        // Prepare objects
        std::vector<QuadTree::Rect> objects;
        objects.reserve(count);

        for (size_t i = 0; i < count; ++i) {
            float x = posDist(gen);
            float y = posDist(gen);
            float w = sizeDist(gen);
            float h = sizeDist(gen);
            objects.emplace_back(x, y, w, h);
        }

        // QuadTree method
        QuadTree::QuadTree<size_t> tree(QuadTree::Rect{0.0f, 0.0f, 10000.0f, 10000.0f}, 10, 8);
        size_t qtCollisions = 0;
        
        auto qtStart = std::chrono::high_resolution_clock::now();
        
        // Insert all objects
        for (size_t i = 0; i < objects.size(); ++i) {
            tree.insert(QuadTree::Object<size_t>{objects[i], i});
        }

        // Query for each object
        for (size_t i = 0; i < objects.size(); ++i) {
            std::vector<QuadTree::Object<size_t>> found;
            tree.query(objects[i], found);
            
            for (const auto& obj : found) {
                if (obj.data != i && objects[i].intersects(obj.bounds)) {
                    qtCollisions++;
                }
            }
        }
        qtCollisions /= 2; // Each collision counted twice
        
        auto qtEnd = std::chrono::high_resolution_clock::now();
        auto qtDuration = std::chrono::duration_cast<std::chrono::microseconds>(qtEnd - qtStart);

        // Brute Force method
        size_t bfCollisions = 0;
        auto bfStart = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < objects.size(); ++i) {
            for (size_t j = i + 1; j < objects.size(); ++j) {
                if (objects[i].intersects(objects[j])) {
                    bfCollisions++;
                }
            }
        }

        auto bfEnd = std::chrono::high_resolution_clock::now();
        auto bfDuration = std::chrono::duration_cast<std::chrono::microseconds>(bfEnd - bfStart);

        double speedup = static_cast<double>(bfDuration.count()) / qtDuration.count();

        std::cout << "  " << std::setw(6) << count
                  << " | " << std::setw(15) << qtDuration.count()
                  << " | " << std::setw(15) << bfDuration.count()
                  << " | " << std::setw(9) << std::fixed << std::setprecision(2) << speedup << "x"
                  << " | " << std::setw(10) << qtCollisions
                  << " | " << std::setw(10) << bfCollisions
                  << "\n";
    }
}

// ============================================================================
// Main function
// ============================================================================

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║        QuadTree Spatial Partitioning - Proof of Concept           ║\n";
    std::cout << "║                    R-Type Project - 2025                           ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";

    try {
        // Basic tests
        testBasicOperations();
        testSubdivision();
        testEdgeCases();

        // Performance benchmarks
        benchmarkInsertion();
        benchmarkQuery();
        benchmarkBruteForce();
        benchmarkQuadTreeVsBruteForce();

        std::cout << "\n";
        std::cout << "╔════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                     All Tests Completed                            ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════════╝\n";
        std::cout << "\n";

    } catch (const std::exception& e) {
        std::cerr << "\n\033[31mError: " << e.what() << "\033[0m\n";
        return 1;
    }

    return 0;
}
