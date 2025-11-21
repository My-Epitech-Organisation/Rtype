# Command Buffer

## Overview

The **CommandBuffer** provides thread-safe deferred ECS operations. It records structural changes (entity/component add/remove) during iteration and applies them later in a single batch, preventing invalidation issues.

## Why Use CommandBuffer?

### The Problem

```cpp
// ❌ UNSAFE: Modifying structure during iteration
registry.view<Health>().each([&](Entity e, Health& hp) {
    if (hp.hp <= 0) {
        registry.kill_entity(e); // ❌ Invalidates iterator!
    }
});
```

### The Solution

```cpp
// ✅ SAFE: Defer operations
CommandBuffer cmd(registry);

registry.view<Health>().each([&](Entity e, Health& hp) {
    if (hp.hp <= 0) {
        cmd.destroy_entity_deferred(e); // ✅ Safe - recorded, not executed
    }
});

cmd.flush(); // Apply all deferred operations
```

## API Reference

### Creation

```cpp
#include "ECS/Core/CommandBuffer.hpp"

CommandBuffer cmd(registry);
```

### Entity Operations

```cpp
// Spawn entity (returns placeholder)
Entity placeholder = cmd.spawn_entity_deferred();

// Destroy entity
cmd.destroy_entity_deferred(entity);
```

### Component Operations

```cpp
// Add component
cmd.emplace_component_deferred<Position>(entity, 0.0f, 0.0f);

// Remove component
cmd.remove_component_deferred<Velocity>(entity);
```

### Execution

```cpp
// Apply all deferred operations
cmd.flush();

// Get pending command count
size_t pending = cmd.pending_count();

// Clear without executing
cmd.clear();
```

## Basic Usage

### Deferred Entity Destruction

```cpp
CommandBuffer cmd(registry);

// Mark dead entities
registry.view<Health>().each([&](Entity e, Health& hp) {
    if (hp.hp <= 0) {
        cmd.destroy_entity_deferred(e);
    }
});

// Remove all dead entities at once
cmd.flush();
```

### Deferred Component Addition

```cpp
CommandBuffer cmd(registry);

// Add components based on condition
registry.view<Position>().each([&](Entity e, const Position& pos) {
    if (pos.x > 100.0f) {
        cmd.emplace_component_deferred<OutOfBounds>(e);
    }
});

cmd.flush();
```

### Deferred Component Removal

```cpp
CommandBuffer cmd(registry);

// Remove components from entities
registry.view<Temporary, Lifetime>().each([&](Entity e, auto& temp, Lifetime& life) {
    life.remaining -= 0.016f;
    if (life.remaining <= 0.0f) {
        cmd.remove_component_deferred<Temporary>(e);
    }
});

cmd.flush();
```

## Thread Safety

### ParallelView Integration

CommandBuffer is thread-safe, making it perfect for parallel iteration:

```cpp
CommandBuffer cmd(registry);

// Parallel iteration with deferred changes
registry.parallel_view<Position, Health>().each([&](Entity e, Position& pos, Health& hp) {
    // Each thread can safely record operations
    if (hp.hp <= 0) {
        cmd.destroy_entity_deferred(e); // Thread-safe
    }
});

// Flush from main thread
cmd.flush();
```

### Locking

Internal mutex protects command queue:

```cpp
// Multiple threads can record commands concurrently
std::thread t1([&] {
    cmd.emplace_component_deferred<ComponentA>(e1);
});

std::thread t2([&] {
    cmd.emplace_component_deferred<ComponentB>(e2);
});

t1.join();
t2.join();
cmd.flush(); // Apply all commands
```

## Entity Placeholders

When spawning entities deferred, you get a placeholder:

```cpp
CommandBuffer cmd(registry);

// Spawn returns placeholder (not real entity yet)
Entity placeholder = cmd.spawn_entity_deferred();

// Can use placeholder in other deferred operations
cmd.emplace_component_deferred<Position>(placeholder, 0.0f, 0.0f);
cmd.emplace_component_deferred<Velocity>(placeholder, 1.0f, 0.0f);

// Flush creates real entity and applies components
cmd.flush();

// Placeholder is now mapped to real entity internally
```

## Advanced Patterns

### Two-Phase Processing

```cpp
void update(Registry& registry) {
    CommandBuffer cmd(registry);
    
    // Phase 1: Mark entities for changes
    registry.view<Health>().each([&](Entity e, Health& hp) {
        if (hp.hp <= 0) {
            cmd.destroy_entity_deferred(e);
        } else if (hp.hp > 100) {
            cmd.emplace_component_deferred<Overheal>(e);
        }
    });
    
    // Phase 2: Apply all changes
    cmd.flush();
}
```

### Batch Entity Spawning

```cpp
void spawn_enemies(Registry& registry, int count) {
    CommandBuffer cmd(registry);
    
    for (int i = 0; i < count; ++i) {
        Entity e = cmd.spawn_entity_deferred();
        cmd.emplace_component_deferred<Position>(e, i * 10.0f, 0.0f);
        cmd.emplace_component_deferred<Enemy>(e);
        cmd.emplace_component_deferred<Health>(e, 50);
    }
    
    // Create all enemies at once
    cmd.flush();
}
```

### Conditional Component Management

