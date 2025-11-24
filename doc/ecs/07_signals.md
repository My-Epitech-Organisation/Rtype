# Signal System

## Overview

The **Signal System** provides an event-driven mechanism for responding to component lifecycle events. It enables reactive programming patterns within the ECS through callbacks triggered on component addition and removal.

## Core Concepts

### Event Types

- **onConstruct**: Triggered when a component is added to an entity
- **onDestroy**: Triggered when a component is removed from an entity

### Use Cases

- Physics body initialization
- Resource cleanup (audio, graphics handles)
- Relationship management (parent-child updates)
- Debugging and logging
- Statistics tracking
- Cache invalidation

## Basic Usage

```cpp
#include "ECS/ECS.hpp"

struct Health { int hp; };

// Register construct callback
registry.onConstruct<Health>([](Entity e) {
    std::cout << "Entity " << e.index() << " gained Health\n";
});

// Register destroy callback
registry.onDestroy<Health>([](Entity e) {
    std::cout << "Entity " << e.index() << " lost Health\n";
});

// Signals are automatically dispatched
auto entity = registry.spawnEntity();
registry.emplaceComponent<Health>(entity, 100); // Triggers onConstruct
registry.removeComponent<Health>(entity);        // Triggers onDestroy
```

## API Reference

### Registration

```cpp
template<typename T>
void onConstruct(std::function<void(Entity)> callback);

template<typename T>
void onDestroy(std::function<void(Entity)> callback);
```

### Callback Management

```cpp
// Clear callbacks for specific component type
registry._signalDispatcher.clearCallbacks(std::type_index(typeid(Health)));

// Clear all callbacks
registry._signalDispatcher.clearAllCallbacks();
```

## Advanced Patterns

### Resource Initialization

```cpp
struct RenderHandle {
    unsigned int texture_id = 0;
    unsigned int vbo_id = 0;
};

// Initialize graphics resources on component add
registry.onConstruct<RenderHandle>([&](Entity e) {
    auto& handle = registry.getComponent<RenderHandle>(e);
    handle.texture_id = create_texture();
    handle.vbo_id = create_vbo();
});

// Cleanup graphics resources on component remove
registry.onDestroy<RenderHandle>([&](Entity e) {
    auto& handle = registry.getComponent<RenderHandle>(e);
    destroy_texture(handle.texture_id);
    destroy_vbo(handle.vbo_id);
});
```

### Relationship Management

```cpp
struct Parent {
    Entity parent_entity;
};

registry.onConstruct<Parent>([&](Entity child) {
    auto& parent_comp = registry.getComponent<Parent>(child);
    // Register child with parent
    if (registry.isAlive(parent_comp.parent_entity)) {
        auto& children = registry.getComponent<Children>(parent_comp.parent_entity);
        children.list.push_back(child);
    }
});

registry.onDestroy<Parent>([&](Entity child) {
    auto& parent_comp = registry.getComponent<Parent>(child);
    // Unregister child from parent
    if (registry.isAlive(parent_comp.parent_entity)) {
        auto& children = registry.getComponent<Children>(parent_comp.parent_entity);
        auto it = std::find(children.list.begin(), children.list.end(), child);
        if (it != children.list.end()) {
            children.list.erase(it);
        }
    }
});
```

### State Validation

```cpp
struct Velocity { float dx, dy; };
struct Stationary {}; // Tag

// Ensure entities can't have both Velocity and Stationary
registry.onConstruct<Velocity>([&](Entity e) {
    if (registry.hasComponent<Stationary>(e)) {
        registry.removeComponent<Stationary>(e);
        std::cout << "Warning: Removed Stationary tag from moving entity\n";
    }
});

registry.onConstruct<Stationary>([&](Entity e) {
    if (registry.hasComponent<Velocity>(e)) {
        registry.removeComponent<Velocity>(e);
        std::cout << "Warning: Removed Velocity from stationary entity\n";
    }
});
```

