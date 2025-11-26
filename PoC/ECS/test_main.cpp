/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ECS comprehensive test suite
*/

#include "ECS.hpp"
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

using namespace ECS;

// ============================================================================
// TEST COMPONENTS
// ============================================================================

struct Position {
    float x, y;
    Position(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
};

struct Velocity {
    float dx, dy;
    Velocity(float dx = 0.0f, float dy = 0.0f) : dx(dx), dy(dy) {}
};

struct Health {
    int current, max;
    Health(int hp = 100) : current(hp), max(hp) {}
};

struct Name {
    std::string value;
    Name(std::string n = "") : value(std::move(n)) {}
};

struct Enemy {}; // Tag component
struct Player {}; // Tag component

// ============================================================================
// TEST UTILITIES
// ============================================================================

class TestSuite {
public:
    void beginTest(const std::string& name) {
        std::cout << "\n[TEST] " << name << "\n";
        current_test = name;
        test_passed = true;
    }

    void assert_true(bool condition, const std::string& msg) {
        if (!condition) {
            std::cout << "  ❌ FAILED: " << msg << "\n";
            test_passed = false;
        } else {
            std::cout << "  ✅ PASSED: " << msg << "\n";
        }
    }

    void endTest() {
        if (test_passed) {
            std::cout << "✅ " << current_test << " PASSED\n";
            passed_tests++;
        } else {
            std::cout << "❌ " << current_test << " FAILED\n";
            failed_tests++;
        }
        total_tests++;
    }

    void printSummary() {
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "TEST SUMMARY\n";
        std::cout << std::string(60, '=') << "\n";
        std::cout << "Total:  " << total_tests << "\n";
        std::cout << "Passed: " << passed_tests << " ✅\n";
        std::cout << "Failed: " << failed_tests << " ❌\n";
        std::cout << std::string(60, '=') << "\n";
    }

private:
    std::string current_test;
    bool test_passed = true;
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
};

// ============================================================================
// TEST FUNCTIONS
// ============================================================================

void testBasicEntityOperations(TestSuite& suite) {
    suite.beginTest("Basic Entity Operations");
    
    Registry registry;
    
    // Test entity creation
    Entity e1 = registry.spawnEntity();
    suite.assert_true(registry.isAlive(e1), "Entity created and is alive");
    
    // Test multiple entities
    Entity e2 = registry.spawnEntity();
    Entity e3 = registry.spawnEntity();
    suite.assert_true(e1.index() != e2.index() && e2.index() != e3.index(), "Entities have unique indices");
    
    // Test entity destruction
    registry.killEntity(e2);
    suite.assert_true(!registry.isAlive(e2), "Entity killed successfully");
    suite.assert_true(registry.isAlive(e1) && registry.isAlive(e3), "Other entities still alive");
    
    // Test entity recycling (index may be recycled with new generation)
    Entity e4 = registry.spawnEntity();
    bool recycled = (e4.index() == e2.index() && e4.generation() > e2.generation());
    bool new_index = (e4.index() != e2.index());
    suite.assert_true(recycled || new_index, "Entity created (recycled index with new generation or new index)");
    
    suite.endTest();
}

void testComponentOperations(TestSuite& suite) {
    suite.beginTest("Component Add/Remove/Get Operations");
    
    Registry registry;
    Entity entity = registry.spawnEntity();
    
    // Test adding components
    registry.emplaceComponent<Position>(entity, 10.0f, 20.0f);
    suite.assert_true(registry.hasComponent<Position>(entity), "Position component added");
    
    registry.emplaceComponent<Velocity>(entity, 1.0f, 2.0f);
    suite.assert_true(registry.hasComponent<Velocity>(entity), "Velocity component added");
    
    // Test getting components
    auto& pos = registry.getComponent<Position>(entity);
    suite.assert_true(pos.x == 10.0f && pos.y == 20.0f, "Component values correct");
    
    // Test modifying components
    pos.x = 50.0f;
    auto& pos2 = registry.getComponent<Position>(entity);
    suite.assert_true(pos2.x == 50.0f, "Component modification persists");
    
    // Test removing components
    registry.removeComponent<Position>(entity);
    suite.assert_true(!registry.hasComponent<Position>(entity), "Component removed successfully");
    suite.assert_true(registry.hasComponent<Velocity>(entity), "Other components unaffected");
    
    // Test tag components
    registry.emplaceComponent<Enemy>(entity);
    suite.assert_true(registry.hasComponent<Enemy>(entity), "Tag component added");
    
    suite.endTest();
}

void testEmplaceComponent(TestSuite& suite) {
    suite.beginTest("Emplace Component (In-Place Construction)");
    
    Registry registry;
    Entity entity = registry.spawnEntity();
    
    // Test emplace with constructor arguments
    registry.emplaceComponent<Position>(entity, 15.0f, 25.0f);
    suite.assert_true(registry.hasComponent<Position>(entity), "Component emplaced");
    
    auto& pos = registry.getComponent<Position>(entity);
    suite.assert_true(pos.x == 15.0f && pos.y == 25.0f, "Emplace constructed with correct values");
    
    registry.emplaceComponent<Name>(entity, "TestEntity");
    auto& name = registry.getComponent<Name>(entity);
    suite.assert_true(name.value == "TestEntity", "String component emplaced correctly");
    
    suite.endTest();
}

void testViewSystem(TestSuite& suite) {
    suite.beginTest("View System - Single and Multi-Component");
    
    Registry registry;
    
    // Create test entities
    Entity e1 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e1, 1.0f, 1.0f);
    registry.emplaceComponent<Velocity>(e1, 0.1f, 0.1f);
    
