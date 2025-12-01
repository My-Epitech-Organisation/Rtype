# Component Storage

## Overview

Components are stored using the **Sparse Set** data structure, which provides O(1) operations while maintaining cache-friendly iteration.

- **SparseSet<T>**: For all components (with or without data)

## Sparse Set Architecture

### Data Structure

```
Entities: [E5, E2, E7, E1, E9]  ← Alive entities

Component Storage:
┌─────────────────────────────┐
│  dense Array (Components)   │  ← Contiguous, cache-friendly
├─────────────────────────────┤
│  [C5] [C2] [C7] [C1] [C9]   │
└─────────────────────────────┘

┌─────────────────────────────┐
│  _packed Array (Entity IDs) │  ← Parallel to dense
├─────────────────────────────┤
│  [E5] [E2] [E7] [E1] [E9]   │
└─────────────────────────────┘

┌───────────────────────────────────┐
│  Sparse Array (Index Lookup)      │  ← Maps entity index to dense index
├───────────────────────────────────┤
│  [1: 3] [2: 1] [5: 0] [7: 2] ...  │
└───────────────────────────────────┘
```

### How It Works

1. **Insert**: Append to dense/_packed, update Sparse mapping
2. **Remove**: Swap-and-pop with last element, update Sparse
3. **Lookup**: `dense[Sparse[entity.index()]]`
4. **Iteration**: Linear scan over dense array (optimal cache usage)

## SparseSet<T>

For components with data (non-empty types).

### API

```cpp
template<typename T>
class SparseSet : public ISparseSet {
public:
    // Add or update component
    template<typename... Args>
    T& emplace(Entity entity, Args&&... args);

    // Remove component
    void remove(Entity entity) override;

    // Check if entity has component
    bool contains(Entity entity) const noexcept override;

    // Get component reference
    T& get(Entity entity);
    const T& get(Entity entity) const;

    // Iteration
    iterator begin() noexcept;
    iterator end() noexcept;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;

    // Capacity management
    void reserve(size_t capacity);
    void shrinkToFit();
    void clear() noexcept override;
    size_t size() const noexcept override;

    // Direct access (advanced)
    const std::vector<Entity>& getPacked() const noexcept;
    const std::vector<T>& getDense() const noexcept;
};
```

### Usage Examples

```cpp
// Define component
struct Position {
    float x, y;
};

// Use registry methods (recommended)
Entity entity = registry.spawnEntity();
registry.emplaceComponent<Position>(entity, 10.0f, 20.0f);

// Check existence
if (registry.hasComponent<Position>(entity)) {
    Position& pos = registry.getComponent<Position>(entity);
    pos.x += 1.0f;
}

// Iterate all positions using views
registry.view<Position>().each([](Entity e, Position& pos) {
    pos.x *= 0.99f; // Apply friction
});

// Remove component
registry.removeComponent<Position>(entity);
```

## Tag Components

Empty components (markers/flags) can be used for tagging entities.

### Usage

```cpp
// Tag components are empty types
struct Player {};
struct Enemy {};
struct Dead {};

// Tags are used identically to regular components
Entity entity = registry.spawnEntity();
registry.emplaceComponent<Player>(entity); // No data needed

if (registry.hasComponent<Player>(entity)) {
    // Entity is tagged as Player
}
```

## Performance Characteristics

| Operation | Time | Space | Cache Behavior |
|-----------|------|-------|----------------|
| emplace | O(1)* | O(n) | Write miss |
| remove | O(1) | O(n) | Read + write |
| contains | O(1) | O(1) | Random access |
| get | O(1) | O(1) | Random access |
| iterate | O(n) | O(n) | Sequential (optimal) |

*Amortized due to vector growth

### Cache Efficiency

```cpp
// Bad: Random access pattern
for (Entity entity : entities) {
    Position& pos = registry.getComponent<Position>(entity);
    pos.x += 1.0f; // Cache miss likely
}

// Good: Sequential iteration
registry.view<Position>().each([](auto e, Position& pos) {
    pos.x += 1.0f; // Cache-friendly
});
```

## ISparseSet Interface

Base interface for type-erased operations:

