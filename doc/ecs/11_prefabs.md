# Prefabs

## Overview

**Prefabs** (prefabricated entities) are templates for spawning pre-configured entities. They provide a convenient way to create entities with consistent component sets, similar to prefabs in Unity or blueprints in Unreal.

## Core Concepts

### Prefab

A prefab is a named template function that configures an entity with components.

### Use Cases

- **Game Objects**: Player, Enemy, Bullet templates
- **Level Design**: Reusable entity types
- **Network Sync**: Consistent entity types across clients
- **Save/Load**: Serialization with entity types

## Basic Usage

```cpp
#include "ECS/Core/Prefab.hpp"

ECS::PrefabManager prefabs(registry);

// Define prefab
prefabs.register_prefab("Player", [](Registry& reg, Entity e) {
    reg.emplace_component<Position>(e, 0.0f, 0.0f);
    reg.emplace_component<Velocity>(e, 0.0f, 0.0f);
    reg.emplace_component<Health>(e, 100);
    reg.emplace_component<Player>(e);
});

// Spawn from prefab
Entity player = prefabs.instantiate("Player");
```

## API Reference

### Prefab Management

```cpp
using PrefabFunc = std::function<void(Registry&, Entity)>;

// Register prefab template
void register_prefab(const std::string& name, PrefabFunc func);

// Unregister prefab
void unregister_prefab(const std::string& name);

// Check if prefab exists
bool has_prefab(const std::string& name) const;

// Get all prefab names
std::vector<std::string> get_prefab_names() const;

// Clear all prefabs
void clear();
```

### Instantiation

```cpp
// Spawn entity from prefab
Entity instantiate(const std::string& name);

// Spawn with additional customization
Entity instantiate(const std::string& name, PrefabFunc customizer);

// Spawn multiple instances
std::vector<Entity> instantiate_multiple(const std::string& name, size_t count);
```

## Advanced Usage

### Prefab with Parameters

```cpp
// Enemy prefab with customizable health
prefabs.register_prefab("Enemy", [](Registry& reg, Entity e) {
    reg.emplace_component<Position>(e, 0.0f, 0.0f);
    reg.emplace_component<Enemy>(e);
    reg.emplace_component<Health>(e, 50); // Default health
});

// Spawn with customization
Entity strong_enemy = prefabs.instantiate("Enemy", [](Registry& reg, Entity e) {
    // Override health
    auto& health = reg.get_component<Health>(e);
    health.hp = 200;
});
```

### Prefab Inheritance

```cpp
// Base character prefab
prefabs.register_prefab("Character", [](Registry& reg, Entity e) {
    reg.emplace_component<Position>(e, 0.0f, 0.0f);
    reg.emplace_component<Velocity>(e, 0.0f, 0.0f);
    reg.emplace_component<Health>(e, 100);
});

// Player extends Character
prefabs.register_prefab("Player", [&prefabs](Registry& reg, Entity e) {
    // Apply base prefab
    prefabs.instantiate_on(e, "Character");
    
    // Add player-specific components
    reg.emplace_component<Player>(e);
    reg.emplace_component<Inventory>(e);
});
```

### Prefab Composition

```cpp
// Weapon prefab
prefabs.register_prefab("Weapon", [](Registry& reg, Entity e) {
    reg.emplace_component<Damage>(e, 10);
    reg.emplace_component<Sprite>(e, "sword.png");
});

// Player with weapon
Entity player = prefabs.instantiate("Player", [&](Registry& reg, Entity player_entity) {
    // Create weapon as separate entity
    Entity weapon = prefabs.instantiate("Weapon");
    
    // Attach weapon to player
    RelationshipManager& relationships = get_relationships(reg);
    relationships.set_parent(weapon, player_entity);
});
```

### Batch Spawning

```cpp
// Spawn 100 enemies
std::vector<Entity> enemies = prefabs.instantiate_multiple("Enemy", 100);

// Customize each instance
for (size_t i = 0; i < enemies.size(); ++i) {
    auto& pos = registry.get_component<Position>(enemies[i]);
    pos.x = i * 10.0f;
    pos.y = 0.0f;
}
```