### Debugging and Profiling

```cpp
#include <chrono>

struct ComponentStats {
    size_t create_count = 0;
    size_t destroy_count = 0;
    std::chrono::steady_clock::time_point last_create;
} stats;

registry.onConstruct<Position>([&](Entity e) {
    stats.create_count++;
    stats.last_create = std::chrono::steady_clock::now();
});

registry.onDestroy<Position>([&](Entity e) {
    stats.destroy_count++;
});

// Print stats periodically
void print_stats() {
    std::cout << "Position components - Created: " << stats.create_count 
              << ", Destroyed: " << stats.destroy_count 
              << ", Active: " << (stats.create_count - stats.destroy_count) << "\n";
}
```

### Cache Invalidation

```cpp
struct CachedData {
    std::vector<Entity> cached_entities;
    bool dirty = false;
};

CachedData cache;

// Invalidate cache when entities change
registry.onConstruct<CacheableComponent>([&](Entity e) {
    cache.dirty = true;
});

registry.onDestroy<CacheableComponent>([&](Entity e) {
    cache.dirty = true;
});

// Rebuild cache when needed
void update_cache() {
    if (cache.dirty) {
        cache.cached_entities.clear();
        registry.view<CacheableComponent>().each([&](Entity e, auto& comp) {
            cache.cached_entities.push_back(e);
        });
        cache.dirty = false;
    }
}
```

## Multiple Callbacks

You can register multiple callbacks for the same component type:

```cpp
// Register multiple construct callbacks
registry.onConstruct<Health>([](Entity e) {
    std::cout << "Callback 1: Health added\n";
});

registry.onConstruct<Health>([](Entity e) {
    std::cout << "Callback 2: Health added\n";
});

// Both callbacks are invoked when component is added
auto entity = registry.spawnEntity();
registry.emplaceComponent<Health>(entity, 100);
// Output:
// Callback 1: Health added
// Callback 2: Health added
```

## Thread Safety

### Guarantees

- ✅ Registering callbacks is thread-safe
- ✅ Dispatching events is thread-safe
- ✅ Multiple threads can register callbacks concurrently
- ⚠️ Callbacks execute WITHOUT locks held (prevent deadlocks)

### Reentrant Dispatch

Callbacks can trigger other events safely:

```cpp
registry.onConstruct<ComponentA>([&](Entity e) {
    // This callback adds ComponentB, triggering onConstruct<ComponentB>
    registry.emplaceComponent<ComponentB>(e);
});

registry.onConstruct<ComponentB>([](Entity e) {
    std::cout << "ComponentB added (possibly from ComponentA callback)\n";
});

// Reentrant dispatch is safe
auto entity = registry.spawnEntity();
registry.emplaceComponent<ComponentA>(entity);
```

### Deadlock Prevention

The signal dispatcher copies callbacks before execution to avoid holding locks:

```cpp
// Implementation detail (for understanding):
void SignalDispatcher::dispatchConstruct(std::type_index type, Entity entity) {
    std::vector<Callback> callbacks_copy;
    {
        std::shared_lock lock(callbacks_mutex);
        // Copy callbacks while holding lock
        if (_constructCallbacks.contains(type)) {
            callbacks_copy = _constructCallbacks[type];
        }
    } // Release lock
    
    // Execute without holding lock (prevents deadlocks)
    for (const auto& callback : callbacks_copy) {
        callback(entity);
    }
}
```

## Performance Considerations

### Callback Cost

- Callback registration: O(1) append to vector
- Event dispatch: O(k) where k = number of registered callbacks
- Memory: O(k) per component type

### When Signals Are Dispatched

```cpp
// Dispatched ONLY when component is newly added
registry.emplaceComponent<Health>(entity, 100); // ✅ Dispatches onConstruct

// NOT dispatched when replacing existing component
registry.emplaceComponent<Health>(entity, 100); // Initial
registry.emplaceComponent<Health>(entity, 50);  // ❌ No signal (replacement)
```

