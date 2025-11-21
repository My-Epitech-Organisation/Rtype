# R-Type ECS Documentation

Welcome to the R-Type Entity Component System (ECS) technical documentation.

## Table of Contents

### Core Concepts
- [Entity System](01_entity_system.md) - Entity identifiers and lifecycle
- [Component Storage](02_component_storage.md) - Sparse set architecture
- [Registry](03_registry.md) - Central ECS coordinator

### Query & Iteration
- [Views](04_views.md) - Component queries and iteration
- [Parallel Processing](05_parallel_processing.md) - Multi-threaded iteration
- [Groups](06_groups.md) - Cached entity collections

### Advanced Features
- [Signal System](07_signals.md) - Event-driven programming
- [System Scheduler](08_system_scheduler.md) - System dependency management
- [Command Buffer](09_command_buffer.md) - Deferred operations
- [Relationships](10_relationships.md) - Entity hierarchies
- [Prefabs](11_prefabs.md) - Entity templates
- [Serialization](12_serialization.md) - Save/load state

### Performance
- [Benchmarking](13_benchmarking.md) - Performance measurement
- [Optimization Guide](14_optimization.md) - Best practices

### Reference
- [API Quick Reference](15_api_reference.md)
- [Examples](16_examples.md)

## Quick Start

```cpp
#include "ECS/ECS.hpp"

// Define components
struct Position { float x, y; };
struct Velocity { float dx, dy; };

int main() {
    ECS::Registry registry;
    
    // Create entity
    auto entity = registry.spawn_entity();
    
    // Add components
    registry.emplace_component<Position>(entity, 0.0f, 0.0f);
    registry.emplace_component<Velocity>(entity, 1.0f, 0.0f);
    
    // Query and iterate
    registry.view<Position, Velocity>().each([](auto e, auto& pos, auto& vel) {
        pos.x += vel.dx;
        pos.y += vel.dy;
    });
    
    return 0;
}
```

## Architecture Overview

The R-Type ECS is a high-performance implementation featuring:

- **Entities**: Lightweight IDs with generational indices (32-bit)
- **Components**: Plain data structures stored in cache-friendly sparse sets
- **Systems**: Functions that operate on component views
- **Registry**: Central coordinator for all ECS operations

### Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| Entity creation | O(1) | Amortized constant time |
| Component add/remove | O(1) | Direct sparse set access |
| Component lookup | O(1) | Array indexing |
| View iteration | O(n) | Optimal cache utilization |
| Parallel iteration | Linear speedup | Near-perfect scaling |

## Design Philosophy

1. **Cache Efficiency**: Contiguous memory layout for fast iteration
2. **Type Safety**: Compile-time type checking for components
3. **Zero Cost Abstractions**: No runtime overhead
4. **Thread Safety**: Explicit concurrency support
5. **Flexibility**: Support for various ECS patterns

## Project Structure

```
src/ECS/
├── Core/           # Entity, Registry, CommandBuffer, Prefab, Relationship
├── Storage/        # SparseSet, TagSparseSet, ISparseSet
├── View/           # View, ParallelView, Group, ExcludeView
├── System/         # SystemScheduler
├── Signal/         # SignalDispatcher
├── Serialization/  # Serialization
├── Traits/         # ComponentTraits
└── Utils/          # Benchmark
```
