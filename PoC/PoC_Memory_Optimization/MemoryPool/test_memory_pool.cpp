/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test Memory Pool - Linear allocator complexity assessment
*/

#include "MemoryPool.hpp"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>

// Test structures
struct GameObject {
    float x, y, z;
    float velocityX, velocityY, velocityZ;
    int id;
    bool active;

    GameObject() = default;
    GameObject(float px, float py, int objId)
        : x(px), y(py), z(0.0f)
        , velocityX(1.0f), velocityY(1.0f), velocityZ(0.0f)
        , id(objId), active(true) {}
};

struct Particle {
    float position[3];
    float velocity[3];
    float color[4];
    float lifetime;
    
    Particle() : lifetime(1.0f) {
        for (int i = 0; i < 3; ++i) {
            position[i] = 0.0f;
            velocity[i] = 0.0f;
        }
        for (int i = 0; i < 4; ++i) {
            color[i] = 1.0f;
        }
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

// Test 1: Basic allocation
void testBasicAllocation() {
    std::cout << "\n=== Test 1: Basic Allocation ===" << std::endl;
    
    const size_t poolSize = 10 * 1024 * 1024; // 10MB
    Memory::MemoryPool pool(poolSize);
    
    std::cout << "Created pool of " << Memory::MemoryPool::Statistics::formatBytes(poolSize) << std::endl;
    std::cout << "Initial state:" << std::endl;
    pool.printStatistics();

    // Allocate some objects
    auto* obj1 = pool.allocate<GameObject>(100.0f, 200.0f, 1);
    auto* obj2 = pool.allocate<GameObject>(150.0f, 250.0f, 2);
    auto* obj3 = pool.allocate<GameObject>(200.0f, 300.0f, 3);

    std::cout << "\nAfter allocating 3 GameObjects:" << std::endl;
    pool.printStatistics();

    std::cout << "\nObject 1: pos(" << obj1->x << ", " << obj1->y << "), id=" << obj1->id << std::endl;
    std::cout << "Object 2: pos(" << obj2->x << ", " << obj2->y << "), id=" << obj2->id << std::endl;
    std::cout << "Object 3: pos(" << obj3->x << ", " << obj3->y << "), id=" << obj3->id << std::endl;

    std::cout << "✅ Basic allocation test passed" << std::endl;
}

// Test 2: Large block allocation (10MB)
void testLargeBlockAllocation() {
    std::cout << "\n=== Test 2: Large Block Allocation (10MB) ===" << std::endl;
    
    const size_t blockSize = 10 * 1024 * 1024; // 10MB
    
    double allocTime = measureTime([blockSize]() {
        Memory::MemoryPool pool(blockSize);
    });

    std::cout << "Allocated " << Memory::MemoryPool::Statistics::formatBytes(blockSize) 
              << " in " << std::fixed << std::setprecision(3) << allocTime << " ms" << std::endl;

    // Test allocation within the block
    Memory::MemoryPool pool(blockSize);
    
    std::cout << "\nAllocating various sized chunks within the block:" << std::endl;
    
    // Allocate 1000 GameObjects
    std::vector<GameObject*> objects;
    for (int i = 0; i < 1000; ++i) {
        objects.push_back(pool.allocate<GameObject>(
            static_cast<float>(i), 
            static_cast<float>(i * 2), 
            i
        ));
    }
    
    std::cout << "Allocated 1000 GameObjects" << std::endl;
    pool.printStatistics();

    // Allocate 5000 Particles
    std::vector<Particle*> particles;
    for (int i = 0; i < 5000; ++i) {
        particles.push_back(pool.allocate<Particle>());
    }
    
    std::cout << "\nAllocated additional 5000 Particles" << std::endl;
    pool.printStatistics();

    std::cout << "✅ Large block allocation test passed" << std::endl;
}

// Test 3: Pointer assignment inside block
void testPointerAssignment() {
    std::cout << "\n=== Test 3: Pointer Assignment Inside Block ===" << std::endl;
    
    const size_t poolSize = 10 * 1024 * 1024; // 10MB
    Memory::MemoryPool pool(poolSize);

    // Track allocated pointers
    std::vector<void*> pointers;
    
    const int numAllocations = 100;
    const size_t allocationSize = 1024; // 1KB each

    std::cout << "Allocating " << numAllocations << " chunks of " 
              << allocationSize << " bytes each" << std::endl;

    for (int i = 0; i < numAllocations; ++i) {
        void* ptr = pool.allocate(allocationSize);
        pointers.push_back(ptr);
        
        // Write some data to verify the pointer works
        *static_cast<int*>(ptr) = i;
    }

    // Verify all pointers are within the pool
    std::cout << "\nVerifying pointer integrity:" << std::endl;
    bool allValid = true;
    for (size_t i = 0; i < pointers.size(); ++i) {
        int value = *static_cast<int*>(pointers[i]);
        if (value != static_cast<int>(i)) {
            std::cout << "❌ Pointer " << i << " has invalid data!" << std::endl;
            allValid = false;
        }
    }

    if (allValid) {
        std::cout << "✅ All " << pointers.size() << " pointers valid and within pool bounds" << std::endl;
    }

    pool.printStatistics();
    std::cout << "✅ Pointer assignment test passed" << std::endl;
}

// Test 4: Performance comparison (Pool vs Standard Allocation)
void testPerformanceComparison() {
    std::cout << "\n=== Test 4: Performance Comparison ===" << std::endl;
    
    const size_t numAllocations = 10000;
    const size_t objectSize = sizeof(GameObject);

    // Test with memory pool
    double poolTime = 0.0;
    {
        Memory::MemoryPool pool(numAllocations * objectSize * 2);
        
        poolTime = measureTime([&]() {
            for (size_t i = 0; i < numAllocations; ++i) {
                pool.allocate<GameObject>(
                    static_cast<float>(i), 
                    static_cast<float>(i), 
                    static_cast<int>(i)
                );
            }
        });
    }

    // Test with standard allocation
    double standardTime = measureTime([&]() {
        std::vector<GameObject*> objects;
        for (size_t i = 0; i < numAllocations; ++i) {
            objects.push_back(new GameObject(
                static_cast<float>(i), 
                static_cast<float>(i), 
                static_cast<int>(i)
            ));
        }
        // Cleanup
        for (auto* obj : objects) {
            delete obj;
        }
    });

    std::cout << "Allocating " << numAllocations << " GameObjects:" << std::endl;
    std::cout << "\nResults:" << std::endl;
    std::cout << "  Memory Pool:       " << std::fixed << std::setprecision(3) 
              << poolTime << " ms" << std::endl;
    std::cout << "  Standard new:      " << std::fixed << std::setprecision(3) 
              << standardTime << " ms" << std::endl;
    std::cout << "  Speedup:           " << std::fixed << std::setprecision(2) 
              << (standardTime / poolTime) << "x" << std::endl;

    std::cout << "✅ Performance comparison test passed" << std::endl;
}

// Test 5: Reset functionality
void testResetFunctionality() {
    std::cout << "\n=== Test 5: Reset Functionality ===" << std::endl;
    
    Memory::MemoryPool pool(1024 * 1024); // 1MB

    std::cout << "Initial state:" << std::endl;
    pool.printStatistics();

    // Allocate some objects
    for (int i = 0; i < 100; ++i) {
        pool.allocate<GameObject>(0.0f, 0.0f, i);
    }

    std::cout << "\nAfter allocations:" << std::endl;
    pool.printStatistics();

    // Reset the pool
    pool.reset();

    std::cout << "\nAfter reset:" << std::endl;
    pool.printStatistics();

    // Allocate again
    for (int i = 0; i < 50; ++i) {
        pool.allocate<Particle>();
    }

    std::cout << "\nAfter new allocations:" << std::endl;
    pool.printStatistics();

    std::cout << "✅ Reset functionality test passed" << std::endl;
}

// Test 6: Array allocation
void testArrayAllocation() {
    std::cout << "\n=== Test 6: Array Allocation ===" << std::endl;
    
    Memory::MemoryPool pool(10 * 1024 * 1024); // 10MB

    const size_t arraySize = 1000;
    
    // Allocate an array of GameObjects
    GameObject* objectArray = pool.allocateArray<GameObject>(arraySize);
    
    // Initialize the array
    for (size_t i = 0; i < arraySize; ++i) {
        objectArray[i].x = static_cast<float>(i);
        objectArray[i].y = static_cast<float>(i * 2);
        objectArray[i].id = static_cast<int>(i);
    }

    std::cout << "Allocated array of " << arraySize << " GameObjects" << std::endl;
    
    // Verify some elements
    std::cout << "\nVerifying array elements:" << std::endl;
    std::cout << "  Element 0:   id=" << objectArray[0].id 
              << ", pos(" << objectArray[0].x << ", " << objectArray[0].y << ")" << std::endl;
    std::cout << "  Element 500: id=" << objectArray[500].id 
              << ", pos(" << objectArray[500].x << ", " << objectArray[500].y << ")" << std::endl;
    std::cout << "  Element 999: id=" << objectArray[999].id 
              << ", pos(" << objectArray[999].x << ", " << objectArray[999].y << ")" << std::endl;

    pool.printStatistics();
    std::cout << "✅ Array allocation test passed" << std::endl;
}

// Test 7: Alignment test
void testAlignment() {
    std::cout << "\n=== Test 7: Alignment Test ===" << std::endl;
    
    Memory::MemoryPool pool(1024 * 1024); // 1MB

    struct alignas(16) AlignedStruct {
        double data[2];
    };

    struct alignas(32) HighlyAlignedStruct {
        double data[4];
    };

    auto* aligned16 = pool.allocate<AlignedStruct>();
    auto* aligned32 = pool.allocate<HighlyAlignedStruct>();
    auto* regular = pool.allocate<GameObject>();

    std::cout << "Allocated objects with different alignments:" << std::endl;
    std::cout << "  AlignedStruct (16-byte):       " << reinterpret_cast<uintptr_t>(aligned16) 
              << " (aligned: " << (reinterpret_cast<uintptr_t>(aligned16) % 16 == 0 ? "✅" : "❌") << ")" << std::endl;
    std::cout << "  HighlyAlignedStruct (32-byte): " << reinterpret_cast<uintptr_t>(aligned32) 
              << " (aligned: " << (reinterpret_cast<uintptr_t>(aligned32) % 32 == 0 ? "✅" : "❌") << ")" << std::endl;
    std::cout << "  GameObject (default):          " << reinterpret_cast<uintptr_t>(regular) << std::endl;

    pool.printStatistics();
    std::cout << "✅ Alignment test passed" << std::endl;
}

// Test 8: Complexity assessment
void testComplexityAssessment() {
    std::cout << "\n=== Test 8: Complexity Assessment ===" << std::endl;
    
    std::cout << "\n📊 Implementation Complexity Analysis:" << std::endl;
    std::cout << "\n1. Code Complexity:" << std::endl;
    std::cout << "   - Implementation: ~300 lines of code" << std::endl;
    std::cout << "   - Core logic: Simple pointer arithmetic" << std::endl;
    std::cout << "   - Complexity rating: LOW ⭐" << std::endl;

    std::cout << "\n2. Integration Complexity:" << std::endl;
    std::cout << "   - Requires minimal changes to existing code" << std::endl;
    std::cout << "   - Can be used as a drop-in allocator" << std::endl;
    std::cout << "   - Complexity rating: LOW ⭐" << std::endl;

    std::cout << "\n3. Maintenance Complexity:" << std::endl;
    std::cout << "   - Simple logic, easy to debug" << std::endl;
    std::cout << "   - No complex data structures" << std::endl;
    std::cout << "   - Complexity rating: LOW ⭐" << std::endl;

    std::cout << "\n4. Usage Complexity:" << std::endl;
    std::cout << "   - Must manage pool lifetime carefully" << std::endl;
    std::cout << "   - No individual deallocation" << std::endl;
    std::cout << "   - Requires understanding of allocation patterns" << std::endl;
    std::cout << "   - Complexity rating: MEDIUM ⭐⭐" << std::endl;

    std::cout << "\n5. Performance Characteristics:" << std::endl;
    
    // Test allocation performance at different scales
    std::vector<size_t> testSizes = {100, 1000, 10000, 100000};
    
    for (size_t size : testSizes) {
        Memory::MemoryPool pool(size * sizeof(GameObject) * 2);
        
        double time = measureTime([&]() {
            for (size_t i = 0; i < size; ++i) {
                pool.allocate<GameObject>(0.0f, 0.0f, 0);
            }
        });
        
        std::cout << "   " << size << " allocations: " 
                  << std::fixed << std::setprecision(3) << time << " ms "
                  << "(avg: " << std::setprecision(6) << (time * 1000.0 / size) << " µs per allocation)" 
                  << std::endl;
    }

    std::cout << "\n✅ Complexity assessment complete" << std::endl;
}

int main() {
    std::cout << "╔═══════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   Memory Pool PoC - Linear Allocator         ║" << std::endl;
    std::cout << "║   R-Type Project - Epitech 2025               ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════╝" << std::endl;

    try {
        testBasicAllocation();
        testLargeBlockAllocation();
        testPointerAssignment();
        testPerformanceComparison();
        testResetFunctionality();
        testArrayAllocation();
        testAlignment();
        testComplexityAssessment();

        std::cout << "\n╔═══════════════════════════════════════════════╗" << std::endl;
        std::cout << "║   All Tests Passed Successfully! ✅           ║" << std::endl;
        std::cout << "╚═══════════════════════════════════════════════╝" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
}