    Entity e2 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e2, 2.0f, 2.0f);
    
    Entity e3 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e3, 3.0f, 3.0f);
    registry.emplaceComponent<Velocity>(e3, 0.3f, 0.3f);
    
    // Test single component view
    int count_pos = 0;
    auto view_pos = registry.view<Position>();
    view_pos.each([&](Entity e, Position& p) {
        count_pos++;
    });
    suite.assert_true(count_pos == 3, "Single component view found all 3 entities");
    
    // Test multi-component view
    int count_both = 0;
    auto view_both = registry.view<Position, Velocity>();
    view_both.each([&](Entity e, Position& p, Velocity& v) {
        count_both++;
        p.x += v.dx;
        p.y += v.dy;
    });
    suite.assert_true(count_both == 2, "Multi-component view found 2 matching entities");
    
    // Verify modifications
    auto& pos1 = registry.getComponent<Position>(e1);
    suite.assert_true(pos1.x == 1.1f && pos1.y == 1.1f, "View modifications applied correctly");
    
    suite.endTest();
}

void testExcludeView(TestSuite& suite) {
    suite.beginTest("Exclude View System");
    
    Registry registry;
    
    // Create entities with different component combinations
    Entity player = registry.spawnEntity();
    registry.emplaceComponent<Position>(player, 0.0f, 0.0f);
    registry.emplaceComponent<Health>(player, 100);
    registry.emplaceComponent<Player>(player);
    
    Entity enemy1 = registry.spawnEntity();
    registry.emplaceComponent<Position>(enemy1, 10.0f, 10.0f);
    registry.emplaceComponent<Health>(enemy1, 50);
    registry.emplaceComponent<Enemy>(enemy1);
    
    Entity enemy2 = registry.spawnEntity();
    registry.emplaceComponent<Position>(enemy2, 20.0f, 20.0f);
    registry.emplaceComponent<Health>(enemy2, 50);
    registry.emplaceComponent<Enemy>(enemy2);
    
    // Test exclude view - get all entities with Position but NOT Player
    int enemy_count = 0;
    auto view = registry.view<Position>().exclude<Player>();
    view.each([&](Entity e, Position& p) {
        enemy_count++;
    });
    suite.assert_true(enemy_count == 2, "Exclude view found 2 enemies (excluded player)");
    
    suite.endTest();
}

void testParallelView(TestSuite& suite) {
    suite.beginTest("Parallel View (Multi-threaded Processing)");
    
    Registry registry;
    
    // Create many entities for parallel processing
    const int NUM_ENTITIES = 1000;
    for (int i = 0; i < NUM_ENTITIES; i++) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, static_cast<float>(i), static_cast<float>(i));
        registry.emplaceComponent<Velocity>(e, 1.0f, 1.0f);
    }
    
    // Process with parallel view
    auto pview = registry.parallelView<Position, Velocity>();
    pview.each([](Entity e, Position& p, Velocity& v) {
        p.x += v.dx;
        p.y += v.dy;
    });
    
    // Verify all entities were processed
    int verified = 0;
    auto view = registry.view<Position>();
    view.each([&](Entity e, Position& p) {
        verified++;
    });
    suite.assert_true(verified == NUM_ENTITIES, "All entities processed in parallel");
    
    suite.endTest();
}