## Prefab Library

### Complete Example

```cpp
class PrefabLibrary {
    PrefabManager& prefabs;
    
public:
    PrefabLibrary(PrefabManager& mgr) : prefabs(mgr) {
        register_all();
    }
    
    void register_all() {
        // Player
        prefabs.register_prefab("Player", [](Registry& reg, Entity e) {
            reg.emplace_component<Position>(e, 0.0f, 0.0f);
            reg.emplace_component<Velocity>(e, 0.0f, 0.0f);
            reg.emplace_component<Health>(e, 100);
            reg.emplace_component<Player>(e);
            reg.emplace_component<Sprite>(e, "player.png");
        });
        
        // Enemy
        prefabs.register_prefab("Enemy", [](Registry& reg, Entity e) {
            reg.emplace_component<Position>(e, 0.0f, 0.0f);
            reg.emplace_component<Velocity>(e, 0.0f, 0.0f);
            reg.emplace_component<Health>(e, 50);
            reg.emplace_component<Enemy>(e);
            reg.emplace_component<AI>(e);
            reg.emplace_component<Sprite>(e, "enemy.png");
        });
        
        // Bullet
        prefabs.register_prefab("Bullet", [](Registry& reg, Entity e) {
            reg.emplace_component<Position>(e, 0.0f, 0.0f);
            reg.emplace_component<Velocity>(e, 10.0f, 0.0f);
            reg.emplace_component<Damage>(e, 25);
            reg.emplace_component<Lifetime>(e, 5.0f);
            reg.emplace_component<Sprite>(e, "bullet.png");
        });
        
        // Particle
        prefabs.register_prefab("Particle", [](Registry& reg, Entity e) {
            reg.emplace_component<Position>(e, 0.0f, 0.0f);
            reg.emplace_component<Velocity>(e, 0.0f, 0.0f);
            reg.emplace_component<Lifetime>(e, 1.0f);
            reg.emplace_component<Color>(e, 255, 255, 255, 255);
            reg.emplace_component<Particle>(e);
        });
    }
};
```

### Factory Pattern

```cpp
class EntityFactory {
    PrefabManager& prefabs;
    
public:
    Entity create_player(float x, float y) {
        return prefabs.instantiate("Player", [x, y](Registry& reg, Entity e) {
            auto& pos = reg.get_component<Position>(e);
            pos.x = x;
            pos.y = y;
        });
    }
    
    Entity create_enemy(float x, float y, int difficulty) {
        return prefabs.instantiate("Enemy", [x, y, difficulty](Registry& reg, Entity e) {
            auto& pos = reg.get_component<Position>(e);
            pos.x = x;
            pos.y = y;
            
            auto& health = reg.get_component<Health>(e);
            health.hp = 50 * difficulty;
        });
    }
    
    Entity create_bullet(Entity owner, float angle) {
        return prefabs.instantiate("Bullet", [owner, angle](Registry& reg, Entity e) {
            // Position at owner's location
            if (reg.has_component<Position>(owner)) {
                auto& owner_pos = reg.get_component<Position>(owner);
                auto& bullet_pos = reg.get_component<Position>(e);
                bullet_pos.x = owner_pos.x;
                bullet_pos.y = owner_pos.y;
            }
            
            // Set velocity based on angle
            auto& vel = reg.get_component<Velocity>(e);
            vel.dx = std::cos(angle) * 10.0f;
            vel.dy = std::sin(angle) * 10.0f;
        });
    }
};
```

## Prefab Variants

```cpp
// Register base prefab
prefabs.register_prefab("Enemy_Base", [](Registry& reg, Entity e) {
    reg.emplace_component<Position>(e, 0.0f, 0.0f);
    reg.emplace_component<Enemy>(e);
    reg.emplace_component<Health>(e, 50);
});

// Create variants
prefabs.register_prefab("Enemy_Fast", [&](Registry& reg, Entity e) {
    prefabs.instantiate_on(e, "Enemy_Base");
    reg.emplace_component<FastMovement>(e);
    auto& health = reg.get_component<Health>(e);
    health.hp = 30; // Less health
});

prefabs.register_prefab("Enemy_Tank", [&](Registry& reg, Entity e) {
    prefabs.instantiate_on(e, "Enemy_Base");
    reg.emplace_component<SlowMovement>(e);
    auto& health = reg.get_component<Health>(e);
    health.hp = 200; // More health
});
```

