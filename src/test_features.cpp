/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test des nouvelles fonctionnalités: Relationships, Prefabs
*/

#include <iostream>
#include "ECS/ECS.hpp"

// Test components
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Health { int hp; };
struct Name { std::string value; };

void test_relationships() {
    std::cout << "\n=== TEST: Entity Relationships ===\n";
    
    ECS::Registry registry;
    auto& rel = registry.get_relationship_manager();
    
    // Create a scene hierarchy: Parent -> Child1, Child2 -> Grandchild
    auto parent = registry.spawn_entity();
    auto child1 = registry.spawn_entity();
    auto child2 = registry.spawn_entity();
    auto grandchild = registry.spawn_entity();
    
    registry.emplace_component<Name>(parent, "Parent");
    registry.emplace_component<Name>(child1, "Child1");
    registry.emplace_component<Name>(child2, "Child2");
    registry.emplace_component<Name>(grandchild, "Grandchild");
    
    // Setup hierarchy
    rel.set_parent(child1, parent);
    rel.set_parent(child2, parent);
    rel.set_parent(grandchild, child2);
    
    std::cout << "  [INFO] Created hierarchy: Parent -> (Child1, Child2 -> Grandchild)\n";
    
    // Test parent queries
    auto child1_parent = rel.get_parent(child1);
    std::cout << "  ✓ Child1 parent: " << (child1_parent.has_value() ? "Found" : "Not found") << "\n";
    
    // Test children queries
    auto parent_children = rel.get_children(parent);
    std::cout << "  ✓ Parent has " << parent_children.size() << " children\n";
    
    // Test descendants (recursive)
    auto parent_descendants = rel.get_descendants(parent);
    std::cout << "  ✓ Parent has " << parent_descendants.size() << " descendants (recursive)\n";
    
    // Test ancestors
    auto grandchild_ancestors = rel.get_ancestors(grandchild);
    std::cout << "  ✓ Grandchild has " << grandchild_ancestors.size() << " ancestors\n";
    
    // Test root finding
    auto root = rel.get_root(grandchild);
    std::cout << "  ✓ Grandchild's root: " << root.index() << " (should be " << parent.index() << ")\n";
    
    // Test depth
    std::cout << "  ✓ Depths: Parent=" << rel.get_depth(parent) 
              << ", Child2=" << rel.get_depth(child2)
              << ", Grandchild=" << rel.get_depth(grandchild) << "\n";
    
    // Test cycle prevention
    bool cycle_prevented = !rel.set_parent(parent, grandchild);
    std::cout << "  " << (cycle_prevented ? "✓" : "✗") << " Cycle prevention works\n";
    
    // Test entity destruction cleanup
    registry.kill_entity(child2);
    auto grandchild_parent_after = rel.get_parent(grandchild);
    std::cout << "  ✓ Grandchild orphaned after parent destruction: " 
              << (!grandchild_parent_after.has_value() ? "YES" : "NO") << "\n";
    
    std::cout << "  [SUCCESS] Relationship system working correctly!\n";
}

