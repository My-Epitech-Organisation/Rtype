# Parallel Processing

## Overview

**ParallelView** enables multi-threaded component iteration for performance on large datasets. It automatically distributes work across CPU cores while maintaining thread safety.

## Basic Usage

```cpp
#include "ecs/src/ECS.hpp"
#include <thread>

// Parallel iteration over components
registry.parallelView<Position, Velocity>().each([](Entity e, Position& pos, Velocity& vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
});

// The work is automatically distributed across available CPU cores
```

## Thread Safety Guarantees

### ✅ Safe Operations

1. **Concurrent reads** of the same component
2. **Concurrent writes** to different components of the same entity
3. **Different entities** processed by different threads

```cpp
// SAFE: Each thread modifies different entities
registry.parallelView<Position, Velocity>().each([](Entity e, Position& pos, Velocity& vel) {
    pos.x += vel.dx; // Different entities, no race
    pos.y += vel.dy;
});

// SAFE: Different components, same entity
std::thread t1([&] { 
    registry.parallelView<Position>().each([](auto e, auto& p) { p.x += 1.0f; }); 
});
std::thread t2([&] { 
    registry.parallelView<Velocity>().each([](auto e, auto& v) { v.dx *= 0.99f; }); 
});
```

### ❌ Unsafe Operations

1. **Adding/removing entities** during iteration
2. **Adding/removing components** during iteration
3. **Modifying shared state** without synchronization

```cpp
// UNSAFE: Adding components during parallel iteration
registry.parallelView<Position>().each([&](Entity e, Position& pos) {
    if (pos.x > 100.0f) {
        registry.emplaceComponent<OutOfBounds>(e); // ❌ RACE CONDITION
    }
});

// SAFE: Use CommandBuffer instead
CommandBuffer cmd(registry);
registry.parallelView<Position>().each([&](Entity e, Position& pos) {
    if (pos.x > 100.0f) {
        cmd.emplaceComponentDeferred<OutOfBounds>(e); // ✅ Safe
    }
});
cmd.flush();
```

## Performance

### Speedup Characteristics

Near-linear speedup for CPU-bound operations:

```
Cores    Speedup    Efficiency
  1       1.0×       100%
  2       1.9×        95%
  4       3.7×        93%
  8       7.2×        90%
 16      14.5×        91%
```

### When to Use Parallel Views

✅ **Use when:**
- Processing > 10,000 entities
- Operations are CPU-bound (math, physics)
- Work per entity is substantial (> 100 instructions)

❌ **Don't use when:**
- Few entities (< 1,000)
- Operations are memory-bound (cache thrashing)
- Work per entity is trivial (< 50 instructions)

### Overhead

Parallel views have fixed overhead:
- Thread creation: ~0.1ms per thread
- Work distribution: ~0.01ms per chunk

For small datasets, this overhead can exceed the benefits.

## Advanced Usage

### Shared State with Synchronization

```cpp
// Accumulation with atomic operations
std::atomic<int> total_health{0};

registry.parallelView<Health>().each([&](Entity e, const Health& hp) {
    total_health.fetch_add(hp.hp, std::memory_order_relaxed);
});

// Or use mutex for complex operations
std::mutex results_mutex;
std::vector<Entity> low_health_entities;

registry.parallelView<Health>().each([&](Entity e, const Health& hp) {
    if (hp.hp < 20) {
        std::lock_guard lock(results_mutex);
        low_health_entities.push_back(e);
    }
});
```

### Thread-Local Storage

```cpp
// Each thread maintains its own accumulator
thread_local std::vector<Entity> local_results;

registry.parallelView<Position>().each([&](Entity e, Position& pos) {
    if (pos.x > 100.0f) {
        local_results.push_back(e); // Thread-local, no lock needed
    }
});
```

### Custom Thread Count

ParallelView uses `std::thread::hardware_concurrency()` by default, but you can control this:

```cpp
// Limit to 4 threads for resource management
unsigned int thread_count = std::min(4u, std::thread::hardware_concurrency());
```

## Comparison with Regular Views

```cpp
// Single-threaded (View)
auto start = std::chrono::high_resolution_clock::now();
registry.view<Position, Velocity>().each([](auto e, auto& pos, auto& vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
});
auto end = std::chrono::high_resolution_clock::now();
// Time: 10ms for 1M entities

// Multi-threaded (ParallelView)
auto start = std::chrono::high_resolution_clock::now();
registry.parallelView<Position, Velocity>().each([](auto e, auto& pos, auto& vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
});
auto end = std::chrono::high_resolution_clock::now();
// Time: 1.5ms for 1M entities (8 cores)
```

## Common Patterns

### Physics Update

```cpp
void physics_update(Registry& registry, float dt) {
    // Parallel position integration
    registry.parallelView<Position, Velocity>().each([dt](auto e, auto& pos, auto& vel) {
        pos.x += vel.dx * dt;
        pos.y += vel.dy * dt;
    });
    
    // Parallel velocity damping
    registry.parallelView<Velocity>().each([](auto e, auto& vel) {
        vel.dx *= 0.99f;
        vel.dy *= 0.99f;
    });
}
```

