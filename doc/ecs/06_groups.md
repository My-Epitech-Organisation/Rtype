# Groups

## Overview

**Groups** are cached entity collections for repeated filtered queries. Unlike views which filter on-the-fly during each iteration, groups maintain a pre-filtered list of entities for maximum iteration speed.

## When to Use Groups

### ✅ Use Groups When:
- Same query runs frequently (every frame)
- Entity structure changes infrequently
- Iteration speed is critical
- Query involves multiple components

### ❌ Use Views When:
- Query runs once or rarely
- Entity structure changes frequently
- Component combinations are dynamic
- Simplicity is preferred over speed

## Performance Comparison

```cpp
// View: Filters on every iteration
for (int i = 0; i < 1000; ++i) {
    registry.view<Position, Velocity>().each([](auto e, auto& p, auto& v) {
        // Checks component presence each time
    });
}
// Time: ~50ms

// Group: Filters once, caches result
auto group = registry.create_group<Position, Velocity>();
for (int i = 0; i < 1000; ++i) {
    group.each([](auto e, auto& p, auto& v) {
        // Direct iteration over cached entities
    });
}
// Time: ~15ms (3.3× faster)
```

## Basic Usage

```cpp
// Create group
auto group = registry.create_group<Position, Velocity>();

// Iterate cached entities
group.each([](Entity e, Position& pos, Velocity& vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
});

// After structural changes, rebuild
registry.emplace_component<Velocity>(new_entity, 1.0f, 0.0f);
group.rebuild(); // Update cache
```

## API Reference

### Creation

```cpp
template<typename... Components>
Group<Components...> create_group();

// Examples
auto pos_group = registry.create_group<Position>();
auto physics_group = registry.create_group<Position, Velocity>();
auto renderable_group = registry.create_group<Position, Sprite, Transform>();
```

### Iteration

```cpp
// With entity and components
group.each([](Entity e, Position& pos, Velocity& vel) {
    // Process entity
});

// Without entity
group.each([](Position& pos, Velocity& vel) {
    // Process components
});
```

### Inspection

```cpp
// Get cached entity list
const std::vector<Entity>& entities = group.get_entities();

// Check size
size_t count = group.size();

// Check if empty
bool empty = group.empty();

// Iterator access
for (Entity e : group) {
    // Process each entity
}
```

### Rebuilding

```cpp
// Manual rebuild after structural changes
group.rebuild();
```

## Lifecycle Management

### Automatic Invalidation

Groups do NOT automatically update when:
- Entities are created/destroyed
- Components are added/removed

You must manually call `rebuild()`:

```cpp
auto group = registry.create_group<Position, Velocity>();

// Initial population
group.rebuild();

// Use group
for (int frame = 0; frame < 100; ++frame) {
    group.each([](auto e, auto& p, auto& v) {
        p.x += v.dx;
    });
}

// Add new entities
for (int i = 0; i < 10; ++i) {
    auto e = registry.spawn_entity();
    registry.emplace_component<Position>(e, 0.0f, 0.0f);
    registry.emplace_component<Velocity>(e, 1.0f, 0.0f);
}

// MUST rebuild to include new entities
group.rebuild();
```

### Rebuild Strategies

#### Strategy 1: Manual (Best Performance)

```cpp
// Rebuild only when structure changes
auto group = registry.create_group<Position, Velocity>();
group.rebuild();

// Game loop
while (running) {
    // Use group many times
    group.each(physics_system);
    group.each(render_system);
    
    // Structural change
    if (spawn_enemy()) {
        group.rebuild(); // Rebuild once
    }
}
```

#### Strategy 2: Periodic

```cpp
auto group = registry.create_group<Position, Velocity>();
int frame_count = 0;

while (running) {
    group.each(update_system);
    
    // Rebuild every 10 frames
    if (++frame_count % 10 == 0) {
        group.rebuild();
    }
}
```

#### Strategy 3: Event-Driven

```cpp
bool group_dirty = true;

// Mark dirty on structural changes
registry.on_construct<Velocity>([&](Entity e) { group_dirty = true; });
registry.on_destroy<Velocity>([&](Entity e) { group_dirty = true; });

// Rebuild when needed
while (running) {
    if (group_dirty) {
        group.rebuild();
        group_dirty = false;
    }
    
    group.each(update_system);
}
```

## Advanced Patterns

### Multiple Groups

```cpp
class GameSystems {
    ECS::Registry& registry;
    Group<Position, Velocity> physics_group;
    Group<Position, Sprite> render_group;
    Group<AI, Position> ai_group;
    
public:
    GameSystems(ECS::Registry& reg) 
        : registry(reg),
          physics_group(reg),
          render_group(reg),
          ai_group(reg) {
        rebuild_all();
    }
    
    void rebuild_all() {
        physics_group.rebuild();
        render_group.rebuild();
        ai_group.rebuild();
    }
    
    void update() {
        physics_group.each(update_physics);
        ai_group.each(update_ai);
        render_group.each(render_sprites);
    }
};
```

### Hierarchical Groups

