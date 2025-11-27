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
prefabs.registerPrefab("Player", [](Registry& reg, Entity e) {
    reg.emplaceComponent<Position>(e, 0.0f, 0.0f);
    reg.emplaceComponent<Velocity>(e, 0.0f, 0.0f);
    reg.emplaceComponent<Health>(e, 100);
    reg.emplaceComponent<Player>(e);
});

// Spawn from prefab
Entity player = prefabs.instantiate("Player");
```

## API Reference

### Prefab Management

```cpp
using PrefabFunc = std::function<void(Registry&, Entity)>;

// Register prefab template
void registerPrefab(const std::string& name, PrefabFunc func);

// Unregister prefab
void unregisterPrefab(const std::string& name);

// Check if prefab exists
bool hasPrefab(const std::string& name) const;

// Get all prefab names
std::vector<std::string> getPrefabNames() const;

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
std::vector<Entity> instantiateMultiple(const std::string& name, size_t count);
```

## Advanced Usage

### Prefab with Parameters

```cpp
// Enemy prefab with customizable health
prefabs.registerPrefab("Enemy", [](Registry& reg, Entity e) {
    reg.emplaceComponent<Position>(e, 0.0f, 0.0f);
    reg.emplaceComponent<Enemy>(e);
    reg.emplaceComponent<Health>(e, 50); // Default health
});

// Spawn with customization
Entity strong_enemy = prefabs.instantiate("Enemy", [](Registry& reg, Entity e) {
    // Override health
    auto& health = reg.getComponent<Health>(e);
    health.hp = 200;
});
```

### Prefab Inheritance

```cpp
// Base character prefab
prefabs.registerPrefab("Character", [](Registry& reg, Entity e) {
    reg.emplaceComponent<Position>(e, 0.0f, 0.0f);
    reg.emplaceComponent<Velocity>(e, 0.0f, 0.0f);
    reg.emplaceComponent<Health>(e, 100);
});

// Player extends Character
prefabs.registerPrefab("Player", [&prefabs](Registry& reg, Entity e) {
    // Apply base prefab
    prefabs.instantiate_on(e, "Character");
    
    // Add player-specific components
    reg.emplaceComponent<Player>(e);
    reg.emplaceComponent<Inventory>(e);
});
```

### Prefab Composition

```cpp
// Weapon prefab
prefabs.registerPrefab("Weapon", [](Registry& reg, Entity e) {
    reg.emplaceComponent<Damage>(e, 10);
    reg.emplaceComponent<Sprite>(e, "sword.png");
});

// Player with weapon
Entity player = prefabs.instantiate("Player", [&](Registry& reg, Entity player_entity) {
    // Create weapon as separate entity
    Entity weapon = prefabs.instantiate("Weapon");
    
    // Attach weapon to player
    RelationshipManager& relationships = get_relationships(reg);
    relationships.setParent(weapon, player_entity);
});
```

### Batch Spawning

```cpp
// Spawn 100 enemies
std::vector<Entity> enemies = prefabs.instantiateMultiple("Enemy", 100);

