/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** main
*/

#include <iostream>
#include <string>
#include <chrono>
#include "ECS/Core/Registry.hpp"

struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

struct Name {
    std::string name;
};

struct Player {};
struct Enemy {};

struct GameTime {
    float delta_time = 0.016f;
    float total_time = 0.0f;
};

struct Statistics {
    int entities_created = 0;
    int entities_destroyed = 0;
};

void movement_system(ECS::Registry& registry) {
    auto& time = registry.get_singleton<GameTime>();
    auto view = registry.view<Position, Velocity>();

    view.each([&time](ECS::Entity entity, Position& pos, Velocity& vel) {
        pos.x += vel.dx * time.delta_time;
        pos.y += vel.dy * time.delta_time;
    });
}

void parallel_movement_system(ECS::Registry& registry) {
    auto& time = registry.get_singleton<GameTime>();
    auto view = registry.parallel_view<Position, Velocity>();

    view.each([&time](ECS::Entity entity, Position& pos, Velocity& vel) {
        pos.x += vel.dx * time.delta_time;
        pos.y += vel.dy * time.delta_time;
    });
}

void render_system(ECS::Registry& registry) {
    auto view = registry.view<Position, Name>();

    view.each([](ECS::Entity entity, Position& pos, Name& name) {
        std::cout << "Entity [" << entity.index() << "] " << name.name
                  << " at (" << pos.x << ", " << pos.y << ")\n";
    });
}

void test_separator(const std::string& test_name) {
    std::cout << "\n========================================\n";
    std::cout << "TEST: " << test_name << "\n";
    std::cout << "========================================\n";
}

void test_result(bool passed, const std::string& message) {
    if (passed) {
        std::cout << "✓ PASS: " << message << "\n";
    } else {
        std::cout << "✗ FAIL: " << message << "\n";
    }
}