```cpp
class ISparseSet {
public:
    virtual ~ISparseSet() = default;
    virtual bool contains(Entity entity) const noexcept = 0;
    virtual void remove(Entity entity) = 0;
    virtual void clear() noexcept = 0;
    virtual size_t size() const noexcept = 0;
};
```

Used internally by Registry for polymorphic storage.

## Memory Management

### Reservation

Pre-allocate capacity to avoid reallocations:

```cpp
registry.reserveComponents<Position>(10000);
registry.reserveComponents<Velocity>(10000);
```

### Compaction

Reclaim unused memory after bulk deletions:

```cpp
// Compact specific component
registry.compactComponent<Position>();

// Compact all components
registry.compact();
```

### Clearing

```cpp
// Remove all components of a type
registry.clearComponents<Position>();
```

## Thread Safety

### Concurrent Operations

```cpp
// ✅ Safe: Different component types
std::thread t1([&] { registry.view<Position>().each(update_pos); });
std::thread t2([&] { registry.view<Velocity>().each(update_vel); });

// ❌ Unsafe: Same component type writes
std::thread t1([&] { registry.view<Position>().each([](auto e, auto& p) { p.x++; }); });
std::thread t2([&] { registry.view<Position>().each([](auto e, auto& p) { p.y++; }); });

// ✅ Safe: Use parallelView for this
registry.parallelView<Position>().each([](auto e, auto& p) {
    p.x++;
    p.y++;
});
```

### Locking

SparseSet operations are internally locked:

```cpp
// Thread-safe individual operations
auto& set = registry.getSparseSet<Position>();
set.emplace(e1, 1.0f, 2.0f); // Thread-safe
set.remove(e2);               // Thread-safe

// Compound operations need external synchronization
std::lock_guard<std::mutex> lock(my_mutex);
if (set.contains(entity)) {
    set.get(entity).x += 1.0f;
}
```

## Advanced Usage

### Direct Dense Array Access

For maximum performance, iterate the dense array directly:
> **Note:** To access the dense array of components, use the `getDense()` method of the SparseSet. This returns a `const std::vector<Position>&` (or similar container), allowing direct indexed access. Ensure you use the correct method name as implemented in your codebase (e.g., `getDense()` or `getPacked()`).

```cpp
auto& pos_set = registry.getSparseSet<Position>();
const auto& positions = pos_set.getDense(); // Returns reference to dense array of Position
const auto& entities = pos_set.getPacked();  // Returns reference to packed entity array
for (size_t i = 0; i < positions.size(); ++i) {
    Entity e = entities[i];
    Position& pos = positions[i]; // Access Position via dense array reference
    // Process...
}
```

### Custom Component Requirements

```cpp
template<typename T>
concept Component = std::is_move_constructible_v<T>;

// ✅ Valid components
struct Position { float x, y; };
struct Velocity { float dx, dy; };

// ❌ Invalid (not move-constructible)
struct Invalid {
    Invalid(const Invalid&) = delete;
    Invalid(Invalid&&) = delete;
};
```

## Component Traits

Compile-time optimization based on component properties:

```cpp
template<typename T>
struct ComponentTraits {
    static constexpr bool isEmpty = std::is_empty_v<T>;
    static constexpr bool isTrivial = std::is_trivially_copyable_v<T>;
    static constexpr bool isTrivialDestructible = std::is_trivially_destructible_v<T>;
};
```

## Best Practices

### ✅ Do

- Keep components small and cache-friendly (POD types preferred)
- Use tags for boolean flags (no data overhead)
- Reserve capacity when you know entity counts
- Use views for iteration (not direct Sparse set access)
- Compact after bulk deletions

### ❌ Don't

- Store large data structures directly in components (use handles/IDs)
- Perform random access in hot loops
- Add/remove components during parallel iteration
- Assume component memory addresses are stable across operations
- Mix entity creation/destruction with component iteration

## See Also

- [Entity System](01_entity_system.md) - Entity identifiers
- [Registry](03_registry.md) - Component management
- [Views](04_views.md) - Component queries
- [Optimization Guide](14_optimization.md) - Performance tips