```cpp
// Broad group
auto all_entities = registry.create_group<Position>();

// Specialized subgroups
auto moving_entities = registry.create_group<Position, Velocity>();
auto visible_entities = registry.create_group<Position, Sprite>();

// Rebuild hierarchy
all_entities.rebuild();
moving_entities.rebuild();
visible_entities.rebuild();
```

### Group Composition

```cpp
// Find entities in group A but not in group B
auto group_a = registry.create_group<Position, Velocity>();
auto group_b = registry.create_group<Dead>();

group_a.rebuild();
group_b.rebuild();

const auto& dead_entities = group_b.get_entities();

group_a.each([&](Entity e, Position& pos, Velocity& vel) {
    // Check if entity is NOT dead
    if (std::find(dead_entities.begin(), dead_entities.end(), e) == dead_entities.end()) {
        pos.x += vel.dx;
    }
});
```

## Performance Optimization

### Memory Locality

Groups store entity IDs, not components. Components are still accessed via registry:

```cpp
// Group only caches entity IDs
std::vector<Entity> entities; // Small memory footprint

// Components accessed on-demand (cache-friendly if accessed sequentially)
group.each([&](Entity e, Position& pos, Velocity& vel) {
    // pos and vel are references to components in sparse sets
});
```

### Rebuild Cost

Rebuilding scans all entities to find matches:

```cpp
// Rebuild complexity: O(n * k)
// n = total entities, k = component types to check
group.rebuild();
```

For 100,000 entities with 3 components: ~0.5ms rebuild time.

### Amortized Cost

```cpp
// Scenario: 10,000 iterations between rebuilds
// View: 10,000 × 0.01ms = 100ms
// Group: 1 × 0.5ms (rebuild) + 10,000 × 0.003ms (iterate) = 30.5ms
// Group is 3.3× faster despite rebuild cost
```

## Thread Safety

Groups are **NOT thread-safe**:

```cpp
// ❌ UNSAFE: Concurrent access
std::thread t1([&] { group.each(update1); });
std::thread t2([&] { group.each(update2); }); // Data race on group state

// ✅ SAFE: Use ParallelView instead
registry.parallel_view<Position, Velocity>().each(update);

// ✅ SAFE: External synchronization
std::mutex group_mutex;
std::thread t1([&] { 
    std::lock_guard lock(group_mutex);
    group.each(update1); 
});
```

## Use Cases

### Game Loop Optimization

```cpp
class GameWorld {
    ECS::Registry& registry;
    Group<Position, Velocity> moving_entities;
    Group<Position, Sprite> visible_entities;
    Group<AI> ai_entities;
    
public:
    void init() {
        moving_entities.rebuild();
        visible_entities.rebuild();
        ai_entities.rebuild();
    }
    
    void fixed_update(float dt) {
        // Physics runs at fixed rate
        moving_entities.each([dt](auto e, auto& p, auto& v) {
            p.x += v.dx * dt;
            p.y += v.dy * dt;
        });
    }
    
    void render() {
        // Render every frame
        visible_entities.each([](auto e, auto& p, auto& s) {
            draw_sprite(s, p.x, p.y);
        });
    }
    
    void update_ai() {
        // AI runs at lower frequency
        ai_entities.each([](auto e, auto& ai) {
            ai.think();
        });
    }
};
```

### Entity Pooling

```cpp
class EntityPool {
    ECS::Registry& registry;
    Group<Inactive> inactive_group;
    
public:
    Entity acquire() {
        if (!inactive_group.empty()) {
            Entity e = *inactive_group.begin();
            registry.remove_component<Inactive>(e);
            inactive_group.rebuild();
            return e;
        }
        return registry.spawn_entity();
    }
    
    void release(Entity e) {
        registry.emplace_component<Inactive>(e);
        inactive_group.rebuild();
    }
};
```

### Spatial Partitioning

```cpp
// Cache entities by spatial region
std::unordered_map<int, Group<Position>> spatial_groups;

void update_spatial_groups() {
    registry.view<Position>().each([&](Entity e, const Position& pos) {
        int region = calculate_region(pos);
        // Add to appropriate group and rebuild
        spatial_groups[region].rebuild();
    });
}

void query_region(int region) {
    if (spatial_groups.contains(region)) {
        spatial_groups[region].each(process_entity);
    }
}
```

## Best Practices

### ✅ Do

- Use for frequently repeated queries
- Rebuild after structural changes
- Profile to verify performance gain
- Keep rebuild frequency low
- Use for stable entity sets

### ❌ Don't

- Don't use for one-off queries (use views)
- Don't forget to rebuild after changes
- Don't use with rapidly changing entity structure
- Don't use for thread-safe iteration (use ParallelView)
- Don't over-optimize premature

## Comparison Table

| Feature | View | Group | ParallelView |
|---------|------|-------|--------------|
| Iteration speed | Fast | Fastest | Fast |
| Memory usage | None | O(n) | None |
| Rebuild cost | None | O(n) | None |
| Thread-safe | No | No | Yes |
| Auto-updates | Yes | No | Yes |
| Best for | Dynamic queries | Repeated queries | Large datasets |

## See Also

- [Views](04_views.md) - On-the-fly filtering
- [Parallel Processing](05_parallel_processing.md) - Multi-threaded iteration
- [Optimization Guide](14_optimization.md) - Performance tips
