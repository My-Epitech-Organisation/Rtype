/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Stress Tests - High Load Multi-threading
*/

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <random>
#include "ECS/ECS.hpp"

// Test components
struct Position { float x, y, z; };
struct Velocity { float dx, dy, dz; };
struct Health { int hp; int max_hp; };
struct Name { std::string value; };
struct Tag {};

void test_result(bool passed, const std::string& message) {
    if (passed) {
        std::cout << "✓ PASS: " << message << "\n";
    } else {
        std::cout << "✗ FAIL: " << message << "\n";
    }
}

void test_separator(const std::string& test_name) {
    std::cout << "\n========================================\n";
    std::cout << "TEST: " << test_name << "\n";
    std::cout << "========================================\n";
}

// Test 1: High-volume entity spawning
bool test_high_volume_entity_spawn() {
    test_separator("High Volume Entity Spawning");
    
    ECS::Registry registry;
    const int NUM_THREADS = 16;
    const int ENTITIES_PER_THREAD = 1000;
    
    std::cout << "Spawning " << (NUM_THREADS * ENTITIES_PER_THREAD) << " entities across " 
              << NUM_THREADS << " threads...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    std::vector<std::vector<ECS::Entity>> thread_entities(NUM_THREADS);
    std::atomic<int> spawn_errors{0};
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&registry, &thread_entities, &spawn_errors, t]() {
            try {
                for (int i = 0; i < ENTITIES_PER_THREAD; ++i) {
                    auto e = registry.spawn_entity();
                    registry.emplace_component<Position>(e, float(t), float(i), 0.0f);
                    registry.emplace_component<Velocity>(e, 1.0f, 1.0f, 0.0f);
                    registry.emplace_component<Health>(e, 100, 100);
                    thread_entities[t].push_back(e);
                }
            } catch (...) {
                spawn_errors.fetch_add(1);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    int alive_count = 0;
    for (const auto& entities : thread_entities) {
        for (auto e : entities) {
            if (registry.is_alive(e)) {
                alive_count++;
            }
        }
    }
    
    std::cout << "Time: " << duration.count() << "ms\n";
    std::cout << "Entities/second: " << (alive_count * 1000 / duration.count()) << "\n";
    
    test_result(spawn_errors == 0, "No spawn errors");
    test_result(alive_count == NUM_THREADS * ENTITIES_PER_THREAD, 
                "All entities alive (got " + std::to_string(alive_count) + ")");
    
    // Cleanup
    for (const auto& entities : thread_entities) {
        for (auto e : entities) {
            registry.kill_entity(e);
        }
    }
    
    return spawn_errors == 0 && alive_count == NUM_THREADS * ENTITIES_PER_THREAD;
}

// Test 2: Concurrent read/write stress test
bool test_concurrent_read_write() {
    test_separator("Concurrent Read/Write Stress");
    
    ECS::Registry registry;
    const int NUM_ENTITIES = 5000;
    const int NUM_READER_THREADS = 8;
    const int NUM_WRITER_THREADS = 4;
    const int OPERATIONS_PER_THREAD = 10000;
    
    std::cout << "Creating " << NUM_ENTITIES << " entities...\n";
    
    std::vector<ECS::Entity> entities;
    entities.reserve(NUM_ENTITIES);
    
    for (int i = 0; i < NUM_ENTITIES; ++i) {
        auto e = registry.spawn_entity();
        registry.emplace_component<Position>(e, float(i), float(i), 0.0f);
        registry.emplace_component<Velocity>(e, 1.0f, 0.0f, 0.0f);
        entities.push_back(e);
    }
    
    std::cout << "Running " << NUM_READER_THREADS << " reader threads and " 
              << NUM_WRITER_THREADS << " writer threads...\n";
    
    std::atomic<int> read_errors{0};
    std::atomic<int> write_errors{0};
    std::atomic<long long> total_reads{0};
    std::atomic<long long> total_writes{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    
    // Reader threads
    for (int t = 0; t < NUM_READER_THREADS; ++t) {
        threads.emplace_back([&registry, &entities, &read_errors, &total_reads]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, entities.size() - 1);
            
            try {
                for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                    int idx = dis(gen);
                    auto e = entities[idx];
                    
                    if (registry.is_alive(e) && 
                        registry.has_component<Position>(e) && 
                        registry.has_component<Velocity>(e)) {
                        auto& pos = registry.get_component<Position>(e);
                        auto& vel = registry.get_component<Velocity>(e);
                        // Simulate computation
                        volatile float result = pos.x + vel.dx;
                        (void)result;
                        total_reads.fetch_add(1);
                    }
                }
            } catch (...) {
                read_errors.fetch_add(1);
            }
        });
    }
    
    // Writer threads
    for (int t = 0; t < NUM_WRITER_THREADS; ++t) {
        threads.emplace_back([&registry, &entities, &write_errors, &total_writes]() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, entities.size() - 1);
            
            try {
                for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
                    int idx = dis(gen);
                    auto e = entities[idx];
                    
                    if (registry.is_alive(e)) {
                        registry.patch<Position>(e, [](Position& p) {
                            p.x += 0.1f;
                            p.y += 0.1f;
                        });
                        total_writes.fetch_add(1);
                    }
                }
            } catch (...) {
                write_errors.fetch_add(1);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    long long total_ops = total_reads + total_writes;
    
    std::cout << "Time: " << duration.count() << "ms\n";
    std::cout << "Total operations: " << total_ops << "\n";
    std::cout << "Reads: " << total_reads << ", Writes: " << total_writes << "\n";
    std::cout << "Operations/second: " << (total_ops * 1000 / duration.count()) << "\n";
    
    test_result(read_errors == 0, "No read errors");
    test_result(write_errors == 0, "No write errors");
    
    // Cleanup
    for (auto e : entities) {
        registry.kill_entity(e);
    }
    
    return read_errors == 0 && write_errors == 0;
}

