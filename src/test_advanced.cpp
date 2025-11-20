/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Advanced ECS Tests - Thread Safety & New Features
*/

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include "ECS/ECS.hpp"

// Test components
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Health { int hp; };
struct Dead {};
struct Frozen {};
struct Player {};

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

// Test 1: ExcludeView functionality
bool test_exclude_view() {
    test_separator("ExcludeView - Query Exclusion");
    
    ECS::Registry registry;
    int tests_passed = 0;
    int tests_total = 0;
    
    // Create entities with different component combinations
    auto e1 = registry.spawn_entity();
    registry.emplace_component<Position>(e1, 1.0f, 1.0f);
    registry.emplace_component<Velocity>(e1, 1.0f, 0.0f);
    
    auto e2 = registry.spawn_entity();
    registry.emplace_component<Position>(e2, 2.0f, 2.0f);
    registry.emplace_component<Velocity>(e2, 1.0f, 0.0f);
    registry.emplace_component<Dead>(e2);
    
    auto e3 = registry.spawn_entity();
    registry.emplace_component<Position>(e3, 3.0f, 3.0f);
    registry.emplace_component<Velocity>(e3, 1.0f, 0.0f);
    registry.emplace_component<Frozen>(e3);
    
    auto e4 = registry.spawn_entity();
    registry.emplace_component<Position>(e4, 4.0f, 4.0f);
    registry.emplace_component<Velocity>(e4, 1.0f, 0.0f);
    registry.emplace_component<Dead>(e4);
    registry.emplace_component<Frozen>(e4);
    
    // Test: Exclude Dead entities
    tests_total++;
    int count_exclude_dead = 0;
    registry.view<Position, Velocity>().exclude<Dead>().each([&](ECS::Entity e, Position& p, Velocity& v) {
        count_exclude_dead++;
        test_result(!registry.has_component<Dead>(e), "Entity should not have Dead component");
    });
    test_result(count_exclude_dead == 2, "Exclude Dead: Should process 2 entities (e1, e3)");
    if (count_exclude_dead == 2) tests_passed++;
    
    // Test: Exclude Dead and Frozen
    tests_total++;
    int count_exclude_both = 0;
    registry.view<Position, Velocity>().exclude<Dead, Frozen>().each([&](ECS::Entity e, Position& p, Velocity& v) {
        count_exclude_both++;
    });
    test_result(count_exclude_both == 1, "Exclude Dead & Frozen: Should process 1 entity (e1 only)");
    if (count_exclude_both == 1) tests_passed++;
    
    // Test: Exclude non-existent component (should return all)
    tests_total++;
    int count_exclude_none = 0;
    registry.view<Position, Velocity>().exclude<Player>().each([&](ECS::Entity e, Position& p, Velocity& v) {
        count_exclude_none++;
    });
    test_result(count_exclude_none == 4, "Exclude non-existent: Should process all 4 entities");
    if (count_exclude_none == 4) tests_passed++;
    
    // Cleanup
    registry.kill_entity(e1);
    registry.kill_entity(e2);
    registry.kill_entity(e3);
    registry.kill_entity(e4);
    
    std::cout << "ExcludeView Tests: " << tests_passed << "/" << tests_total << " passed\n";
    return tests_passed == tests_total;
}