## Data-Driven Prefabs

```cpp
struct PrefabData {
    std::string name;
    std::unordered_map<std::string, std::any> components;
};

void load_prefabs_from_json(PrefabManager& prefabs, const std::string& filepath) {
    // Parse JSON file
    auto data = parse_json(filepath);
    
    for (const auto& prefab_data : data) {
        prefabs.register_prefab(prefab_data.name, [prefab_data](Registry& reg, Entity e) {
            // Instantiate components from data
            if (prefab_data.components.contains("Position")) {
                auto pos_data = std::any_cast<PositionData>(prefab_data.components.at("Position"));
                reg.emplace_component<Position>(e, pos_data.x, pos_data.y);
            }
            
            if (prefab_data.components.contains("Health")) {
                auto hp_data = std::any_cast<int>(prefab_data.components.at("Health"));
                reg.emplace_component<Health>(e, hp_data);
            }
            
            // ... more components
        });
    }
}
```

## Thread Safety

PrefabManager is NOT thread-safe for registration:

```cpp
// ❌ UNSAFE: Concurrent registration
std::thread t1([&] { prefabs.register_prefab("A", func_a); });
std::thread t2([&] { prefabs.register_prefab("B", func_b); });

// ✅ SAFE: Concurrent instantiation (after registration)
prefabs.register_prefab("Enemy", enemy_func);

std::thread t1([&] { prefabs.instantiate("Enemy"); });
std::thread t2([&] { prefabs.instantiate("Enemy"); });
```

## Performance

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| register_prefab | O(1) | Hash map insertion |
| instantiate | O(k) | k = components in prefab |
| has_prefab | O(1) | Hash map lookup |
| get_prefab_names | O(n) | n = prefab count |

### Optimization Tips

```cpp
// Pre-register all prefabs at startup
void init_prefabs(PrefabManager& prefabs) {
    prefabs.register_prefab("Player", player_func);
    prefabs.register_prefab("Enemy", enemy_func);
    // ... all prefabs
}

// Fast instantiation during gameplay
Entity enemy = prefabs.instantiate("Enemy"); // Just hash lookup + function call
```

## Best Practices

### ✅ Do

- Register all prefabs at initialization
- Use descriptive names
- Keep prefab functions focused
- Use customizers for variants
- Document prefab requirements
- Test prefab instantiation

### ❌ Don't

- Don't register during gameplay (do it once)
- Don't use complex logic in prefab functions
- Don't forget to check `has_prefab()` for dynamic names
- Don't store state in prefab functions (use components)
- Don't create circular prefab dependencies

## Integration with Other Systems

### With Serialization

```cpp
struct EntityPrefabRef {
    std::string prefab_name;
};

// Save entity type
void save_entity(Entity e, Serializer& ser) {
    if (registry.has_component<EntityPrefabRef>(e)) {
        auto& ref = registry.get_component<EntityPrefabRef>(e);
        ser.write("prefab", ref.prefab_name);
    }
    // Save component data...
}

// Load entity
Entity load_entity(Serializer& ser, PrefabManager& prefabs) {
    std::string prefab_name = ser.read<std::string>("prefab");
    Entity e = prefabs.instantiate(prefab_name);
    // Load component data...
    return e;
}
```

### With Networking

```cpp
// Network spawn message
struct SpawnMessage {
    uint32_t network_id;
    std::string prefab_name;
    // ... additional data
};

void handle_spawn_message(const SpawnMessage& msg, PrefabManager& prefabs) {
    Entity e = prefabs.instantiate(msg.prefab_name);
    // Map network ID to entity
    network_entities[msg.network_id] = e;
}
```

## See Also

- [Registry](03_registry.md) - Entity creation
- [Relationships](10_relationships.md) - Hierarchical prefabs
- [Serialization](12_serialization.md) - Save/load prefabs