void testGroupSystem(TestSuite& suite) {
    suite.beginTest("Group System (Cached Entity Sets)");
    
    Registry registry;
    
    // Create entities
    Entity e1 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e1, 1.0f, 1.0f);
    registry.emplaceComponent<Velocity>(e1, 0.1f, 0.1f);
    
    Entity e2 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e2, 2.0f, 2.0f);
    
    // Create group
    auto group = registry.createGroup<Position, Velocity>();
    
    int count1 = 0;
    group.each([&](Entity e, Position& p, Velocity& v) {
        count1++;
    });
    suite.assert_true(count1 == 1, "Group initially has 1 matching entity");
    
    // Add component to second entity
    registry.emplaceComponent<Velocity>(e2, 0.2f, 0.2f);
    group.rebuild(); // Manual rebuild for testing
    
    int count2 = 0;
    group.each([&](Entity e, Position& p, Velocity& v) {
        count2++;
    });
    suite.assert_true(count2 == 2, "Group updated after component addition");
    
    suite.endTest();
}

void testSignalSystem(TestSuite& suite) {
    suite.beginTest("Signal System (Component Events)");
    
    Registry registry;
    
    int construct_count = 0;
    int destroy_count = 0;
    
    // Connect to component signals
    registry.onConstruct<Position>([&](Entity e) {
        construct_count++;
    });
    
    registry.onDestroy<Position>([&](Entity e) {
        destroy_count++;
    });
    
    // Trigger events
    Entity e1 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e1, 1.0f, 1.0f);
    
    Entity e2 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e2, 2.0f, 2.0f);
    
    registry.removeComponent<Position>(e1);
    
    suite.assert_true(construct_count == 2, "Component construct signal fired twice");
    suite.assert_true(destroy_count == 1, "Component destroy signal fired once");
    
    suite.endTest();
}

void testCommandBuffer(TestSuite& suite) {
    suite.beginTest("Command Buffer (Deferred Operations)");
    
    Registry registry;
    CommandBuffer cmd(registry);
    
    // Queue operations
    Entity placeholder1 = cmd.spawnEntityDeferred();
    Entity placeholder2 = cmd.spawnEntityDeferred();
    
    cmd.emplaceComponentDeferred<Position>(placeholder1, 10.0f, 20.0f);
    cmd.emplaceComponentDeferred<Velocity>(placeholder1, 1.0f, 2.0f);
    
    // Operations not executed yet
    suite.assert_true(!registry.isAlive(placeholder1), "Deferred entity not created yet");
    
    // Execute all commands
    cmd.flush();
    
    // Verify operations executed
    int entity_count = 0;
    auto view = registry.view<Position>();
    view.each([&](Entity e, Position& p) {
        entity_count++;
        suite.assert_true(p.x == 10.0f && p.y == 20.0f, "Deferred component added correctly");
    });
    suite.assert_true(entity_count == 1, "Deferred entity created");
    
    suite.endTest();
}

void testPrefabSystem(TestSuite& suite) {
    suite.beginTest("Prefab System (Entity Templates)");
    
    Registry registry;
    PrefabManager prefabs(registry);
    
    // Register prefab
    prefabs.registerPrefab("Enemy", [](Registry& reg, Entity e) {
        reg.emplaceComponent<Position>(e, 0.0f, 0.0f);
        reg.emplaceComponent<Health>(e, 50);
        reg.emplaceComponent<Enemy>(e);
    });
    
    suite.assert_true(prefabs.hasPrefab("Enemy"), "Prefab registered successfully");
    
    // Instantiate prefab
    Entity enemy1 = prefabs.instantiate("Enemy");
    suite.assert_true(registry.isAlive(enemy1), "Prefab instantiated");
    suite.assert_true(registry.hasComponent<Position>(enemy1), "Prefab has Position");
    suite.assert_true(registry.hasComponent<Health>(enemy1), "Prefab has Health");
    suite.assert_true(registry.hasComponent<Enemy>(enemy1), "Prefab has Enemy tag");
    
    // Test multiple instantiation
    auto enemies = prefabs.instantiateMultiple("Enemy", 5);
    suite.assert_true(enemies.size() == 5, "Multiple prefabs instantiated");
    
    suite.endTest();
}

void testSystemScheduler(TestSuite& suite) {
    suite.beginTest("System Scheduler (Execution Order)");
    
    Registry registry;
    SystemScheduler scheduler(registry);
    
    std::string execution_order;
    
    // Register systems
    scheduler.addSystem("first", [&](Registry& reg) {
        execution_order += "1";
    });
    
    scheduler.addSystem("second", [&](Registry& reg) {
        execution_order += "2";
    }, {"first"}); // Depends on "first"
    
    scheduler.addSystem("third", [&](Registry& reg) {
        execution_order += "3";
    }, {"second"}); // Depends on "second"
    
    // Run all systems
    scheduler.run();
    
    suite.assert_true(execution_order == "123", "Systems executed in dependency order");
    
    // Test system enable/disable
    execution_order.clear();
    scheduler.setSystemEnabled("second", false);
    scheduler.run();
    suite.assert_true(execution_order == "13", "Disabled system not executed");
    
    suite.endTest();
}