// Test 2: CommandBuffer with placeholder mapping
bool test_command_buffer_mapping() {
    test_separator("CommandBuffer - Placeholder Entity Mapping");
    
    ECS::Registry registry;
    ECS::CommandBuffer cmd(registry);
    int tests_passed = 0;
    int tests_total = 0;
    
    // Test: Spawn entity and add component deferred
    tests_total++;
    auto placeholder = cmd.spawn_entity_deferred();
    cmd.emplace_component_deferred<Position>(placeholder, 10.0f, 20.0f);
    cmd.emplace_component_deferred<Velocity>(placeholder, 5.0f, 5.0f);
    
    test_result(cmd.pending_count() == 3, "CommandBuffer has 3 pending commands");
    if (cmd.pending_count() == 3) tests_passed++;
    
    // Execute commands
    cmd.flush();
    
    // Test: Components should be added to real entity
    tests_total++;
    int entity_count = 0;
    registry.view<Position, Velocity>().each([&](ECS::Entity e, Position& p, Velocity& v) {
        entity_count++;
        test_result(p.x == 10.0f && p.y == 20.0f, "Position component has correct values");
        test_result(v.dx == 5.0f && v.dy == 5.0f, "Velocity component has correct values");
    });
    test_result(entity_count == 1, "One entity created with both components");
    if (entity_count == 1) tests_passed++;
    
    // Test: Multiple deferred spawns
    tests_total++;
    auto p1 = cmd.spawn_entity_deferred();
    auto p2 = cmd.spawn_entity_deferred();
    auto p3 = cmd.spawn_entity_deferred();
    
    cmd.emplace_component_deferred<Position>(p1, 1.0f, 1.0f);
    cmd.emplace_component_deferred<Position>(p2, 2.0f, 2.0f);
    cmd.emplace_component_deferred<Position>(p3, 3.0f, 3.0f);
    
    cmd.flush();
    
    int spawned_count = 0;
    registry.view<Position>().each([&](ECS::Entity e, Position& p) {
        spawned_count++;
    });
    test_result(spawned_count == 4, "Total 4 entities with Position component");
    if (spawned_count == 4) tests_passed++;
    
    std::cout << "CommandBuffer Tests: " << tests_passed << "/" << tests_total << " passed\n";
    return tests_passed == tests_total;
}

// Test 3: Thread-safety of SignalDispatcher
bool test_signal_dispatcher_thread_safety() {
    test_separator("SignalDispatcher - Thread Safety");
    
    ECS::Registry registry;
    std::atomic<int> callback_count{0};
    int tests_passed = 0;
    int tests_total = 0;
    
    // Register callbacks from multiple threads
    const int NUM_THREADS = 10;
    const int CALLBACKS_PER_THREAD = 5;
    
    tests_total++;
    std::vector<std::thread> threads;
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&registry, &callback_count]() {
            for (int i = 0; i < CALLBACKS_PER_THREAD; ++i) {
                registry.on_construct<Position>([&callback_count](ECS::Entity e) {
                    callback_count.fetch_add(1, std::memory_order_relaxed);
                });
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    test_result(true, "Concurrent callback registration completed without crash");
    tests_passed++;
    
    // Trigger callbacks
    tests_total++;
    auto entity = registry.spawn_entity();
    registry.emplace_component<Position>(entity, 0.0f, 0.0f);
    
    int expected_callbacks = NUM_THREADS * CALLBACKS_PER_THREAD;
    test_result(callback_count == expected_callbacks, 
                "All " + std::to_string(expected_callbacks) + " callbacks executed (got " + 
                std::to_string(callback_count.load()) + ")");
    if (callback_count == expected_callbacks) tests_passed++;
    
    registry.kill_entity(entity);
    
    std::cout << "SignalDispatcher Thread Safety Tests: " << tests_passed << "/" << tests_total << " passed\n";
    return tests_passed == tests_total;
}

// Test 4: Concurrent entity operations
bool test_concurrent_entity_operations() {
    test_separator("Concurrent Entity Operations");
    
    ECS::Registry registry;
    int tests_passed = 0;
    int tests_total = 0;
    
    const int ENTITIES_PER_THREAD = 100;
    const int NUM_THREADS = 8;
    
    // Test: Concurrent entity spawning
    tests_total++;
    std::vector<std::thread> spawn_threads;
    std::vector<std::vector<ECS::Entity>> thread_entities(NUM_THREADS);
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        spawn_threads.emplace_back([&registry, &thread_entities, t]() {
            for (int i = 0; i < ENTITIES_PER_THREAD; ++i) {
                auto e = registry.spawn_entity();
                registry.emplace_component<Position>(e, float(t), float(i));
                thread_entities[t].push_back(e);
            }
        });
    }
    
    for (auto& thread : spawn_threads) {
        thread.join();
    }
    
    // Verify all entities exist
    int alive_count = 0;
    for (const auto& entities : thread_entities) {
        for (auto e : entities) {
            if (registry.is_alive(e) && registry.has_component<Position>(e)) {
                alive_count++;
            }
        }
    }
    
    int expected = NUM_THREADS * ENTITIES_PER_THREAD;
    test_result(alive_count == expected, 
                "All " + std::to_string(expected) + " entities spawned and valid (got " + 
                std::to_string(alive_count) + ")");
    if (alive_count == expected) tests_passed++;
    
    // Test: Concurrent component modification
    tests_total++;
    std::vector<std::thread> modify_threads;
    
    for (int t = 0; t < NUM_THREADS; ++t) {
        modify_threads.emplace_back([&registry, &thread_entities, t]() {
            for (auto e : thread_entities[t]) {
                if (registry.is_alive(e)) {
                    registry.patch<Position>(e, [](Position& p) {
                        p.x += 100.0f;
                    });
                }
            }
        });
    }
    
    for (auto& thread : modify_threads) {
        thread.join();
    }
    
    // Verify modifications
    bool all_modified = true;
    for (const auto& entities : thread_entities) {
        for (auto e : entities) {
            if (registry.is_alive(e)) {
                auto& pos = registry.get_component<Position>(e);
                if (pos.x < 100.0f) {
                    all_modified = false;
                    break;
                }
            }
        }
        if (!all_modified) break;
    }
    
    test_result(all_modified, "All entities successfully modified concurrently");
    if (all_modified) tests_passed++;
    
    // Cleanup
    for (const auto& entities : thread_entities) {
        for (auto e : entities) {
            registry.kill_entity(e);
        }
    }
    
    std::cout << "Concurrent Operations Tests: " << tests_passed << "/" << tests_total << " passed\n";
    return tests_passed == tests_total;
}

