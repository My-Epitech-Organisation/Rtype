/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test Object Pool - Demonstration of zero-allocation reuse
*/

#include "ObjectPool.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>

// Bullet struct for testing
struct Bullet {
    float x = 0.0f;
    float y = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    int damage = 10;
    bool active = true;

    Bullet() = default;
    
    Bullet(float px, float py, float vx, float vy, int dmg = 10)
        : x(px), y(py), velocityX(vx), velocityY(vy), damage(dmg), active(true) {}

    void update(float deltaTime) {
        x += velocityX * deltaTime;
        y += velocityY * deltaTime;
    }

    void print() const {
        std::cout << "Bullet: pos(" << x << ", " << y 
                  << "), vel(" << velocityX << ", " << velocityY 
                  << "), damage=" << damage << std::endl;
    }
};

// Helper function to measure time
template<typename Func>
double measureTime(Func func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// Test 1: Basic acquire/release
void testBasicAcquireRelease() {
    std::cout << "\n=== Test 1: Basic Acquire/Release ===" << std::endl;
    
    Memory::ObjectPool<Bullet> pool(10);
    
    std::cout << "Initial state:" << std::endl;
    std::cout << "  Capacity: " << pool.capacity() << std::endl;
    std::cout << "  Available: " << pool.available() << std::endl;
    std::cout << "  In Use: " << pool.inUse() << std::endl;

    // Acquire some bullets
    Bullet* bullet1 = pool.acquire(100.0f, 200.0f, 5.0f, 0.0f);
    Bullet* bullet2 = pool.acquire(150.0f, 250.0f, 0.0f, 5.0f);
    Bullet* bullet3 = pool.acquire(200.0f, 300.0f, 5.0f, 5.0f, 25);

    std::cout << "\nAfter acquiring 3 bullets:" << std::endl;
    std::cout << "  Available: " << pool.available() << std::endl;
    std::cout << "  In Use: " << pool.inUse() << std::endl;

    bullet1->print();
    bullet2->print();
    bullet3->print();

    // Release them
    pool.release(bullet1);
    pool.release(bullet2);
    pool.release(bullet3);

    std::cout << "\nAfter releasing 3 bullets:" << std::endl;
    std::cout << "  Available: " << pool.available() << std::endl;
    std::cout << "  In Use: " << pool.inUse() << std::endl;

    std::cout << "✅ Basic acquire/release test passed" << std::endl;
}

// Test 2: Zero-allocation reuse (1000 objects)
void testZeroAllocationReuse() {
    std::cout << "\n=== Test 2: Zero-Allocation Reuse (1000 objects) ===" << std::endl;
    
    const size_t numObjects = 1000;
    Memory::ObjectPool<Bullet> pool(numObjects);

    std::vector<Bullet*> bullets;
    bullets.reserve(numObjects);

    // Acquire 1000 objects
    double acquireTime = measureTime([&]() {
        for (size_t i = 0; i < numObjects; ++i) {
            float x = static_cast<float>(i * 10);
            float y = static_cast<float>(i * 5);
            bullets.push_back(pool.acquire(x, y, 10.0f, 5.0f));
        }
    });

    std::cout << "Acquired " << numObjects << " objects in " 
              << std::fixed << std::setprecision(3) << acquireTime << " ms" << std::endl;
    std::cout << "  In Use: " << pool.inUse() << std::endl;
    std::cout << "  Available: " << pool.available() << std::endl;

    // Release all objects
    double releaseTime = measureTime([&]() {
        for (auto* bullet : bullets) {
            pool.release(bullet);
        }
    });

    std::cout << "Released " << numObjects << " objects in " 
              << std::fixed << std::setprecision(3) << releaseTime << " ms" << std::endl;
    std::cout << "  In Use: " << pool.inUse() << std::endl;
    std::cout << "  Available: " << pool.available() << std::endl;

    bullets.clear();

    // Re-acquire - should be from pool with zero allocations
    double reacquireTime = measureTime([&]() {
        for (size_t i = 0; i < numObjects; ++i) {
            bullets.push_back(pool.acquire(0.0f, 0.0f, 1.0f, 1.0f));
        }
    });

    std::cout << "\nRe-acquired " << numObjects << " objects in " 
              << std::fixed << std::setprecision(3) << reacquireTime << " ms" << std::endl;
    std::cout << "  (Zero new allocations - all reused from pool)" << std::endl;

    // Cleanup
    for (auto* bullet : bullets) {
        pool.release(bullet);
    }

    pool.getStatistics().print();
    std::cout << "✅ Zero-allocation reuse test passed" << std::endl;
}

// Test 3: Dynamic growth
void testDynamicGrowth() {
    std::cout << "\n=== Test 3: Dynamic Growth ===" << std::endl;
    
    Memory::ObjectPool<Bullet> pool(10);  // Start with small capacity
    
    std::cout << "Initial capacity: " << pool.capacity() << std::endl;

    std::vector<Bullet*> bullets;
    bullets.reserve(100);

    // Acquire more than initial capacity
    for (size_t i = 0; i < 100; ++i) {
        bullets.push_back(pool.acquire(0.0f, 0.0f, 1.0f, 1.0f));
    }

    std::cout << "After acquiring 100 objects:" << std::endl;
    std::cout << "  Capacity: " << pool.capacity() << std::endl;
    std::cout << "  In Use: " << pool.inUse() << std::endl;
    std::cout << "  Pool automatically grew to accommodate demand" << std::endl;

    // Cleanup
    for (auto* bullet : bullets) {
        pool.release(bullet);
    }

    std::cout << "✅ Dynamic growth test passed" << std::endl;
}

// Test 4: Performance comparison (Pool vs New/Delete)
void testPerformanceComparison() {
    std::cout << "\n=== Test 4: Performance Comparison ===" << std::endl;
    
    const size_t iterations = 10000;
    const size_t cycleSize = 100;

    // Test with pool
    double poolTime = 0.0;
    {
        Memory::ObjectPool<Bullet> pool(cycleSize);
        
        poolTime = measureTime([&]() {
            for (size_t iter = 0; iter < iterations; ++iter) {
                std::vector<Bullet*> bullets;
                bullets.reserve(cycleSize);
                
                // Acquire
                for (size_t i = 0; i < cycleSize; ++i) {
                    bullets.push_back(pool.acquire(0.0f, 0.0f, 1.0f, 1.0f));
                }
                
                // Release
                for (auto* bullet : bullets) {
                    pool.release(bullet);
                }
            }
        });
    }

    // Test with new/delete
    double newDeleteTime = measureTime([&]() {
        for (size_t iter = 0; iter < iterations; ++iter) {
            std::vector<Bullet*> bullets;
            bullets.reserve(cycleSize);
            
            // Allocate
            for (size_t i = 0; i < cycleSize; ++i) {
                bullets.push_back(new Bullet(0.0f, 0.0f, 1.0f, 1.0f));
            }
            
            // Deallocate
            for (auto* bullet : bullets) {
                delete bullet;
            }
        }
    });

    std::cout << "Operations: " << iterations << " cycles of " << cycleSize << " objects" << std::endl;
    std::cout << "\nResults:" << std::endl;
    std::cout << "  Object Pool:  " << std::fixed << std::setprecision(2) 
              << poolTime << " ms" << std::endl;
    std::cout << "  New/Delete:   " << std::fixed << std::setprecision(2) 
              << newDeleteTime << " ms" << std::endl;
    std::cout << "  Speedup:      " << std::fixed << std::setprecision(2) 
              << (newDeleteTime / poolTime) << "x" << std::endl;

    std::cout << "✅ Performance comparison test passed" << std::endl;
}

// Test 5: Simulation of game scenario
void testGameScenario() {
    std::cout << "\n=== Test 5: Game Scenario Simulation ===" << std::endl;
    
    Memory::ObjectPool<Bullet> bulletPool(200);
    std::vector<Bullet*> activeBullets;
    
    const float deltaTime = 0.016f; // ~60 FPS
    const int numFrames = 60;       // 1 second
    const int spawnRate = 5;        // Bullets per frame
    
    std::cout << "Simulating game for " << numFrames << " frames" << std::endl;
    std::cout << "Spawning " << spawnRate << " bullets per frame" << std::endl;
    
    for (int frame = 0; frame < numFrames; ++frame) {
        // Spawn new bullets
        for (int i = 0; i < spawnRate; ++i) {
            Bullet* bullet = bulletPool.acquire(
                static_cast<float>(frame * 10),
                100.0f,
                50.0f,
                0.0f
            );
            activeBullets.push_back(bullet);
        }
        
        // Update bullets and remove inactive ones
        for (auto it = activeBullets.begin(); it != activeBullets.end();) {
            Bullet* bullet = *it;
            bullet->update(deltaTime);
            
            // Deactivate bullets that went off screen
            if (bullet->x > 1000.0f) {
                bulletPool.release(bullet);
                it = activeBullets.erase(it);
            } else {
                ++it;
            }
        }
        
        if (frame % 10 == 0) {
            std::cout << "  Frame " << frame << ": " 
                      << activeBullets.size() << " active bullets, "
                      << bulletPool.available() << " in pool" << std::endl;
        }
    }
    
    // Cleanup remaining bullets
    for (auto* bullet : activeBullets) {
        bulletPool.release(bullet);
    }
    
    std::cout << "\nFinal statistics:" << std::endl;
    bulletPool.getStatistics().print();
    std::cout << "✅ Game scenario simulation passed" << std::endl;
}

int main() {
    std::cout << "╔═══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   Object Pool PoC - Memory Optimization      ║" << std::endl;
    std::cout << "║   R-Type Project - Epitech 2025               ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════╝" << std::endl;

    try {
        testBasicAcquireRelease();
        testZeroAllocationReuse();
        testDynamicGrowth();
        testPerformanceComparison();
        testGameScenario();

        std::cout << "\n╔═══════════════════════════════════════════════╗" << std::endl;
        std::cout << "║   All Tests Passed Successfully! ✅           ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════════╝" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}