```cpp
void update_states(Registry& registry) {
    CommandBuffer cmd(registry);
    
    registry.view<State>().each([&](Entity e, State& state) {
        switch (state.current) {
            case State::Idle:
                cmd.remove_component_deferred<Moving>(e);
                cmd.emplace_component_deferred<Stationary>(e);
                break;
                
            case State::Moving:
                cmd.remove_component_deferred<Stationary>(e);
                cmd.emplace_component_deferred<Moving>(e);
                break;
                
            case State::Dead:
                cmd.destroy_entity_deferred(e);
                break;
        }
    });
    
    cmd.flush();
}
```

### Hierarchical Entity Creation

```cpp
void create_hierarchy(Registry& registry) {
    CommandBuffer cmd(registry);
    
    // Create parent
    Entity parent = cmd.spawn_entity_deferred();
    cmd.emplace_component_deferred<Transform>(parent);
    cmd.emplace_component_deferred<Parent>(parent);
    
    // Create children
    for (int i = 0; i < 5; ++i) {
        Entity child = cmd.spawn_entity_deferred();
        cmd.emplace_component_deferred<Transform>(child);
        cmd.emplace_component_deferred<Child>(child, parent);
    }
    
    // Create all at once
    cmd.flush();
}
```

## Performance

### Batching Benefits

```cpp
// Without CommandBuffer: many small operations
for (int i = 0; i < 1000; ++i) {
    auto e = registry.spawn_entity();          // Lock acquired 1000 times
    registry.emplace_component<Position>(e);   // Lock acquired 1000 times
}
// Time: ~5ms

// With CommandBuffer: single batch
CommandBuffer cmd(registry);
for (int i = 0; i < 1000; ++i) {
    auto e = cmd.spawn_entity_deferred();
    cmd.emplace_component_deferred<Position>(e);
}
cmd.flush(); // Lock acquired once
// Time: ~1ms (5× faster)
```

### Memory Overhead

CommandBuffer stores commands as `std::function` objects:

```cpp
// Approximate memory per command
spawn_entity_deferred()           // ~64 bytes
destroy_entity_deferred()         // ~64 bytes
emplace_component_deferred<T>()   // ~64 bytes + sizeof(T)
remove_component_deferred<T>()    // ~64 bytes
```

For 10,000 commands: ~640 KB memory

## Best Practices

### ✅ Do

- Use with parallel iteration
- Batch many operations together
- Flush after iteration completes
- Use for structural changes during iteration
- Clear after flush if reusing

### ❌ Don't

- Don't flush during iteration (defeats purpose)
- Don't expect immediate effect (operations are deferred)
- Don't query deferred changes before flush
- Don't forget to flush (commands won't execute)
- Don't reuse placeholders after flush

## Common Patterns

### Game Loop Integration

```cpp
void game_update(Registry& registry, float dt) {
    CommandBuffer cmd(registry);
    
    // Systems that need deferred operations
    update_lifetime(registry, cmd, dt);
    update_health(registry, cmd);
    update_spawners(registry, cmd, dt);
    
    // Apply all deferred changes
    cmd.flush();
    
    // Systems that need stable structure
    update_physics(registry, dt);
    update_rendering(registry);
}
```

### Persistent CommandBuffer

```cpp
class GameWorld {
    Registry& registry;
    CommandBuffer cmd;
    
public:
    GameWorld(Registry& reg) : registry(reg), cmd(reg) {}
    
    void update() {
        // Record operations
        update_entities(cmd);
        
        // Flush and clear for next frame
        cmd.flush();
        cmd.clear();
    }
};
```

### Conditional Flushing

```cpp
CommandBuffer cmd(registry);

registry.view<Position>().each([&](Entity e, Position& pos) {
    if (pos.x > 1000.0f) {
        cmd.destroy_entity_deferred(e);
    }
});

// Only flush if there are pending commands
if (cmd.pending_count() > 0) {
    std::cout << "Applying " << cmd.pending_count() << " deferred operations\n";
    cmd.flush();
}
```

## Error Handling

```cpp
CommandBuffer cmd(registry);

// Operations are recorded without validation
cmd.destroy_entity_deferred(invalid_entity); // Recorded successfully

try {
    cmd.flush(); // May throw if operations are invalid
} catch (const std::runtime_error& e) {
    std::cerr << "CommandBuffer flush failed: " << e.what() << "\n";
    cmd.clear(); // Clear invalid commands
}
```

## Debugging

```cpp
CommandBuffer cmd(registry);

// Track operations
cmd.emplace_component_deferred<Position>(e1);
cmd.remove_component_deferred<Velocity>(e2);
cmd.destroy_entity_deferred(e3);

std::cout << "Pending commands: " << cmd.pending_count() << "\n";
// Output: Pending commands: 3

cmd.flush();
std::cout << "Pending commands: " << cmd.pending_count() << "\n";
// Output: Pending commands: 0
```

## Comparison with Immediate Operations

| Aspect | Immediate | Deferred (CommandBuffer) |
|--------|-----------|-------------------------|
| Thread safety | Limited | Full |
| During iteration | ❌ Unsafe | ✅ Safe |
| Performance | Good | Better (batched) |
| Memory | None | O(commands) |
| Complexity | Simple | Moderate |
| Debugging | Easy | Harder |

## See Also

- [Parallel Processing](05_parallel_processing.md) - Multi-threaded iteration
- [Registry](03_registry.md) - Entity/component operations
- [Views](04_views.md) - Component queries
