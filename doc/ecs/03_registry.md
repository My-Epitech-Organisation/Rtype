# Registry

## Overview

The **Registry** is the central coordinator of the ECS, managing entities, components, views, and global resources. It serves as the primary interface for all ECS operations.

## Core Responsibilities

1. **Entity Lifecycle**: Creation, destruction, validation
2. **Component Management**: Add, remove, query components
3. **View Creation**: Query entities with specific components
4. **Signal Dispatching**: Event notifications for component changes
5. **Resource Management**: Global singleton objects
6. **Memory Management**: Capacity reservation and compaction

## Basic Usage

```cpp
#include "ECS/ECS.hpp"

ECS::Registry registry;

// Create entity
Entity player = registry.spawn_entity();

// Add components
registry.emplace_component<Position>(player, 0.0f, 0.0f);
registry.emplace_component<Velocity>(player, 5.0f, 0.0f);
registry.emplace_component<Player>(player); // Tag

// Check component
if (registry.has_component<Position>(player)) {
    Position& pos = registry.get_component<Position>(player);
    pos.x += 1.0f;
}

// Remove component
registry.remove_component<Velocity>(player);

// Destroy entity
registry.kill_entity(player);
```

## Entity Management

### Creation

```cpp
// Create single entity
Entity entity = registry.spawn_entity();

// Reserve capacity for batch creation
registry.reserve_entities(10000);
std::vector<Entity> entities;
for (int i = 0; i < 10000; ++i) {
    entities.push_back(registry.spawn_entity());
}
```

### Validation

```cpp
Entity entity = registry.spawn_entity();

// Check if entity is valid
bool alive = registry.is_alive(entity);

// Entity remains invalid after destruction
registry.kill_entity(entity);
alive = registry.is_alive(entity); // false
```

### Cleanup

```cpp
// Remove all dead entities and recycle slots
size_t recycled = registry.cleanup_tombstones();

// Remove entities conditionally
size_t removed = registry.remove_entities_if([](Entity e) {
    return /* condition */;
});
```

## Component Management

### Adding Components

```cpp
// Emplace with constructor arguments
registry.emplace_component<Transform>(entity, position, rotation, scale);

// Emplace and modify
auto& health = registry.emplace_component<Health>(entity, 100);
health.max_hp = 100;

// Idempotent - replaces if exists
registry.emplace_component<Position>(entity, 0.0f, 0.0f);
registry.emplace_component<Position>(entity, 5.0f, 5.0f); // Replaces
```

### Accessing Components

```cpp
// Get component (throws if missing)
try {
    Position& pos = registry.get_component<Position>(entity);
    pos.x += 1.0f;
} catch (const std::runtime_error& e) {
    // Component missing
}

// Try-get with check
if (registry.has_component<Velocity>(entity)) {
    auto& vel = registry.get_component<Velocity>(entity);
    vel.dx *= 0.99f;
}

// Get multiple components at once
if (registry.all_of<Position, Velocity>(entity)) {
    auto& pos = registry.get_component<Position>(entity);
    auto& vel = registry.get_component<Velocity>(entity);
    pos.x += vel.dx;
}
```

### Removing Components

```cpp
// Remove single component
registry.remove_component<Velocity>(entity);

// Check before removing
if (registry.has_component<Temporary>(entity)) {
    registry.remove_component<Temporary>(entity);
}
```

### Bulk Operations

```cpp
// Get component count for a type
size_t count = registry.component_count<Position>();

// Clear all components of a type
registry.clear_components<Position>();

// Reserve component capacity
registry.reserve_components<Position>(10000);
registry.reserve_components<Velocity>(10000);
```

## View Creation

Views allow querying entities with specific component combinations.

```cpp
// Single component
auto view = registry.view<Position>();

// Multiple components (intersection)
auto view = registry.view<Position, Velocity>();

// Exclude components
auto view = registry.view<Position, Velocity>()
                   .exclude<Dead, Frozen>();

// Parallel iteration
auto view = registry.parallel_view<Position, Velocity>();

// Cached group
auto group = registry.create_group<Position, Velocity>();
```

See [Views](04_views.md) for detailed usage.

## Resource Management

Singleton resources are globally accessible objects.

```cpp
// Define resource
struct GameConfig {
    int width = 1920;
    int height = 1080;
    float time_scale = 1.0f;
};

// Add resource
registry.set_resource<GameConfig>(1920, 1080, 1.0f);

// Access resource
GameConfig& config = registry.get_resource<GameConfig>();
config.time_scale = 0.5f; // Slow motion

// Check existence
if (registry.has_resource<GameConfig>()) {
    // Use resource
}

// Remove resource
registry.remove_resource<GameConfig>();
```

### Resource Use Cases

- **Configuration**: Game settings, constants
- **Services**: Audio manager, physics world
- **Shared State**: Delta time, frame counter
- **Singleton Systems**: Input handler, renderer

## Signal System

Register callbacks for component lifecycle events.

```cpp
// Listen for component additions
registry.on_construct<Health>([](Entity e) {
    std::cout << "Entity " << e.index() << " gained Health\n";
});

// Listen for component removals
registry.on_destroy<Health>([](Entity e) {
    std::cout << "Entity " << e.index() << " lost Health\n";
});
```