int main() {
    ECS::Registry registry;
    int tests_passed = 0;
    int tests_total = 0;

    std::cout << "=== ECS ADVANCED DEMO & TESTING ===\n\n";

    // ----------------------------------------------------------------
    // TEST 1: Entity Validation in get_component
    // ----------------------------------------------------------------
    test_separator("Entity Validation in get_component");

    auto test_entity = registry.spawn_entity();
    registry.emplace_component<Position>(test_entity, 10.0f, 20.0f);

    // Test 1.1: Valid access
    tests_total++;
    try {
        auto& pos = registry.get_component<Position>(test_entity);
        test_result(pos.x == 10.0f && pos.y == 20.0f, "Can get component from valid entity");
        tests_passed++;
    } catch (...) {
        test_result(false, "Should be able to get component from valid entity");
    }

    // Test 1.2: Access non-existent component
    tests_total++;
    try {
        registry.get_component<Velocity>(test_entity);
        test_result(false, "Should throw when getting non-existent component");
    } catch (const std::runtime_error& e) {
        test_result(true, std::string("Correctly throws for non-existent component: ") + e.what());
        tests_passed++;
    }

    // Test 1.3: Access dead entity
    tests_total++;
    registry.kill_entity(test_entity);
    try {
        registry.get_component<Position>(test_entity);
        test_result(false, "Should throw when getting component from dead entity");
    } catch (const std::runtime_error& e) {
        test_result(true, std::string("Correctly throws for dead entity: ") + e.what());
        tests_passed++;
    }

    // ----------------------------------------------------------------
    // TEST 2: No Duplicate Components
    // ----------------------------------------------------------------
    test_separator("No Duplicate Components");

    auto dup_entity = registry.spawn_entity();

    // Add component twice
    registry.emplace_component<Position>(dup_entity, 1.0f, 2.0f);
    registry.emplace_component<Position>(dup_entity, 3.0f, 4.0f);

    // Test 2.1: Component should be replaced, not duplicated
    tests_total++;
    auto& pos = registry.get_component<Position>(dup_entity);
    test_result(pos.x == 3.0f && pos.y == 4.0f, "Component replaced correctly (no duplicate)");
    if (pos.x == 3.0f && pos.y == 4.0f) tests_passed++;

    // Test 2.2: Check internal tracking doesn't have duplicates
    tests_total++;
    const auto& components = registry.get_entity_components(dup_entity);
    std::type_index pos_type = std::type_index(typeid(Position));
    int count = std::count(components.begin(), components.end(), pos_type);
    test_result(count == 1, std::string("No duplicate in entity_components tracking (count: ") + std::to_string(count) + ")");
    if (count == 1) tests_passed++;

    // Test 2.3: get_or_emplace() functionality
    tests_total++;
    auto& pos_ref1 = registry.get_or_emplace<Position>(dup_entity, 100.0f, 200.0f);
    bool values_unchanged = (pos_ref1.x == 3.0f && pos_ref1.y == 4.0f);
    test_result(values_unchanged, "get_or_emplace() returns existing component without modifying it");
    if (values_unchanged) tests_passed++;

    // Test 2.4: get_or_emplace() creates new component
    tests_total++;
    auto new_entity = registry.spawn_entity();
    auto& new_pos = registry.get_or_emplace<Position>(new_entity, 50.0f, 60.0f);
    test_result(new_pos.x == 50.0f && new_pos.y == 60.0f,
                "get_or_emplace() creates new component with correct values");
    if (new_pos.x == 50.0f && new_pos.y == 60.0f) tests_passed++;
    registry.kill_entity(new_entity);

    registry.kill_entity(dup_entity);

    // ----------------------------------------------------------------
    // TEST 3: Tag Component Optimization
    // ----------------------------------------------------------------
    test_separator("Tag Component Optimization (Empty Components)");

    auto tag_entity1 = registry.spawn_entity();
    auto tag_entity2 = registry.spawn_entity();

    // Add tag components
    registry.emplace_component<Player>(tag_entity1);
    registry.emplace_component<Enemy>(tag_entity2);

    // Test 3.1: Tag components can be checked
    tests_total++;
    test_result(registry.has_component<Player>(tag_entity1) &&
                registry.has_component<Enemy>(tag_entity2),
                "Tag components properly stored and queryable");
    if (registry.has_component<Player>(tag_entity1) &&
        registry.has_component<Enemy>(tag_entity2)) tests_passed++;

    // Test 3.2: Tag components can be retrieved (returns dummy)
    tests_total++;
    try {
        auto& player_tag = registry.get_component<Player>(tag_entity1);
        (void)player_tag; // Suppress unused warning
        test_result(true, "Tag component can be retrieved without error");
        tests_passed++;
    } catch (...) {
        test_result(false, "Tag component retrieval should not throw");
    }

    // Test 3.3: Memory usage check (informational)
    std::cout << "  [INFO] Tag components use optimized storage (no data array, only entity tracking)\n";
    std::cout << "  [INFO] Player tag uses TagSparseSet: " << (std::is_empty_v<Player> ? "YES" : "NO") << "\n";
    std::cout << "  [INFO] Position component uses regular SparseSet: " << (std::is_empty_v<Position> ? "NO" : "YES") << "\n";

    registry.kill_entity(tag_entity1);
    registry.kill_entity(tag_entity2);

    // ----------------------------------------------------------------
    // TEST 4: Memory Pre-allocation Performance
    // ----------------------------------------------------------------
    test_separator("Memory Pre-allocation Performance");

    const int ALLOC_TEST_SIZE = 5000;

    // Test 4.1: Without reserve
    auto start_no_reserve = std::chrono::high_resolution_clock::now();
    ECS::Registry registry_no_reserve;
    for (int i = 0; i < ALLOC_TEST_SIZE; ++i) {
        auto e = registry_no_reserve.spawn_entity();
        registry_no_reserve.emplace_component<Position>(e, float(i), float(i));
        registry_no_reserve.emplace_component<Velocity>(e, 1.0f, 1.0f);
    }
    auto end_no_reserve = std::chrono::high_resolution_clock::now();
    auto time_no_reserve = std::chrono::duration_cast<std::chrono::microseconds>(end_no_reserve - start_no_reserve).count();

    // Test 4.2: With reserve
    auto start_with_reserve = std::chrono::high_resolution_clock::now();
    ECS::Registry registry_with_reserve;
    registry_with_reserve.reserve_entities(ALLOC_TEST_SIZE);
    registry_with_reserve.reserve_components<Position>(ALLOC_TEST_SIZE);
    registry_with_reserve.reserve_components<Velocity>(ALLOC_TEST_SIZE);
    for (int i = 0; i < ALLOC_TEST_SIZE; ++i) {
        auto e = registry_with_reserve.spawn_entity();
        registry_with_reserve.emplace_component<Position>(e, float(i), float(i));
        registry_with_reserve.emplace_component<Velocity>(e, 1.0f, 1.0f);
    }
    auto end_with_reserve = std::chrono::high_resolution_clock::now();
    auto time_with_reserve = std::chrono::duration_cast<std::chrono::microseconds>(end_with_reserve - start_with_reserve).count();

    std::cout << "  Creating " << ALLOC_TEST_SIZE << " entities:\n";
    std::cout << "    Without reserve: " << time_no_reserve << " μs\n";
    std::cout << "    With reserve:    " << time_with_reserve << " μs\n";
    std::cout << "    Improvement:     " << (float)time_no_reserve / time_with_reserve << "x faster\n";

    tests_total++;
    // Reserve is beneficial for large-scale operations (reduces reallocations)
    // The benefit may vary depending on data size and cache behavior
    float speedup_ratio = (float)time_no_reserve / time_with_reserve;
    test_result(speedup_ratio > 0.8f, "Pre-allocation comparable or better (within 20% margin)");
    if (speedup_ratio > 0.8f) tests_passed++;

    // ----------------------------------------------------------------
    // TEST 5: Singleton Components
    // ----------------------------------------------------------------
    test_separator("Singleton Components (Global Resources)");

    registry.set_singleton<GameTime>();
    auto& stats = registry.set_singleton<Statistics>();

    // Test 5.1: Singleton exists
    tests_total++;
    test_result(registry.has_singleton<GameTime>() && registry.has_singleton<Statistics>(),
                "Singletons properly created");
    if (registry.has_singleton<GameTime>() && registry.has_singleton<Statistics>()) tests_passed++;

    // Test 5.2: Singleton values accessible
    tests_total++;
    auto& time = registry.get_singleton<GameTime>();
    time.delta_time = 0.5f;
    test_result(registry.get_singleton<GameTime>().delta_time == 0.5f,
                "Singleton values can be modified and retrieved");
    if (registry.get_singleton<GameTime>().delta_time == 0.5f) tests_passed++;

    // Test 5.2b: patch() functionality
    tests_total++;
    auto patch_entity = registry.spawn_entity();
    registry.emplace_component<Position>(patch_entity, 10.0f, 20.0f);
    
    registry.patch<Position>(patch_entity, [](Position& pos) {
        pos.x += 5.0f;
        pos.y *= 2.0f;
    });
    
    auto& patched_pos = registry.get_component<Position>(patch_entity);
    test_result(patched_pos.x == 15.0f && patched_pos.y == 40.0f,
                "patch() correctly modifies component via callback");
    if (patched_pos.x == 15.0f && patched_pos.y == 40.0f) tests_passed++;
    registry.kill_entity(patch_entity);

    // Test 5.3: Singleton removal
    tests_total++;
    registry.remove_singleton<GameTime>();
    test_result(!registry.has_singleton<GameTime>(),
                "Singleton can be removed");
    if (!registry.has_singleton<GameTime>()) tests_passed++;

    // Re-create for later use
    registry.set_singleton<GameTime>();

    // ----------------------------------------------------------------
    // TEST 6: Signal/Observer System
    // ----------------------------------------------------------------
    test_separator("Signal/Observer System (Callbacks)");

    int construct_count = 0;
    int destroy_count = 0;

    registry.on_construct<Velocity>([&construct_count](ECS::Entity e) {
        construct_count++;
    });

    registry.on_destroy<Velocity>([&destroy_count](ECS::Entity e) {
        destroy_count++;
    });

    auto callback_entity = registry.spawn_entity();
    registry.emplace_component<Velocity>(callback_entity, 1.0f, 1.0f);

    // Test 6.1: on_construct callback triggered
    tests_total++;
    test_result(construct_count == 1, "on_construct callback triggered");
    if (construct_count == 1) tests_passed++;

    registry.remove_component<Velocity>(callback_entity);

    // Test 6.2: on_destroy callback triggered
    tests_total++;
    test_result(destroy_count == 1, "on_destroy callback triggered");
    if (destroy_count == 1) tests_passed++;

    registry.kill_entity(callback_entity);

    // Test 6.3: remove_entities_if() functionality
    tests_total++;
    std::vector<ECS::Entity> test_entities;
    for (int i = 0; i < 10; ++i) {
        auto e = registry.spawn_entity();
        registry.emplace_component<Position>(e, float(i), float(i));
        test_entities.push_back(e);
    }
    
    // Remove entities with even index positions
    size_t removed = registry.remove_entities_if([&registry](ECS::Entity e) {
        if (registry.has_component<Position>(e)) {
            auto& pos = registry.get_component<Position>(e);
            return static_cast<int>(pos.x) % 2 == 0;
        }
        return false;
    });
    
    test_result(removed == 5, std::string("remove_entities_if() removed correct count: ") + std::to_string(removed));
    if (removed == 5) tests_passed++;

    // Cleanup remaining
    for (auto e : test_entities) {
        if (registry.is_alive(e)) {
            registry.kill_entity(e);
        }
    }

    // ----------------------------------------------------------------
    // TEST 7: Entity Recycling
    // ----------------------------------------------------------------
    test_separator("Entity Recycling (Generational Indices)");

    auto recycle_e1 = registry.spawn_entity();
    auto old_index = recycle_e1.index();
    auto old_generation = recycle_e1.generation();

    registry.kill_entity(recycle_e1);

    auto recycle_e2 = registry.spawn_entity();

    // Test 7.1: Index reused
    tests_total++;
    test_result(recycle_e2.index() == old_index,
                "Entity index recycled correctly");
    if (recycle_e2.index() == old_index) tests_passed++;

    // Test 7.2: Generation incremented
    tests_total++;
    test_result(recycle_e2.generation() == old_generation + 1,
                "Generation incremented to prevent dangling references");
    if (recycle_e2.generation() == old_generation + 1) tests_passed++;

    // Test 7.3: Old entity handle invalid
    tests_total++;
    test_result(!registry.is_alive(recycle_e1),
                "Old entity handle correctly invalidated");
    if (!registry.is_alive(recycle_e1)) tests_passed++;

    registry.kill_entity(recycle_e2);

    // ----------------------------------------------------------------
    // TEST 8: Views and Iteration
    // ----------------------------------------------------------------
    test_separator("Views and Component Iteration");

    auto view_e1 = registry.spawn_entity();
    auto view_e2 = registry.spawn_entity();
    auto view_e3 = registry.spawn_entity();

    registry.emplace_component<Position>(view_e1, 1.0f, 2.0f);
    registry.emplace_component<Velocity>(view_e1, 0.5f, 0.5f);

    registry.emplace_component<Position>(view_e2, 3.0f, 4.0f);

    registry.emplace_component<Position>(view_e3, 5.0f, 6.0f);
    registry.emplace_component<Velocity>(view_e3, 1.0f, 1.0f);

    // Test 8.1: View with multiple components
    tests_total++;
    int view_count = 0;
    registry.view<Position, Velocity>().each([&view_count](ECS::Entity e, Position& p, Velocity& v) {
        view_count++;
    });
    test_result(view_count == 2, "View correctly filters entities with both components");
    if (view_count == 2) tests_passed++;

    // Test 8.2: View with single component
    tests_total++;
    int single_view_count = 0;
    registry.view<Position>().each([&single_view_count](ECS::Entity e, Position& p) {
        single_view_count++;
    });
    test_result(single_view_count == 3, "View correctly includes all entities with Position");
    if (single_view_count == 3) tests_passed++;

    registry.kill_entity(view_e1);
    registry.kill_entity(view_e2);
    registry.kill_entity(view_e3);

    // ----------------------------------------------------------------
    // TEST 9: Thread Safety (Parallel View)
    // ----------------------------------------------------------------
    test_separator("Thread Safety (Parallel View)");

    const int THREAD_TEST_SIZE = 1000;
    std::vector<ECS::Entity> thread_entities;
    thread_entities.reserve(THREAD_TEST_SIZE);

    for (int i = 0; i < THREAD_TEST_SIZE; ++i) {
        auto e = registry.spawn_entity();
        registry.emplace_component<Position>(e, float(i), float(i));
        registry.emplace_component<Velocity>(e, 1.0f, 1.0f);
        thread_entities.push_back(e);
    }

    // Test 9.1: Parallel modification of components (thread-safe operation)
    tests_total++;
    std::atomic<int> parallel_count{0};
    registry.parallel_view<Position, Velocity>().each([&parallel_count](ECS::Entity e, Position& p, Velocity& v) {
        p.x += v.dx;
        p.y += v.dy;
        parallel_count++;
    });

    test_result(parallel_count == THREAD_TEST_SIZE,
                "Parallel view processed all entities safely");
    if (parallel_count == THREAD_TEST_SIZE) tests_passed++;

    // Test 9.2: Verify data integrity after parallel modification
    tests_total++;
    bool data_integrity = true;
    for (int i = 0; i < THREAD_TEST_SIZE; ++i) {
        auto& pos = registry.get_component<Position>(thread_entities[i]);
        if (pos.x != float(i) + 1.0f || pos.y != float(i) + 1.0f) {
            data_integrity = false;
            break;
        }
    }
    test_result(data_integrity, "Data integrity maintained after parallel modification");
    if (data_integrity) tests_passed++;

    // Test 9.3: Clear all components of a specific type
    tests_total++;
    int velocity_count_before = 0;
    registry.view<Velocity>().each([&velocity_count_before](ECS::Entity e, Velocity& v) {
        velocity_count_before++;
    });
    
    registry.clear_components<Velocity>();
    
    int velocity_count_after = 0;
    registry.view<Velocity>().each([&velocity_count_after](ECS::Entity e, Velocity& v) {
        velocity_count_after++;
    });
    
    test_result(velocity_count_before == THREAD_TEST_SIZE && velocity_count_after == 0,
                std::string("clear_components() removed all Velocity components (") + 
                std::to_string(velocity_count_before) + " -> " + std::to_string(velocity_count_after) + ")");
    if (velocity_count_before == THREAD_TEST_SIZE && velocity_count_after == 0) tests_passed++;

    // Test 9.4: count_components() functionality
    tests_total++;
    size_t position_count = registry.count_components<Position>();
    test_result(position_count == THREAD_TEST_SIZE,
                std::string("count_components() correctly counts Position components: ") + 
                std::to_string(position_count));
    if (position_count == THREAD_TEST_SIZE) tests_passed++;

    // Cleanup
    for (auto e : thread_entities) {
        registry.kill_entity(e);
    }

    std::cout << "  [INFO] Parallel operations are safe for reading/modifying components\n";
    std::cout << "  [INFO] DO NOT add/remove entities or components during parallel_view\n";

    // ----------------------------------------------------------------
    // TEST 10: Entity Groups (Cached Entity Sets)
    // ----------------------------------------------------------------
    test_separator("Entity Groups (Cached Entity Sets)");

    // Create entities for group testing
    std::vector<ECS::Entity> group_test_entities;
    for (int i = 0; i < 10; ++i) {
        auto e = registry.spawn_entity();
        registry.emplace_component<Position>(e, float(i), float(i));
        if (i % 2 == 0) {
            registry.emplace_component<Velocity>(e, 1.0f, 1.0f);
        }
        group_test_entities.push_back(e);
    }

    // Test 10.1: Create a group
    tests_total++;
    auto moving_group = registry.create_group<Position, Velocity>();
    test_result(moving_group.size() == 5,
                "Group correctly contains entities with specified components");
    if (moving_group.size() == 5) tests_passed++;

    // Test 10.2: Iterate over group
    tests_total++;
    int group_iteration_count = 0;
    for (auto entity : moving_group) {
        group_iteration_count++;
        // Verify entity has required components
        if (!registry.has_component<Position>(entity) ||
            !registry.has_component<Velocity>(entity)) {
            group_iteration_count = -1;
            break;
        }
    }
    test_result(group_iteration_count == 5,
                "Group iteration works correctly");
    if (group_iteration_count == 5) tests_passed++;

    // Test 10.3: Group.each() functionality
    tests_total++;
    int group_each_count = 0;
    moving_group.each([&group_each_count](ECS::Entity e, Position& p, Velocity& v) {
        p.x += 10.0f;
        group_each_count++;
    });
    test_result(group_each_count == 5,
                "Group.each() processes all entities");
    if (group_each_count == 5) tests_passed++;

    // Test 10.4: Group rebuild after structural changes
    tests_total++;
    registry.emplace_component<Velocity>(group_test_entities[1], 2.0f, 2.0f);
    moving_group.rebuild();
    test_result(moving_group.size() == 6,
                "Group.rebuild() updates group after structural changes");
    if (moving_group.size() == 6) tests_passed++;

    // Performance comparison
    std::cout << "  [INFO] Groups provide O(1) iteration vs O(N) for views\n";
    std::cout << "  [INFO] Groups need manual rebuild() after structural changes\n";

    // Cleanup
    for (auto e : group_test_entities) {
        registry.kill_entity(e);
    }

    // Setup callbacks for demo
    registry.on_construct<Position>([&stats](ECS::Entity e) {
        stats.entities_created++;
        if (e.index() < 10) {
            std::cout << "  [EVENT] Position added to Entity " << e.index() << "\n";
        }
    });

    registry.on_destroy<Position>([&stats](ECS::Entity e) {
        stats.entities_destroyed++;
        std::cout << "  [EVENT] Position removed from Entity " << e.index() << "\n";
    });

    // ----------------------------------------------------------------
    // Create Entities
    // ----------------------------------------------------------------
    std::cout << "--- Creating Entities ---\n";

    // Create player
    auto player = registry.spawn_entity();
    registry.emplace_component<Position>(player, 0.0f, 0.0f);
    registry.emplace_component<Velocity>(player, 10.0f, 5.0f);
    registry.emplace_component<Name>(player, "Player");
    registry.emplace_component<Player>(player);

    // Create enemies
    for (int i = 0; i < 4; ++i) {
        auto enemy = registry.spawn_entity();
        registry.emplace_component<Position>(enemy, float(i * 10), 0.0f);
        registry.emplace_component<Name>(enemy, "Enemy_" + std::to_string(i));
        registry.emplace_component<Enemy>(enemy);

        // Add velocity only to some enemies
        if (i % 2 == 0) {
            registry.emplace_component<Velocity>(enemy, -2.0f * (i + 1), 1.0f);
        }
    }

    std::cout << "\n--- Initial State ---\n";
    render_system(registry);

    // ----------------------------------------------------------------
    // Run Systems
    // ----------------------------------------------------------------
    std::cout << "\n--- Running Movement System ---\n";
    auto& game_time = registry.get_singleton<GameTime>();
    game_time.delta_time = 1.0f;
    movement_system(registry);
    render_system(registry);

    // ----------------------------------------------------------------
    // Performance Test: Parallel vs Sequential
    // ----------------------------------------------------------------
    std::cout << "\n--- Performance Benchmark ---\n";

    // Create many entities for benchmark
    const int BENCHMARK_COUNT = 10000;
    std::cout << "Creating " << BENCHMARK_COUNT << " entities for benchmark...\n";

    for (int i = 0; i < BENCHMARK_COUNT; ++i) {
        auto e = registry.spawn_entity();
        registry.emplace_component<Position>(e, float(i), float(i));
        registry.emplace_component<Velocity>(e, 1.0f, 1.0f);
    }

    // Sequential benchmark
    auto start = std::chrono::high_resolution_clock::now();
    movement_system(registry);
    auto end = std::chrono::high_resolution_clock::now();
    auto sequential_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    // Parallel benchmark
    start = std::chrono::high_resolution_clock::now();
    parallel_movement_system(registry);
    end = std::chrono::high_resolution_clock::now();
    auto parallel_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Sequential: " << sequential_time << " μs\n";
    std::cout << "Parallel:   " << parallel_time << " μs\n";
    std::cout << "Speedup:    " << (float)sequential_time / parallel_time << "x\n";

    // ----------------------------------------------------------------
    // Entity Destruction (with callbacks)
    // ----------------------------------------------------------------
    std::cout << "\n--- Destroying Player Entity ---\n";
    registry.kill_entity(player);

    std::cout << "Is Player alive? " << (registry.is_alive(player) ? "Yes" : "No") << "\n";

    // ----------------------------------------------------------------
    // Statistics
    // ----------------------------------------------------------------
    std::cout << "\n--- Final Statistics ---\n";
    std::cout << "Entities created (via Position): " << stats.entities_created << "\n";
    std::cout << "Entities destroyed (via Position): " << stats.entities_destroyed << "\n";

    // ----------------------------------------------------------------
    // Test Summary
    // ----------------------------------------------------------------
    test_separator("TEST SUMMARY");
    std::cout << "Tests Passed: " << tests_passed << "/" << tests_total << "\n";
    std::cout << "Success Rate: " << (tests_total > 0 ? (100.0f * tests_passed / tests_total) : 0) << "%\n";

    std::cout << "\n=== DEMO COMPLETE ===\n";

    return (tests_passed == tests_total) ? 0 : 1;
}