// Test 3: Entity churn (constant spawn/destroy)
bool test_entity_churn() {
    test_separator("Entity Churn Test");
    
    ECS::Registry registry;
    const int NUM_THREADS = 8;
    const int ITERATIONS = 100;
    const int BATCH_SIZE = 100;
    
    std::cout << "Running entity churn test with " << NUM_THREADS << " threads...\n";
    
    std::atomic<int> errors{0};
    std::atomic<long long> spawns{0};
    std::atomic<long long> kills{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&registry, &errors, &spawns, &kills]() {
            try {
                std::vector<ECS::Entity> local_entities;
                
                for (int iter = 0; iter < ITERATIONS; ++iter) {
                    // Spawn batch
                    for (int i = 0; i < BATCH_SIZE; ++i) {
                        auto e = registry.spawn_entity();
                        registry.emplace_component<Position>(e, 0.0f, 0.0f, 0.0f);
                        local_entities.push_back(e);
                        spawns.fetch_add(1);
                    }
                    
                    // Kill half
                    for (size_t i = 0; i < local_entities.size() / 2; ++i) {
                        registry.kill_entity(local_entities[i]);
                        kills.fetch_add(1);
                    }
                    
                    // Remove dead from list
                    local_entities.erase(local_entities.begin(), 
                                        local_entities.begin() + local_entities.size() / 2);
                }
                
                // Cleanup remaining
                for (auto e : local_entities) {
                    registry.kill_entity(e);
                    kills.fetch_add(1);
                }
            } catch (...) {
                errors.fetch_add(1);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Time: " << duration.count() << "ms\n";
    std::cout << "Total spawns: " << spawns << "\n";
    std::cout << "Total kills: " << kills << "\n";
    std::cout << "Operations/second: " << ((spawns + kills) * 1000 / duration.count()) << "\n";
    
    // Periodic cleanup
    size_t cleaned = registry.cleanup_tombstones();
    std::cout << "Tombstones cleaned: " << cleaned << "\n";
    
    test_result(errors == 0, "No errors during churn test");
    
    return errors == 0;
}

// Test 4: Component pool stress
bool test_component_pool_stress() {
    test_separator("Component Pool Stress");
    
    ECS::Registry registry;
    const int NUM_THREADS = 12;
    const int ENTITIES_PER_THREAD = 500;
    
    std::cout << "Testing component pool with multiple component types...\n";
    
    std::atomic<int> errors{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    std::vector<std::vector<ECS::Entity>> thread_entities(NUM_THREADS);
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&registry, &thread_entities, &errors, t]() {
            try {
                for (int i = 0; i < ENTITIES_PER_THREAD; ++i) {
                    auto e = registry.spawn_entity();
                    
                    // Add different component combinations
                    if (i % 4 == 0) {
                        registry.emplace_component<Position>(e, 1.0f, 2.0f, 3.0f);
                        registry.emplace_component<Velocity>(e, 0.1f, 0.2f, 0.3f);
                        registry.emplace_component<Health>(e, 100, 100);
                        registry.emplace_component<Tag>(e);
                    } else if (i % 4 == 1) {
                        registry.emplace_component<Position>(e, 1.0f, 2.0f, 3.0f);
                        registry.emplace_component<Health>(e, 50, 100);
                    } else if (i % 4 == 2) {
                        registry.emplace_component<Velocity>(e, 0.5f, 0.6f, 0.7f);
                        registry.emplace_component<Tag>(e);
                    } else {
                        registry.emplace_component<Position>(e, 1.0f, 2.0f, 3.0f);
                        registry.emplace_component<Velocity>(e, 0.1f, 0.2f, 0.3f);
                    }
                    
                    thread_entities[t].push_back(e);
                }
            } catch (...) {
                errors.fetch_add(1);
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Verify component counts
    size_t pos_count = registry.count_components<Position>();
    size_t vel_count = registry.count_components<Velocity>();
    size_t health_count = registry.count_components<Health>();
    size_t tag_count = registry.count_components<Tag>();
    
    std::cout << "Time: " << duration.count() << "ms\n";
    std::cout << "Position components: " << pos_count << "\n";
    std::cout << "Velocity components: " << vel_count << "\n";
    std::cout << "Health components: " << health_count << "\n";
    std::cout << "Tag components: " << tag_count << "\n";
    
    test_result(errors == 0, "No errors");
    test_result(pos_count > 0, "Position components created");
    test_result(vel_count > 0, "Velocity components created");
    
    // Cleanup
    for (const auto& entities : thread_entities) {
        for (auto e : entities) {
            registry.kill_entity(e);
        }
    }
    
    return errors == 0;
}

int main() {
    std::cout << "\n";
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║   ECS STRESS TEST SUITE (MT HEAVY)    ║\n";
    std::cout << "╚════════════════════════════════════════╝\n";
    std::cout << "\n";
    std::cout << "Hardware threads available: " << std::thread::hardware_concurrency() << "\n";
    
    int passed = 0;
    int total = 4;
    
    if (test_high_volume_entity_spawn()) passed++;
    if (test_concurrent_read_write()) passed++;
    if (test_entity_churn()) passed++;
    if (test_component_pool_stress()) passed++;
    
    test_separator("FINAL RESULTS");
    std::cout << "Tests Passed: " << passed << "/" << total << "\n";
    std::cout << "Success Rate: " << (passed * 100 / total) << "%\n";
    
    if (passed == total) {
        std::cout << "\n🎉 ALL STRESS TESTS PASSED! 🎉\n";
        return 0;
    } else {
        std::cout << "\n❌ SOME TESTS FAILED ❌\n";
        return 1;
    }
}