### Particle Systems

```cpp
void update_particles(Registry& registry, float dt) {
    // Update thousands of particles in parallel
    registry.parallelView<Particle>().each([dt](Entity e, Particle& p) {
        p.position += p.velocity * dt;
        p.velocity += p.acceleration * dt;
        p.lifetime -= dt;
        p.color.a *= 0.99f; // Fade out
    });
}
```

### Spatial Hashing

```cpp
void build_spatial_hash(Registry& registry, SpatialHash& hash) {
    // Clear hash (not thread-safe)
    hash.clear();
    
    // Parallel insertion with per-thread buckets
    std::mutex hash_mutex;
    
    registry.parallelView<Position>().each([&](Entity e, const Position& pos) {
        int cell_x = static_cast<int>(pos.x / 10.0f);
        int cell_y = static_cast<int>(pos.y / 10.0f);
        
        std::lock_guard lock(hash_mutex);
        hash.insert(cell_x, cell_y, e);
    });
}
```

### Batch Processing

```cpp
void process_ai(Registry& registry) {
    // Process AI in parallel (each entity independent)
    registry.parallelView<AI, Position>().each([&](Entity e, AI& ai, Position& pos) {
        // Pathfinding, decision making, etc.
        ai.state_machine.update();
        ai.navigate_to(ai.target);
    });
}
```

## Deferred Operations with CommandBuffer

```cpp
#include "ECS/Core/CommandBuffer.hpp"

void update_with_structural_changes(Registry& registry) {
    CommandBuffer cmd(registry);
    
    // Parallel iteration with deferred changes
    registry.parallelView<Health>().each([&](Entity e, Health& hp) {
        hp.hp -= 10; // Safe: modifying component
        
        if (hp.hp <= 0) {
            // Defer structural changes
            cmd.emplaceComponentDeferred<Dead>(e);
            cmd.removeComponentDeferred<AI>(e);
        }
    });
    
    // Apply all deferred changes (single-threaded)
    cmd.flush();
    
    // Cleanup dead entities
    registry.view<Dead>().each([&](Entity e) {
        registry.killEntity(e);
    });
}
```

## Performance Profiling

```cpp
#include "ECS/Utils/Benchmark.hpp"

void profile_parallel_performance(Registry& registry) {
    Benchmark bench;
    
    // Single-threaded
    bench.measure("View", [&]() {
        registry.view<Position, Velocity>().each([](auto e, auto& p, auto& v) {
            p.x += v.dx;
            p.y += v.dy;
        });
    }, 100);
    
    // Multi-threaded
    bench.measure("ParallelView", [&]() {
        registry.parallelView<Position, Velocity>().each([](auto e, auto& p, auto& v) {
            p.x += v.dx;
            p.y += v.dy;
        });
    }, 100);
    
    bench.print_results();
}
```

## Best Practices

### ✅ Do

- Use for large datasets (> 10,000 entities)
- Profile to verify speedup
- Minimize shared state access
- Use atomics for simple aggregations
- Use CommandBuffer for structural changes
- Keep operations per entity substantial

### ❌ Don't

- Don't add/remove entities during iteration
- Don't add/remove components during iteration
- Don't use for trivial operations
- Don't access shared state without locks
- Don't over-parallelize (diminishing returns)
- Don't use with < 1,000 entities

## Troubleshooting

### No Performance Gain

**Possible causes:**
1. Dataset too small (overhead dominates)
2. Operations memory-bound (cache thrashing)
3. Too much synchronization (lock contention)
4. Work per entity too trivial

**Solutions:**
- Profile with Benchmark utility
- Increase batch size
- Reduce shared state access
- Optimize cache usage

### Data Races

**Symptoms:**
- Random crashes
- Corrupted data
- Non-deterministic results

**Solutions:**
- Never modify shared state without locks
- Use CommandBuffer for structural changes
- Use atomic types for counters
- Validate with thread sanitizer

### Poor Scaling

**Causes:**
- False sharing (adjacent data on same cache line)
- Lock contention
- Uneven work distribution

**Solutions:**
- Align data structures to cache lines
- Minimize lock scope
- Ensure uniform work per entity

## Platform Considerations

### Thread Count

```cpp
// Detect available cores
unsigned int cores = std::thread::hardware_concurrency();
// Typical values: 4-16 for consumer CPUs, 32-128 for server CPUs
```

### NUMA Awareness

On NUMA systems, consider thread affinity:

```cpp
// Note: ParallelView doesn't currently support thread affinity
// For NUMA optimization, implement custom parallel iteration
```

## See Also

- [Views](04_views.md) - Single-threaded iteration
- [Command Buffer](09_command_buffer.md) - Deferred operations
- [Benchmarking](13_benchmarking.md) - Performance measurement
- [Optimization Guide](14_optimization.md) - Performance tips
