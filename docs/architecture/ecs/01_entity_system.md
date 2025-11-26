# Entity System

## Overview

The Entity System is the foundation of the ECS architecture. Entities are lightweight identifiers that serve as indices for accessing components. The R-Type ECS uses **generational indices** to ensure safety and prevent the ABA problem.

## Entity Structure

```cpp
struct Entity {
    std::uint32_t id = _NullID;

    static constexpr std::uint32_t _IndexBits = 20;     // Entity slot position
    static constexpr std::uint32_t _GenerationBits = 12; // Version counter
    static constexpr std::uint32_t _IndexMask = (1 << _IndexBits) - 1;
    static constexpr std::uint32_t _GenerationMask = (1 << _GenerationBits) - 1;
    static constexpr std::uint32_t _MaxGeneration = _GenerationMask;
    static constexpr std::uint32_t _NullID = std::numeric_limits<std::uint32_t>::max();

    constexpr std::uint32_t index() const noexcept;
    constexpr std::uint32_t generation() const noexcept;
    constexpr bool isNull() const noexcept;
    constexpr bool isTombstone() const noexcept;
};
```

### Memory Layout

Entities are 32-bit packed structures:

```
┌─────────────┬──────────────────────┐
│ Generation  │        Index         │
│  (12 bits)  │      (20 bits)       │
│   [31:20]   │       [19:0]         │
└─────────────┴──────────────────────┘
```

- **Index** (20 bits): Position in the entity array (max 1,048,576 entities)
- **Generation** (12 bits): Version counter (max 4,096 generations)

## Generational Indices

Generational indices solve the "dangling entity reference" problem:

```cpp
// Create entity
Entity e1 = registry.spawnEntity(); // id = 0x00000000 (index=0, gen=0)

// Entity is destroyed
registry.killEntity(e1);
// Internally, generation is incremented: gen=1

// New entity reuses the slot
Entity e2 = registry.spawnEntity(); // id = 0x00100000 (index=0, gen=1)

// Old reference is automatically invalid
bool valid = registry.isAlive(e1); // false - generation mismatch!
```

### Benefits

1. **Safety**: Old entity handles become invalid automatically
2. **Memory Efficiency**: Entity IDs can be reused without confusion
3. **Performance**: No need for pointer indirection or reference counting

## API Reference

### Creation and Destruction

```cpp
// Create a new entity
Entity entity = registry.spawnEntity();

// Destroy an entity (marks as tombstone)
registry.killEntity(entity);

// Check if entity is still valid
bool alive = registry.isAlive(entity);
```

### Entity Properties

```cpp
Entity entity(42, 5); // index=42, generation=5

// Extract index
std::uint32_t idx = entity.index(); // 42

// Extract generation
std::uint32_t gen = entity.generation(); // 5

// Check if null (uninitialized)
bool isNull = entity.isNull();

// Check if tombstone (destroyed, max generation)
bool isTombstone = entity.isTombstone();
```

### Comparison

```cpp
Entity e1(10, 0);
Entity e2(10, 1);
Entity e3(10, 0);

e1 == e3; // true - same index and generation
e1 != e2; // true - different generation
e1 <=> e2; // -1 (three-way comparison)
```

### Hashing

Entities can be used as keys in hash maps:

```cpp
std::unordered_map<Entity, std::string> entity_names;
entity_names[entity] = "Player";
```

## Lifecycle Management

### Entity States

1. **Alive**: Active entity with valid generation
2. **Dead**: Destroyed entity (generation incremented)
3. **Tombstone**: Entity with maximum generation (recyclable)
4. **Null**: Uninitialized entity (id = 0xFFFFFFFF)

### Recycling

```cpp
// Cleanup tombstones and recycle slots
size_t recycled = registry.cleanupTombstones();
```

This resets tombstone generations to 0, making them available for reuse.

### Memory Reservation

For performance, pre-allocate entity storage:

```cpp
registry.reserveEntities(10000); // Reserve space for 10k entities
```

## Advanced Usage

### Conditional Entity Removal

```cpp
// Remove all entities matching a predicate
size_t removed = registry.removeEntitiesIf([](Entity e) {
    // Return true to remove
    return should_remove(e);
});
```

### Entity Validation

```cpp
void process_entity(Entity entity) {
    if (!registry.isAlive(entity)) {
        throw std::runtime_error("Entity is dead!");
    }
    // Safe to use entity
}
```

## Performance Characteristics

| Operation | Time Complexity | Notes |
|-----------|----------------|-------|
| spawnEntity | O(1) amortized | May trigger vector resize |
| killEntity | O(k) | k = number of component types |
| isAlive | O(1) | Array lookup + generation check |
| cleanupTombstones | O(n) | n = total entity slots |

## Best Practices

### ✅ Do

- Always check `isAlive()` before using stored entity references
- Use `reserveEntities()` for known entity counts
- Periodically call `cleanupTombstones()` to reclaim memory
- Store entities by value (they're just 32-bit integers)

### ❌ Don't

- Don't assume entity indices are contiguous
- Don't use entities after destruction without validation
- Don't store entity pointers (entities are trivially copyable)
- Don't create more than 1,048,576 entities (index limit)

## Thread Safety

- `spawnEntity()`: Thread-safe with internal locking
- `killEntity()`: Thread-safe with internal locking
- `isAlive()`: Thread-safe (read-only operation)
- `cleanupTombstones()`: Thread-safe with internal locking

⚠️ **Warning**: Entity operations are safe, but adding/removing components during parallel iteration is **not safe**.

## Example: Entity Pool

```cpp
class EntityPool {
    ECS::Registry& registry;
    std::vector<ECS::Entity> entities;

public:
    EntityPool(ECS::Registry& reg) : registry(reg) {}

    ECS::Entity acquire() {
        if (!entities.empty()) {
            auto e = entities.back();
            entities.pop_back();
            return e;
        }
        return registry.spawnEntity();
    }

    void release(ECS::Entity entity) {
        // Remove all components
        registry.killEntity(entity);
        entities.push_back(entity);
    }
};
```

## Debugging

```cpp
void print_entity_info(Entity entity) {
    std::cout << "Entity["
              << "index=" << entity.index()
              << ", gen=" << entity.generation()
              << ", alive=" << registry.isAlive(entity)
              << "]\n";
}
```

## See Also

- [Registry](03_registry.md) - Entity management
- [Component Storage](02_component_storage.md) - Component association
- [Command Buffer](09_command_buffer.md) - Deferred entity operations