### Optimization Tips

1. Keep callbacks lightweight
2. Avoid expensive operations in callbacks
3. Defer heavy work to systems
4. Use callbacks for notifications only

```cpp
// ❌ Bad: Heavy work in callback
registry.onConstruct<Sprite>([&](Entity e) {
    load_texture_from_disk(); // Expensive!
    parse_animation_data();
    build_mesh();
});

// ✅ Good: Mark for deferred processing
std::vector<Entity> needs_initialization;

registry.onConstruct<Sprite>([&](Entity e) {
    needs_initialization.push_back(e); // Fast notification
});

// Process in dedicated system
void init_sprites() {
    for (Entity e : needs_initialization) {
        // Do expensive work here
        load_texture_from_disk();
    }
    needs_initialization.clear();
}
```

## Common Patterns

### Automatic Component Pairing

```cpp
// Automatically add Health when Player tag is added
registry.onConstruct<Player>([&](Entity e) {
    if (!registry.hasComponent<Health>(e)) {
        registry.emplaceComponent<Health>(e, 100);
    }
});
```

### Event Logging

```cpp
struct Logger {
    void log(const std::string& msg) {
        std::cout << "[" << timestamp() << "] " << msg << "\n";
    }
};

Logger logger;

registry.onConstruct<Enemy>([&](Entity e) {
    logger.log("Enemy spawned: " + std::to_string(e.index()));
});

registry.onDestroy<Enemy>([&](Entity e) {
    logger.log("Enemy destroyed: " + std::to_string(e.index()));
});
```

### Reference Counting

```cpp
struct ResourceHandle {
    int resource_id;
};

std::unordered_map<int, int> resource_refcount;

registry.onConstruct<ResourceHandle>([&](Entity e) {
    auto& handle = registry.getComponent<ResourceHandle>(e);
    resource_refcount[handle.resource_id]++;
});

registry.onDestroy<ResourceHandle>([&](Entity e) {
    auto& handle = registry.getComponent<ResourceHandle>(e);
    if (--resource_refcount[handle.resource_id] == 0) {
        // Last reference removed, free resource
        free_resource(handle.resource_id);
    }
});
```

### Component Dependencies

```cpp
// Ensure Transform exists when Sprite is added
registry.onConstruct<Sprite>([&](Entity e) {
    if (!registry.hasComponent<Transform>(e)) {
        registry.emplaceComponent<Transform>(e);
    }
});
```

## Best Practices

### ✅ Do

- Use for resource initialization/cleanup
- Keep callbacks fast and simple
- Use for notifications and logging
- Handle reentrant callbacks gracefully
- Use for maintaining invariants

### ❌ Don't

- Don't perform expensive operations in callbacks
- Don't assume callback execution order
- Don't create infinite callback loops
- Don't access deleted entities in destroy callbacks
- Don't throw exceptions from callbacks (catch internally)

## Error Handling

```cpp
registry.onConstruct<Component>([&](Entity e) {
    try {
        // Risky operation
        initialize_resource(e);
    } catch (const std::exception& ex) {
        std::cerr << "Error in onConstruct: " << ex.what() << "\n";
        // Handle error gracefully
    }
});
```

## Testing

```cpp
void test_signals() {
    ECS::Registry registry;
    
    bool construct_called = false;
    bool destroy_called = false;
    
    registry.onConstruct<Health>([&](Entity e) {
        construct_called = true;
    });
    
    registry.onDestroy<Health>([&](Entity e) {
        destroy_called = true;
    });
    
    auto entity = registry.spawnEntity();
    registry.emplaceComponent<Health>(entity, 100);
    assert(construct_called);
    
    registry.removeComponent<Health>(entity);
    assert(destroy_called);
}
```

## See Also

- [Registry](03_registry.md) - Component management
- [System Scheduler](08_system_scheduler.md) - System coordination
- [Relationships](10_relationships.md) - Entity hierarchies
