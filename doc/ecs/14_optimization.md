# Optimization Guide

## Overview

This guide provides best practices and techniques for maximizing ECS performance in the R-Type engine.

## Data-Oriented Design Principles

### 1. Keep Components Small

```cpp
// ❌ Bad: Large component with mixed data
struct Entity {
    std::string name;           // 32+ bytes
    std::vector<int> inventory; // 24+ bytes
    float position[3];          // 12 bytes
    float rotation[4];          // 16 bytes
    // Total: 84+ bytes, poor cache utilization
};

// ✅ Good: Split into focused components
struct Position { float x, y, z; };          // 12 bytes
struct Rotation { float x, y, z, w; };       // 16 bytes
struct Name { std::string_view name; };      // 16 bytes (view, not copy)
struct Inventory { uint32_t item_ids[10]; }; // 40 bytes (fixed size)
```

### 2. Use Tags for Boolean Flags

```cpp
// ❌ Bad: Boolean component wastes memory
struct Flags {
    bool is_player;
    bool is_enemy;
    bool is_dead;
};

// ✅ Good: Use empty tag components
struct Player {};  // 0 bytes (optimized as TagSparseSet)
struct Enemy {};   // 0 bytes
struct Dead {};    // 0 bytes
```

### 3. Prefer POD Types

```cpp
// ❌ Bad: Complex types with virtual functions
struct Component {
    virtual ~Component() = default;
    virtual void update() = 0;
    std::string data;
};

// ✅ Good: Plain Old Data
struct Transform {
    float x, y;
    float rotation;
    float scale;
};
```

## Memory Optimization

### Reserve Capacity

```cpp
void init_game(ECS::Registry& registry) {
    // Reserve upfront to avoid reallocations
    registry.reserve_entities(10000);
    registry.reserve_components<Position>(10000);
    registry.reserve_components<Velocity>(10000);
    registry.reserve_components<Sprite>(5000);
}
```

### Compact After Bulk Deletions

```cpp
void cleanup_level(ECS::Registry& registry) {
    // Remove all level entities
    registry.view<LevelEntity>().each([&](Entity e) {
        registry.kill_entity(e);
    });
    
    // Reclaim memory
    registry.compact();
    registry.cleanup_tombstones();
}
```

### Avoid Fragmentation

```cpp
// ❌ Bad: Create and destroy frequently
for (int i = 0; i < 1000; ++i) {
    auto e = registry.spawn_entity();
    // ... use entity
    registry.kill_entity(e); // Fragments memory
}

// ✅ Good: Reuse entities with object pool
class EntityPool {
    std::vector<Entity> inactive;
    
public:
    Entity acquire(Registry& reg) {
        if (!inactive.empty()) {
            Entity e = inactive.back();
            inactive.pop_back();
            return e;
        }
        return reg.spawn_entity();
    }
    
    void release(Entity e) {
        inactive.push_back(e);
    }
};
```

## Iteration Optimization

### Use Views Instead of Manual Iteration

```cpp
// ❌ Bad: Manual iteration
for (auto& [entity, components] : all_entities) {
    if (has_component<Position>(entity) && 
        has_component<Velocity>(entity)) {
        auto& pos = get_component<Position>(entity);
        auto& vel = get_component<Velocity>(entity);
        pos.x += vel.dx;
    }
}

// ✅ Good: View iteration
registry.view<Position, Velocity>().each([](auto e, auto& pos, auto& vel) {
    pos.x += vel.dx;
});
```

### Query Smallest Component Set First

```cpp
// 10,000 entities with Position
// 100 entities with RareComponent

// ❌ Bad: Iterates 10,000 entities
registry.view<Position, RareComponent>().each(process);

// ✅ Good: Iterates 100 entities (automatic optimization)
// Views automatically select the smallest set
```

### Use Exclude Views Efficiently

```cpp
// ✅ Good: Exclude checks only matching entities
registry.view<Position, Velocity>()
       .exclude<Dead>()
       .each(update);
// Only checks Dead component for entities with Position+Velocity
```

### Cache Repeated Queries with Groups