// Test 5: Edge cases and robustness
bool test_edge_cases() {
    test_separator("Edge Cases & Robustness");
    
    ECS::Registry registry;
    int tests_passed = 0;
    int tests_total = 0;
    
    // Test: Empty view exclusion
    tests_total++;
    int empty_count = 0;
    registry.view<Position>().exclude<Dead>().each([&](ECS::Entity e, Position& p) {
        empty_count++;
    });
    test_result(empty_count == 0, "Empty view with exclusion works correctly");
    if (empty_count == 0) tests_passed++;
    
    // Test: CommandBuffer clear
    tests_total++;
    ECS::CommandBuffer cmd(registry);
    cmd.spawn_entity_deferred();
    cmd.spawn_entity_deferred();
    cmd.spawn_entity_deferred();
    size_t pending_before = cmd.pending_count();
    cmd.clear();
    size_t pending_after = cmd.pending_count();
    test_result(pending_before == 3 && pending_after == 0, 
                "CommandBuffer clear works correctly");
    if (pending_before == 3 && pending_after == 0) tests_passed++;
    
    // Test: Exclude with tag components
    tests_total++;
    auto e1 = registry.spawn_entity();
    auto e2 = registry.spawn_entity();
    registry.emplace_component<Position>(e1, 1.0f, 1.0f);
    registry.emplace_component<Position>(e2, 2.0f, 2.0f);
    registry.emplace_component<Player>(e1);
    
    int non_player_count = 0;
    registry.view<Position>().exclude<Player>().each([&](ECS::Entity e, Position& p) {
        non_player_count++;
    });
    test_result(non_player_count == 1, "Exclude with tag component works correctly");
    if (non_player_count == 1) tests_passed++;
    
    registry.kill_entity(e1);
    registry.kill_entity(e2);
    
    std::cout << "Edge Cases Tests: " << tests_passed << "/" << tests_total << " passed\n";
    return tests_passed == tests_total;
}

int main() {
    std::cout << "=== ECS ADVANCED FEATURES TEST SUITE ===\n";
    
    int total_tests = 0;
    int passed_tests = 0;
    
    if (test_exclude_view()) passed_tests++;
    total_tests++;
    
    if (test_command_buffer_mapping()) passed_tests++;
    total_tests++;
    
    if (test_signal_dispatcher_thread_safety()) passed_tests++;
    total_tests++;
    
    if (test_concurrent_entity_operations()) passed_tests++;
    total_tests++;
    
    if (test_edge_cases()) passed_tests++;
    total_tests++;
    
    test_separator("FINAL RESULTS");
    std::cout << "Test Suites Passed: " << passed_tests << "/" << total_tests << "\n";
    std::cout << "Success Rate: " << (100.0f * passed_tests / total_tests) << "%\n";
    
    if (passed_tests == total_tests) {
        std::cout << "\n🎉 ALL TESTS PASSED! 🎉\n";
        return 0;
    } else {
        std::cout << "\n❌ SOME TESTS FAILED ❌\n";
        return 1;
    }
}