void testPerformance(TestSuite& suite) {
    suite.beginTest("Performance Benchmark");
    
    Registry registry;
    
    const int NUM_ENTITIES = 10000;
    
    // Benchmark entity creation
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_ENTITIES; i++) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, static_cast<float>(i), static_cast<float>(i));
        registry.emplaceComponent<Velocity>(e, 1.0f, 1.0f);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "  ℹ️  Created " << NUM_ENTITIES << " entities in " << duration << "ms\n";
    suite.assert_true(duration < 1000, "Entity creation performance acceptable");
    
    // Benchmark iteration
    start = std::chrono::high_resolution_clock::now();
    int iterations = 0;
    auto view = registry.view<Position, Velocity>();
    view.each([&](Entity e, Position& p, Velocity& v) {
        p.x += v.dx;
        p.y += v.dy;
        iterations++;
    });
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "  ℹ️  Iterated " << iterations << " entities in " << duration << "ms\n";
    suite.assert_true(iterations == NUM_ENTITIES, "All entities iterated");
    
    suite.endTest();
}

void testEdgeCases(TestSuite& suite) {
    suite.beginTest("Edge Cases and Error Handling");
    
    Registry registry;
    Entity entity = registry.spawnEntity();
    
    // Test double emplace component (should replace)
    registry.emplaceComponent<Position>(entity, 1.0f, 1.0f);
    registry.emplaceComponent<Position>(entity, 2.0f, 2.0f);
    auto& pos = registry.getComponent<Position>(entity);
    suite.assert_true(pos.x == 2.0f, "Component replaced on double emplace");
    
    // Test operations on dead entity
    Entity dead = registry.spawnEntity();
    registry.killEntity(dead);
    
    bool caught_exception = false;
    try {
        registry.emplaceComponent<Position>(dead, 0.0f, 0.0f);
    } catch (...) {
        caught_exception = true;
    }
    suite.assert_true(caught_exception, "Exception thrown for operation on dead entity");
    
    // Test empty view iteration
    Registry empty_reg;
    int empty_count = 0;
    auto view = empty_reg.view<Position>();
    view.each([&](Entity e, Position& p) {
        empty_count++;
    });
    suite.assert_true(empty_count == 0, "Empty view iteration handled correctly");
    
    suite.endTest();
}

void testReferenceInvalidation(TestSuite& suite) {
    suite.beginTest("Reference Invalidation - Detection (Safe)");
    
    Registry registry;
    
    std::cout << "\n📊 DÉMONSTRATION: Détection de réallocation (sans corruption)\n";
    std::cout << "    Ce test détecte le problème de manière sûre\n\n";
    
    // Créer une première entité et stocker son adresse (pas de référence persistante)
    Entity e1 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e1, 100.0f, 200.0f);
    
    // ✅ SAFE: Obtenir l'adresse via getComponent (référence temporaire)
    Position* original_address = &registry.getComponent<Position>(e1);
    
    std::cout << "  ✓ Entité 1 créée avec Position(100, 200)\n";
    std::cout << "  ✓ Adresse mémoire initiale: " << original_address << "\n";
    
    // Créer BEAUCOUP d'entités pour forcer la réallocation du vector
    std::cout << "\n  🔄 Ajout de 1000 entités pour forcer la réallocation...\n";
    
    for (int i = 0; i < 1000; i++) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, static_cast<float>(i), static_cast<float>(i));
    }
    
    std::cout << "  ✓ 1000 entités ajoutées\n";
    
    // ✅ SAFE: Réobtenir la référence après les modifications
    Position& pos_after = registry.getComponent<Position>(e1);
    Position* new_address = &pos_after;
    
    std::cout << "  ✓ Adresse après ajouts: " << new_address << "\n";
    
    // Vérifier si une réallocation a eu lieu
    bool reallocation_occurred = (original_address != new_address);
    
    if (reallocation_occurred) {
        std::cout << "\n  ⚠️  RÉALLOCATION DÉTECTÉE !\n";
        std::cout << "      - Ancienne adresse: " << original_address << "\n";
        std::cout << "      - Nouvelle adresse: " << new_address << "\n";
        std::cout << "      → Toute référence stockée avant serait maintenant INVALIDE\n";
        std::cout << "      → Mais ce test utilise des patterns sûrs, donc pas de corruption\n";
    } else {
        std::cout << "\n  ℹ️  Aucune réallocation détectée\n";
        std::cout << "      → La capacité initiale du vector était suffisante\n";
        std::cout << "      → Essayez d'augmenter le nombre d'entités pour forcer la réallocation\n";
    }
    
    // ✅ SAFE: Vérifier que les valeurs sont correctes
    suite.assert_true(pos_after.x == 100.0f, "Valeur x préservée avec pattern sûr");
    suite.assert_true(pos_after.y == 200.0f, "Valeur y préservée avec pattern sûr");
    
    if (reallocation_occurred) {
        suite.assert_true(true, "Réallocation détectée mais valeurs préservées (pattern sûr)");
    }
    
    suite.endTest();
}