Signals are dispatched automatically when components are added/removed.

See [Signal System](07_signals.md) for details.

## Memory Management

### Reservation

Pre-allocate memory to avoid reallocations:

```cpp
// Reserve entity capacity
registry.reserve_entities(10000);

// Reserve component capacity
registry.reserve_components<Position>(10000);
registry.reserve_components<Velocity>(10000);
```

### Compaction

Reclaim memory after bulk deletions:

```cpp
// Compact specific component type
registry.compact_component<Position>();

// Compact all component types
registry.compact();
```

### Clearing

```cpp
// Clear all components of a type
registry.clear_components<Position>();

// Clear all entities and components
registry.clear();
```

## API Reference

### Entity Operations

```cpp
Entity spawn_entity();
void kill_entity(Entity entity) noexcept;
bool is_alive(Entity entity) const noexcept;
size_t cleanup_tombstones();
void reserve_entities(size_t capacity);

template<typename Func>
size_t remove_entities_if(Func&& predicate);
```

### Component Operations

```cpp
template<typename T, typename... Args>
T& emplace_component(Entity entity, Args&&... args);

template<typename T>
void remove_component(Entity entity);

template<typename T>
bool has_component(Entity entity) const;

template<typename T>
T& get_component(Entity entity);

template<typename... Ts>
bool all_of(Entity entity) const;

template<typename... Ts>
bool any_of(Entity entity) const;

template<typename T>
size_t component_count() const;

template<typename T>
void clear_components();

template<typename T>
void reserve_components(size_t capacity);

template<typename T>
void compact_component();

void compact();
```

### View Operations

```cpp
template<typename... Components>
View<Components...> view();

template<typename... Components>
ParallelView<Components...> parallel_view();

template<typename... Components>
Group<Components...> create_group();
```

### Resource Operations

```cpp
template<typename T, typename... Args>
T& set_resource(Args&&... args);

template<typename T>
T& get_resource();

template<typename T>
bool has_resource() const;

template<typename T>
void remove_resource();
```

### Signal Operations

```cpp
template<typename T>
void on_construct(std::function<void(Entity)> callback);

template<typename T>
void on_destroy(std::function<void(Entity)> callback);
```

## Thread Safety

### Safe Operations

- Entity creation/destruction (internally locked)
- Component add/remove on different entities (internally locked)
- Reading different component types concurrently
- Resource access (read-only from multiple threads)

### Unsafe Operations

❌ **Never do these during parallel iteration:**

- Adding/removing entities
- Adding/removing components
- Structural changes to the registry

✅ **Use CommandBuffer instead:**

```cpp
CommandBuffer cmd(registry);

registry.parallel_view<Position>().each([&](Entity e, Position& pos) {
    if (pos.x > 100.0f) {
        cmd.destroy_entity_deferred(e); // Safe
    }
});

cmd.flush(); // Apply changes after iteration
```

## Performance Tips

### Do's ✅

1. **Reserve capacity** for known entity/component counts
2. **Use views** instead of manual iteration
3. **Batch operations** when possible
4. **Compact** after bulk deletions
5. **Use tags** for boolean flags
6. **Keep components small** (cache-friendly)

### Don'ts ❌

1. Don't add/remove during iteration (use CommandBuffer)
2. Don't store component references across frames
3. Don't perform random entity lookups in hot loops
4. Don't create entities in tight loops without reservation
5. Don't store large data in components (use handles)

## Complete Example

```cpp
#include "ECS/ECS.hpp"
#include <iostream>

struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Health { int hp; };
struct Player {}; // Tag

int main() {
    ECS::Registry registry;
    
    // Reserve capacity
    registry.reserve_entities(100);
    registry.reserve_components<Position>(100);
    
    // Create player
    auto player = registry.spawn_entity();
    registry.emplace_component<Position>(player, 0.0f, 0.0f);
    registry.emplace_component<Velocity>(player, 1.0f, 0.0f);
    registry.emplace_component<Health>(player, 100);
    registry.emplace_component<Player>(player);
    
    // Create enemies
    for (int i = 0; i < 10; ++i) {
        auto enemy = registry.spawn_entity();
        registry.emplace_component<Position>(enemy, i * 10.0f, 0.0f);
        registry.emplace_component<Velocity>(enemy, -0.5f, 0.0f);
        registry.emplace_component<Health>(enemy, 50);
    }
    
    // Movement system
    registry.view<Position, Velocity>().each([](auto e, auto& pos, auto& vel) {
        pos.x += vel.dx;
        pos.y += vel.dy;
    });
    
    // Remove dead entities
    registry.view<Health>().each([&](Entity e, Health& hp) {
        if (hp.hp <= 0) {
            registry.kill_entity(e);
        }
    });
    
    // Cleanup
    registry.cleanup_tombstones();
    
    return 0;
}
```

## See Also

- [Entity System](01_entity_system.md) - Entity lifecycle
- [Component Storage](02_component_storage.md) - Storage implementation
- [Views](04_views.md) - Query and iteration
- [Command Buffer](09_command_buffer.md) - Deferred operations
- [Signal System](07_signals.md) - Event-driven programming