```cpp
// ❌ Bad: Re-filtering every frame
for (int frame = 0; frame < 1000; ++frame) {
    registry.view<Position, Velocity>().each(update); // Filters 1000 times
}

// ✅ Good: Cache with group
auto group = registry.create_group<Position, Velocity>();
group.rebuild();

for (int frame = 0; frame < 1000; ++frame) {
    group.each(update); // No filtering, just iteration
}
```

## Parallel Processing

### When to Use ParallelView

```cpp
// Use parallel processing for:
// - Large datasets (> 10,000 entities)
// - CPU-intensive operations
// - Independent entity processing

void physics_update(Registry& reg) {
    // ✅ Good: Large dataset, CPU-intensive
    reg.parallel_view<Position, Velocity, RigidBody>().each([](auto e, auto& p, auto& v, auto& rb) {
        // Complex physics calculations
        apply_forces(rb);
        integrate_velocity(v, rb);
        integrate_position(p, v);
    });
}

// Don't use parallel for:
// - Small datasets (< 1,000 entities)
// - Memory-bound operations
// - Operations with shared state

void simple_update(Registry& reg) {
    // ❌ Bad: Too small for parallelism overhead
    reg.parallel_view<Position>().each([](auto e, auto& p) {
        p.x += 1.0f;
    });
    
    // ✅ Good: Regular view sufficient
    reg.view<Position>().each([](auto e, auto& p) {
        p.x += 1.0f;
    });
}
```

### Minimize Shared State

```cpp
// ❌ Bad: Shared state requires locking
std::mutex mutex;
std::vector<Entity> results;

reg.parallel_view<Position>().each([&](Entity e, auto& pos) {
    if (pos.x > 100.0f) {
        std::lock_guard lock(mutex); // Contention!
        results.push_back(e);
    }
});

// ✅ Good: Thread-local accumulation
thread_local std::vector<Entity> local_results;

reg.parallel_view<Position>().each([&](Entity e, auto& pos) {
    if (pos.x > 100.0f) {
        local_results.push_back(e); // No lock needed
    }
});

// Use CommandBuffer for structural changes
CommandBuffer cmd(reg);
reg.parallel_view<Health>().each([&](Entity e, auto& hp) {
    if (hp.hp <= 0) {
        cmd.destroy_entity_deferred(e); // Thread-safe
    }
});
cmd.flush();
```

## Component Design

### Structure of Arrays (SoA) Benefits

The sparse set naturally provides SoA layout:

```cpp
// Components are stored contiguously
struct Position { float x, y; }; // All x values together, all y values together

// Iteration is cache-friendly
registry.view<Position>().each([](auto e, Position& pos) {
    pos.x += 1.0f; // Sequential memory access
});
```

### Align Data to Cache Lines

```cpp
// For hot components, consider alignment
struct alignas(64) HotComponent {
    float data[15]; // Fits in one cache line (64 bytes)
};

// Avoid false sharing in parallel contexts
struct alignas(64) ParallelData {
    std::atomic<int> counter;
    char padding[60]; // Prevent false sharing
};
```

### Use Fixed-Size Arrays Instead of Vectors

```cpp
// ❌ Bad: Dynamic allocation per component
struct Inventory {
    std::vector<Item> items; // Separate allocation
};

// ✅ Good: Fixed-size inline storage
struct Inventory {
    std::array<Item, 32> items; // Inline, cache-friendly
    uint8_t item_count;
};
```

## System Optimization

### Batch Operations

```cpp
// ❌ Bad: Many small operations
for (int i = 0; i < 1000; ++i) {
    auto e = registry.spawn_entity();
    registry.emplace_component<Position>(e, i * 1.0f, 0.0f);
}

// ✅ Good: Batch with CommandBuffer
CommandBuffer cmd(registry);
for (int i = 0; i < 1000; ++i) {
    auto e = cmd.spawn_entity_deferred();
    cmd.emplace_component_deferred<Position>(e, i * 1.0f, 0.0f);
}
cmd.flush(); // Single batch operation
```

### Sort by System Frequency