// Customize each instance
for (size_t i = 0; i < enemies.size(); ++i) {
    auto& pos = registry.getComponent<Position>(enemies[i]);
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
        prefabs.registerPrefab("Player", [](Registry& reg, Entity e) {
            reg.emplaceComponent<Position>(e, 0.0f, 0.0f);
            reg.emplaceComponent<Velocity>(e, 0.0f, 0.0f);
            reg.emplaceComponent<Health>(e, 100);
            reg.emplaceComponent<Player>(e);
            reg.emplaceComponent<Sprite>(e, "player.png");
        });
        
        // Enemy
        prefabs.registerPrefab("Enemy", [](Registry& reg, Entity e) {
            reg.emplaceComponent<Position>(e, 0.0f, 0.0f);
            reg.emplaceComponent<Velocity>(e, 0.0f, 0.0f);
            reg.emplaceComponent<Health>(e, 50);
            reg.emplaceComponent<Enemy>(e);
            reg.emplaceComponent<AI>(e);
            reg.emplaceComponent<Sprite>(e, "enemy.png");
        });
        
        // Bullet
        prefabs.registerPrefab("Bullet", [](Registry& reg, Entity e) {
            reg.emplaceComponent<Position>(e, 0.0f, 0.0f);
            reg.emplaceComponent<Velocity>(e, 10.0f, 0.0f);
            reg.emplaceComponent<Damage>(e, 25);
            reg.emplaceComponent<Lifetime>(e, 5.0f);
            reg.emplaceComponent<Sprite>(e, "bullet.png");
        });
        
        // Particle
        prefabs.registerPrefab("Particle", [](Registry& reg, Entity e) {
            reg.emplaceComponent<Position>(e, 0.0f, 0.0f);
            reg.emplaceComponent<Velocity>(e, 0.0f, 0.0f);
            reg.emplaceComponent<Lifetime>(e, 1.0f);
            reg.emplaceComponent<Color>(e, 255, 255, 255, 255);
            reg.emplaceComponent<Particle>(e);
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
            auto& pos = reg.getComponent<Position>(e);
            pos.x = x;
            pos.y = y;
        });
    }
    
    Entity create_enemy(float x, float y, int difficulty) {
        return prefabs.instantiate("Enemy", [x, y, difficulty](Registry& reg, Entity e) {
            auto& pos = reg.getComponent<Position>(e);
            pos.x = x;
            pos.y = y;
            
            auto& health = reg.getComponent<Health>(e);
            health.hp = 50 * difficulty;
        });
    }
    
    Entity create_bullet(Entity owner, float angle) {
        return prefabs.instantiate("Bullet", [owner, angle](Registry& reg, Entity e) {
            // Position at owner's location
            if (reg.hasComponent<Position>(owner)) {
                auto& owner_pos = reg.getComponent<Position>(owner);
                auto& bullet_pos = reg.getComponent<Position>(e);
                bullet_pos.x = owner_pos.x;
                bullet_pos.y = owner_pos.y;
            }
            
            // Set velocity based on angle
            auto& vel = reg.getComponent<Velocity>(e);
            vel.dx = std::cos(angle) * 10.0f;
            vel.dy = std::sin(angle) * 10.0f;
        });
    }
};
```

## Prefab Variants

```cpp
// Register base prefab
prefabs.registerPrefab("Enemy_Base", [](Registry& reg, Entity e) {
    reg.emplaceComponent<Position>(e, 0.0f, 0.0f);
    reg.emplaceComponent<Enemy>(e);
    reg.emplaceComponent<Health>(e, 50);
});

// Create variants
prefabs.registerPrefab("Enemy_Fast", [&](Registry& reg, Entity e) {
    prefabs.instantiate_on(e, "Enemy_Base");
    reg.emplaceComponent<FastMovement>(e);
    auto& health = reg.getComponent<Health>(e);
    health.hp = 30; // Less health
});

prefabs.registerPrefab("Enemy_Tank", [&](Registry& reg, Entity e) {
    prefabs.instantiate_on(e, "Enemy_Base");
    reg.emplaceComponent<SlowMovement>(e);
    auto& health = reg.getComponent<Health>(e);
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
        prefabs.registerPrefab(prefab_data.name, [prefab_data](Registry& reg, Entity e) {
            // Instantiate components from data
            if (prefab_data.components.contains("Position")) {
                auto pos_data = std::any_cast<PositionData>(prefab_data.components.at("Position"));
                reg.emplaceComponent<Position>(e, pos_data.x, pos_data.y);
            }
            
            if (prefab_data.components.contains("Health")) {
                auto hp_data = std::any_cast<int>(prefab_data.components.at("Health"));
                reg.emplaceComponent<Health>(e, hp_data);
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
std::thread t1([&] { prefabs.registerPrefab("A", func_a); });
std::thread t2([&] { prefabs.registerPrefab("B", func_b); });

// ✅ SAFE: Concurrent instantiation (after registration)
prefabs.registerPrefab("Enemy", enemy_func);

std::thread t1([&] { prefabs.instantiate("Enemy"); });
std::thread t2([&] { prefabs.instantiate("Enemy"); });
```

## Performance

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| registerPrefab | O(1) | Hash map insertion |
| instantiate | O(k) | k = components in prefab |
| hasPrefab | O(1) | Hash map lookup |
| getPrefabNames | O(n) | n = prefab count |

### Optimization Tips

```cpp
// Pre-register all prefabs at startup
void init_prefabs(PrefabManager& prefabs) {
    prefabs.registerPrefab("Player", player_func);
    prefabs.registerPrefab("Enemy", enemy_func);
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
- Don't forget to check `hasPrefab()` for dynamic names
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
    if (registry.hasComponent<EntityPrefabRef>(e)) {
        auto& ref = registry.getComponent<EntityPrefabRef>(e);
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