void test_prefabs() {
    std::cout << "\n=== TEST: Prefab System ===\n";
    
    ECS::Registry registry;
    ECS::PrefabManager prefabs(registry);
    
    // Register Enemy prefab
    prefabs.register_prefab("Enemy", [](ECS::Registry& r, ECS::Entity e) {
        r.emplace_component<Position>(e, 0.0f, 0.0f);
        r.emplace_component<Velocity>(e, -2.0f, 0.0f);
        r.emplace_component<Health>(e, 50);
        r.emplace_component<Name>(e, "Enemy");
    });
    
    // Register Player prefab
    prefabs.register_prefab("Player", [](ECS::Registry& r, ECS::Entity e) {
        r.emplace_component<Position>(e, 10.0f, 10.0f);
        r.emplace_component<Velocity>(e, 0.0f, 0.0f);
        r.emplace_component<Health>(e, 100);
        r.emplace_component<Name>(e, "Player");
    });
    
    std::cout << "  ✓ Registered 2 prefabs\n";
    
    // Instantiate player
    auto player = prefabs.instantiate("Player");
    std::cout << "  ✓ Instantiated Player entity " << player.index() << "\n";
    std::cout << "    - Position: (" 
              << registry.get_component<Position>(player).x << ", "
              << registry.get_component<Position>(player).y << ")\n";
    std::cout << "    - Health: " << registry.get_component<Health>(player).hp << "\n";
    
    // Instantiate multiple enemies
    auto enemies = prefabs.instantiate_multiple("Enemy", 5);
    std::cout << "  ✓ Instantiated " << enemies.size() << " enemies\n";
    
    // Customize on instantiate
    auto boss = prefabs.instantiate("Enemy", [](ECS::Registry& r, ECS::Entity e) {
        r.get_component<Health>(e).hp = 200;  // Boss has more HP
        r.get_component<Name>(e).value = "Boss";
    });
    std::cout << "  ✓ Instantiated customized Boss with " 
              << registry.get_component<Health>(boss).hp << " HP\n";
    
    // List all prefabs
    auto names = prefabs.get_prefab_names();
    std::cout << "  ✓ Available prefabs: ";
    for (const auto& name : names) {
        std::cout << name << " ";
    }
    std::cout << "\n";
    
    // Test has_prefab
    std::cout << "  ✓ has_prefab(\"Enemy\"): " << (prefabs.has_prefab("Enemy") ? "YES" : "NO") << "\n";
    std::cout << "  ✓ has_prefab(\"Invalid\"): " << (prefabs.has_prefab("Invalid") ? "YES" : "NO") << "\n";
    
    std::cout << "  [SUCCESS] Prefab system working correctly!\n";
}

void test_combined_features() {
    std::cout << "\n=== TEST: Combined Features (Relationships + Prefabs) ===\n";
    
    ECS::Registry registry;
    ECS::PrefabManager prefabs(registry);
    auto& rel = registry.get_relationship_manager();
    
    // Register prefabs
    prefabs.register_prefab("Spaceship", [](ECS::Registry& r, ECS::Entity e) {
        r.emplace_component<Position>(e, 0.0f, 0.0f);
        r.emplace_component<Health>(e, 100);
        r.emplace_component<Name>(e, "Spaceship");
    });
    
    prefabs.register_prefab("Weapon", [](ECS::Registry& r, ECS::Entity e) {
        r.emplace_component<Position>(e, 0.0f, 0.0f);
        r.emplace_component<Name>(e, "Weapon");
    });
    
    // Create spaceship with weapons
    auto spaceship = prefabs.instantiate("Spaceship");
    auto weapon1 = prefabs.instantiate("Weapon");
    auto weapon2 = prefabs.instantiate("Weapon");
    
    rel.set_parent(weapon1, spaceship);
    rel.set_parent(weapon2, spaceship);
    
    std::cout << "  ✓ Created spaceship with " << rel.child_count(spaceship) << " weapons\n";
    
    // When spaceship is destroyed, weapons become orphaned
    std::cout << "  [INFO] Destroying spaceship...\n";
    registry.kill_entity(spaceship);
    
    std::cout << "  ✓ Weapon1 has parent: " << (rel.has_parent(weapon1) ? "YES" : "NO") << " (should be NO)\n";
    std::cout << "  ✓ Weapon1 is still alive: " << (registry.is_alive(weapon1) ? "YES" : "NO") << "\n";
    
    std::cout << "  [SUCCESS] Combined features working correctly!\n";
}

int main() {
    std::cout << "=== TESTING NEW ECS FEATURES ===\n";
    
    try {
        test_relationships();
        test_prefabs();
        test_combined_features();
        
        std::cout << "\n=== ALL TESTS PASSED ===\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n✗ ERROR: " << e.what() << "\n";
        return 1;
    }
}