```cpp
// Execute systems by frequency
void game_loop() {
    // Every frame (60 FPS)
    input_system();
    render_system();
    
    // Fixed timestep (physics at 30 FPS)
    if (accumulator >= fixed_dt) {
        physics_system();
        collision_system();
        accumulator -= fixed_dt;
    }
    
    // Low frequency (AI at 10 FPS)
    if (frame_count % 6 == 0) {
        ai_system();
        pathfinding_system();
    }
}
```

### Minimize Component Checks

```cpp
// ❌ Bad: Repeated component checks
registry.view<Entity>().each([&](Entity e) {
    if (registry.has_component<Position>(e)) {
        auto& pos = registry.get_component<Position>(e);
    }
    if (registry.has_component<Velocity>(e)) {
        auto& vel = registry.get_component<Velocity>(e);
    }
});

// ✅ Good: Use views for filtering
registry.view<Position, Velocity>().each([](auto e, auto& pos, auto& vel) {
    // Components guaranteed to exist
});
```

## Profiling and Measurement

### Use Benchmark Utility

```cpp
#include "ECS/Utils/Benchmark.hpp"

void profile_systems(Registry& reg) {
    Benchmark bench;
    
    bench.measure("Physics", [&]() { physics_system(reg); }, 100);
    bench.measure("Collision", [&]() { collision_system(reg); }, 100);
    bench.measure("Render", [&]() { render_system(reg); }, 100);
    
    bench.print_results();
}
```

### Identify Hotspots

```bash
# Use profiling tools
# Linux: perf, valgrind --tool=callgrind
# Windows: Visual Studio Profiler
# Mac: Instruments

perf record -g ./your_game
perf report
```

## Common Anti-Patterns

### 1. God Components

```cpp
// ❌ Bad: One component with everything
struct GameObject {
    Position pos;
    Velocity vel;
    Health hp;
    Sprite sprite;
    AI ai;
    // ... everything
};

// ✅ Good: Separate focused components
struct Position { float x, y; };
struct Velocity { float dx, dy; };
struct Health { int hp; };
// etc.
```

### 2. Inheritance Hierarchies

```cpp
// ❌ Bad: Deep inheritance
class Entity {};
class MovableEntity : public Entity {};
class LivingEntity : public MovableEntity {};
class Player : public LivingEntity {};

// ✅ Good: Composition
Entity player = registry.spawn_entity();
registry.emplace_component<Position>(player);
registry.emplace_component<Velocity>(player);
registry.emplace_component<Health>(player);
registry.emplace_component<Player>(player);
```

### 3. Storing Pointers in Components

```cpp
// ❌ Bad: Pointers invalidated by component operations
struct Parent {
    Position* parent_pos; // DANGEROUS!
};

// ✅ Good: Store entity references
struct Parent {
    Entity parent_entity;
};

// Access when needed
auto parent = registry.get_component<Parent>(child).parent_entity;
if (registry.is_alive(parent)) {
    auto& pos = registry.get_component<Position>(parent);
}
```

## Performance Checklist

### ✅ Memory

- [ ] Reserved capacity for entities and components
- [ ] Using tags for boolean flags
- [ ] Components are small (<= 64 bytes)
- [ ] Using POD types where possible
- [ ] Compacting after bulk deletions

### ✅ Iteration

- [ ] Using views instead of manual iteration
- [ ] Using groups for repeated queries
- [ ] Using exclude views for filtering
- [ ] Minimizing component checks
- [ ] Cache-friendly access patterns

### ✅ Parallelism

- [ ] Using ParallelView for large datasets
- [ ] Minimizing shared state
- [ ] Using CommandBuffer for deferred operations
- [ ] Avoiding false sharing
- [ ] Profiled parallel vs sequential

### ✅ Systems

- [ ] Systems run at appropriate frequencies
- [ ] Batching operations where possible
- [ ] Using SystemScheduler for dependencies
- [ ] Minimal work in hot paths
- [ ] Profiled system performance

## See Also

- [Benchmarking](13_benchmarking.md) - Performance measurement
- [Parallel Processing](05_parallel_processing.md) - Multi-threading
- [Component Storage](02_component_storage.md) - Storage architecture
- [Views](04_views.md) - Query optimization
