# Component Storage

## Overview

Components are stored using the **Sparse Set** data structure, which provides O(1) operations while maintaining cache-friendly iteration. The R-Type ECS uses two specialized implementations:

- **SparseSet<T>**: For regular components with data
- **TagSparseSet<T>**: For zero-size marker components (tags)

## Sparse Set Architecture

### Data Structure

```
Entities: [E5, E2, E7, E1, E9]  ← Alive entities

Component Storage:
┌─────────────────────────────┐
│  Dense Array (Components)   │  ← Contiguous, cache-friendly
├─────────────────────────────┤
│  [C5] [C2] [C7] [C1] [C9]   │
└─────────────────────────────┘

┌─────────────────────────────┐
│  Packed Array (Entity IDs)  │  ← Parallel to dense
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

1. **Insert**: Append to dense/packed, update sparse mapping
2. **Remove**: Swap-and-pop with last element, update sparse
3. **Lookup**: `dense[sparse[entity.index()]]`
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
    
    // Capacity management
    void reserve(size_t capacity);
    void shrink_to_fit();
    void clear() noexcept override;
    size_t size() const noexcept override;
    
    // Direct access (advanced)
    const std::vector<Entity>& get_packed() const noexcept;
    const std::vector<T>& get_dense() const noexcept;
};
```

### Usage Examples

```cpp
// Define component
struct Position {
    float x, y;
};

// Get sparse set from registry
auto& pos_storage = registry.get_sparse_set<Position>();

// Add component
Entity entity = registry.spawn_entity();
pos_storage.emplace(entity, 10.0f, 20.0f);

// Check existence
if (pos_storage.contains(entity)) {
    Position& pos = pos_storage.get(entity);
    pos.x += 1.0f;
}

// Iterate all positions
for (Position& pos : pos_storage) {
    pos.x *= 0.99f; // Apply friction
}

// Remove component
pos_storage.remove(entity);
```

## TagSparseSet<T>

For empty components (markers/flags). Only stores entity IDs, no component data.

### Requirements

```cpp
// Tag components must be empty types
struct Player {};
struct Enemy {};
struct Dead {};

static_assert(std::is_empty_v<Player>); // ✅ Valid tag
```

### Memory Efficiency

```cpp
struct Health { int hp; }; // sizeof(Health) = 4 bytes
struct Dead {};            // sizeof(Dead) = 1 byte

// 10,000 entities with Health: 10,000 * 4 = 40 KB
// 10,000 entities with Dead tag:
//   - Regular SparseSet: 10,000 * 1 = 10 KB
//   - TagSparseSet: 0 KB (only entity IDs stored)
```

### Usage

```cpp
// Tags are used identically to regular components
auto& player_tags = registry.get_sparse_set<Player>();

Entity entity = registry.spawn_entity();
player_tags.emplace(entity); // No data needed

if (player_tags.contains(entity)) {
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
    Position& pos = registry.get_component<Position>(entity);
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
registry.reserve_components<Position>(10000);
registry.reserve_components<Velocity>(10000);
```

### Compaction

Reclaim unused memory after bulk deletions:

```cpp
// Compact specific component
registry.compact_component<Position>();

// Compact all components
registry.compact();
```

### Clearing

```cpp
// Remove all components of a type
registry.get_sparse_set<Position>().clear();
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

// ✅ Safe: Use parallel_view for this
registry.parallel_view<Position>().each([](auto e, auto& p) {
    p.x++;
    p.y++;
});
```

### Locking

SparseSet operations are internally locked:

```cpp
// Thread-safe individual operations
auto& set = registry.get_sparse_set<Position>();
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

```cpp
auto& pos_set = registry.get_sparse_set<Position>();
const auto& positions = pos_set.get_dense();
const auto& entities = pos_set.get_packed();

for (size_t i = 0; i < positions.size(); ++i) {
    Entity e = entities[i];
    Position& pos = positions[i];
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
    static constexpr bool is_empty = std::is_empty_v<T>;
    static constexpr bool is_trivial = std::is_trivially_copyable_v<T>;
    static constexpr bool is_trivially_destructible = std::is_trivially_destructible_v<T>;
};

// Automatically uses TagSparseSet for empty types
// Automatically optimizes trivial types
```

## Best Practices

### ✅ Do

- Keep components small and cache-friendly (POD types preferred)
- Use tags for boolean flags (no data overhead)
- Reserve capacity when you know entity counts
- Use views for iteration (not direct sparse set access)
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