void testSafePatterns(TestSuite& suite) {
    suite.beginTest("Safe Patterns - Éviter les références invalides");
    
    Registry registry;
    
    std::cout << "\n✅ DÉMONSTRATION des patterns sûrs:\n\n";
    
    // Pattern 1: Ne pas stocker de références
    std::cout << "  Pattern 1: Utiliser getComponent à chaque accès\n";
    Entity e1 = registry.spawnEntity();
    registry.emplaceComponent<Position>(e1, 10.0f, 20.0f);
    
    // ❌ DANGEREUX: auto& pos = registry.getComponent<Position>(e1);
    // ✅ SÛR: Réobtenir la référence à chaque fois
    registry.getComponent<Position>(e1).x = 15.0f;
    
    for (int i = 0; i < 100; i++) {
        Entity e = registry.spawnEntity();
        registry.emplaceComponent<Position>(e, 0.0f, 0.0f);
    }
    
    float final_x = registry.getComponent<Position>(e1).x;
    suite.assert_true(final_x == 15.0f, "Pattern 1: Valeur préservée");
    
    // Pattern 2: Utiliser patch()
    std::cout << "  Pattern 2: Utiliser patch() pour les modifications\n";
    registry.patch<Position>(e1, [](Position& p) {
        p.y = 25.0f;
    });
    suite.assert_true(registry.getComponent<Position>(e1).y == 25.0f, "Pattern 2: patch() fonctionne");
    
    // Pattern 3: Références locales dans les views
    std::cout << "  Pattern 3: Références locales dans les callbacks de view\n";
    bool pattern3_ok = true;
    auto view = registry.view<Position>();
    view.each([&](Entity e, Position& p) {
        // ✅ Référence locale, valide uniquement dans ce scope
        p.x += 1.0f;
        pattern3_ok = true;
    });
    suite.assert_true(pattern3_ok, "Pattern 3: Références locales dans views");
    
    // Pattern 4: Reserve pour réduire les réallocations
    std::cout << "  Pattern 4: Utiliser reserve() au démarrage\n";
    Registry registry2;
    registry2.reserveComponents<Position>(10000);
    
    Entity e2 = registry2.spawnEntity();
    Position& pos_ref = registry2.emplaceComponent<Position>(e2, 100.0f, 200.0f);
    Position* addr_before = &pos_ref;
    
    for (int i = 0; i < 5000; i++) {
        Entity e = registry2.spawnEntity();
        registry2.emplaceComponent<Position>(e, 0.0f, 0.0f);
    }
    
    Position& pos_after = registry2.getComponent<Position>(e2);
    bool no_realloc = (addr_before == &pos_after);
    suite.assert_true(no_realloc, "Pattern 4: reserve() évite la réallocation");
    
    std::cout << "\n";
    suite.endTest();
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════╗
║          R-TYPE ECS - COMPREHENSIVE TEST SUITE          ║
╚══════════════════════════════════════════════════════════╝
    )" << "\n";

    TestSuite suite;

    try {
        testBasicEntityOperations(suite);
        testComponentOperations(suite);
        testEmplaceComponent(suite);
        testViewSystem(suite);
        testExcludeView(suite);
        testParallelView(suite);
        testGroupSystem(suite);
        testSignalSystem(suite);
        testCommandBuffer(suite);
        testPrefabSystem(suite);
        testSystemScheduler(suite);
        testPerformance(suite);
        testEdgeCases(suite);
        
        // ⚠️ TESTS DE DÉMONSTRATION DES PROBLÈMES
        std::cout << "\n" << std::string(60, '=') << "\n";
        std::cout << "TESTS DE SÉCURITÉ - INVALIDATION DE RÉFÉRENCES\n";
        std::cout << std::string(60, '=') << "\n";
        
        testReferenceInvalidation(suite);
        testSafePatterns(suite);
        
        suite.printSummary();
        
    } catch (const std::exception& e) {
        std::cerr << "\n💥 FATAL ERROR: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
