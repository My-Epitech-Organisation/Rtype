# Views

## Overview

**Views** provide efficient queries for iterating over entities with specific component combinations. They are lightweight, non-owning objects that automatically select the optimal iteration strategy.

## View Types

| Type | Use Case | Thread Safety |
|------|----------|---------------|
| `View<T...>` | Single-threaded iteration | Not thread-safe |
| `ParallelView<T...>` | Multi-threaded iteration | Thread-safe reads/writes |
| `Group<T...>` | Cached, repeated queries | Not thread-safe |
| `ExcludeView<I..., E...>` | Filtered exclusion | Not thread-safe |

## Basic View

### Creation

```cpp
// Single component
auto view = registry.view<Position>();

// Multiple components (intersection)
auto view = registry.view<Position, Velocity>();

// Three or more components
auto view = registry.view<Position, Velocity, Sprite>();
```

### Iteration

```cpp
// Lambda with entity and components
registry.view<Position, Velocity>().each([](Entity e, Position& pos, Velocity& vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
});

// Without entity parameter
registry.view<Position, Velocity>().each([](Position& pos, Velocity& vel) {
    pos.x += vel.dx;
    pos.y += vel.dy;
});

// Capture external variables
float delta_time = 0.016f;
registry.view<Position, Velocity>().each([delta_time](auto e, auto& pos, auto& vel) {
    pos.x += vel.dx * delta_time;
    pos.y += vel.dy * delta_time;
});
```

## ExcludeView

Filter out entities with specific components.

### Usage

```cpp
// Entities with Position and Velocity, but NOT Dead or Frozen
registry.view<Position, Velocity>()
       .exclude<Dead, Frozen>()
       .each([](Entity e, Position& pos, Velocity& vel) {
           pos.x += vel.dx;
       });

// Common patterns
// Active entities (not dead)
registry.view<Health>().exclude<Dead>();

// Moving entities (not stationary)
registry.view<Position>().exclude<Stationary>();

// Visible entities (not hidden or culled)
registry.view<Sprite>().exclude<Hidden, Culled>();
```

### Performance

ExcludeView checks exclusion criteria **only for entities that match include criteria**, making it efficient:

```cpp
// 10,000 entities with Position
// 100 entities with Dead tag
// ExcludeView only checks Dead for the 10,000 Position entities
// Much faster than checking all entities
```

## View Optimization

Views automatically select the smallest component pool for iteration:

```cpp
// 1,000 entities with Position
// 100 entities with Velocity
// View iterates over Velocity (smaller), checks Position
auto view = registry.view<Position, Velocity>();
// Iterates 100 times, not 1,000
```

### Manual Optimization

```cpp
// If you know the distribution, query the smaller set first
// Good: 100 players with Position
registry.view<Player, Position>();

// Bad: 10,000 entities with Position, 100 with Player
// (Still works, but iterates more entities)
```

## Component Access Patterns

### Read-Only

```cpp
registry.view<const Position, const Velocity>().each([](Entity e, const Position& pos, const Velocity& vel) {
    // Cannot modify components
    float distance = std::sqrt(pos.x * pos.x + pos.y * pos.y);
});
```

### Mixed Access

```cpp
registry.view<Position, const Velocity>().each([](Entity e, Position& pos, const Velocity& vel) {
    pos.x += vel.dx; // Modify position
    // Cannot modify velocity
});
```

### Component Getters in Callbacks

```cpp
registry.view<Position>().each([&](Entity e, Position& pos) {
    // Get additional components on-demand
    if (registry.has_component<Velocity>(e)) {
        auto& vel = registry.get_component<Velocity>(e);
        pos.x += vel.dx;
    }
});
```

## Advanced Patterns

### Conditional Processing

```cpp
registry.view<Health>().each([&](Entity e, Health& hp) {
    if (hp.hp <= 0) {
        // Mark for deletion (don't kill during iteration)
        registry.emplace_component<Dead>(e);
    }
});

// Remove dead entities after iteration
registry.view<Dead>().each([&](Entity e) {
    registry.kill_entity(e);
});
```

### Component Relationships

```cpp
// Process parent-child relationships
registry.view<Transform, Parent>().each([&](Entity e, Transform& t, Parent& p) {
    if (registry.is_alive(p.entity)) {
        auto& parent_transform = registry.get_component<Transform>(p.entity);
        // Apply parent transform to child
        t.world_position = parent_transform.world_position + t.local_position;
    }
});
```

### State Machines

```cpp
registry.view<State>().each([&](Entity e, State& state) {
    switch (state.current) {
        case State::Idle:
            if (/* condition */) {
                state.current = State::Moving;
                registry.emplace_component<Velocity>(e, 1.0f, 0.0f);
            }
            break;
        case State::Moving:
            if (/* condition */) {
                state.current = State::Idle;
                registry.remove_component<Velocity>(e);
            }
            break;
    }
});
```

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| View creation | O(k) | k = number of component types |
| Iteration setup | O(k) | Find smallest pool |
| Iteration | O(n) | n = size of smallest pool |
| Component access | O(1) | Direct array indexing |
| Exclusion check | O(e) | e = number of excluded types |

### Memory

- Views are lightweight (few pointers)
- No allocation during iteration
- Components accessed in-place (cache-friendly)

## Thread Safety

### Single View

❌ **Not thread-safe for concurrent iteration:**

```cpp
// UNSAFE
std::thread t1([&] { registry.view<Position>().each(update); });
std::thread t2([&] { registry.view<Position>().each(update); }); // Data race!
```

### Multiple Views

✅ **Safe for different components:**

```cpp
// SAFE: Different component types
std::thread t1([&] { registry.view<Position>().each(update_pos); });
std::thread t2([&] { registry.view<Velocity>().each(update_vel); });
```

### Parallel View

✅ **Use ParallelView for concurrent iteration:**

```cpp
// SAFE: Parallel iteration
registry.parallel_view<Position, Velocity>().each([](Entity e, Position& pos, Velocity& vel) {
    pos.x += vel.dx; // Each thread processes different entities
});
```

See [Parallel Processing](05_parallel_processing.md) for details.

## Best Practices

### ✅ Do

- Use views for all component iteration
- Leverage exclude() for filtering
- Keep callback lambdas small and focused
- Capture by reference `[&]` for external state
- Process structural changes after iteration

### ❌ Don't

- Don't add/remove entities during iteration
- Don't add/remove components to iterated entities
- Don't store view objects long-term (create on demand)
- Don't perform expensive operations in tight loops
- Don't nest view iterations unnecessarily

## Common Patterns

### Two-Pass Processing

```cpp
// Pass 1: Mark entities
registry.view<Health>().each([&](Entity e, Health& hp) {
    if (hp.hp <= 0) {
        registry.emplace_component<Dead>(e);
    }
});

// Pass 2: Remove marked entities
registry.view<Dead>().each([&](Entity e) {
    registry.kill_entity(e);
});
```

### Spatial Queries

```cpp
struct Position { float x, y; };

// Find entities in radius
std::vector<Entity> in_radius;
float radius = 10.0f;
Position center{50.0f, 50.0f};

registry.view<Position>().each([&](Entity e, const Position& pos) {
    float dx = pos.x - center.x;
    float dy = pos.y - center.y;
    if (dx*dx + dy*dy <= radius*radius) {
        in_radius.push_back(e);
    }
});
```

### Component Aggregation

```cpp
// Calculate total health
int total_health = 0;
registry.view<Health>().each([&](Entity e, const Health& hp) {
    total_health += hp.hp;
});

// Find maximum speed
float max_speed = 0.0f;
registry.view<Velocity>().each([&](Entity e, const Velocity& vel) {
    float speed = std::sqrt(vel.dx * vel.dx + vel.dy * vel.dy);
    max_speed = std::max(max_speed, speed);
});
```

## Example: Game Loop

```cpp
void update(ECS::Registry& registry, float dt) {
    // Physics
    registry.view<Position, Velocity>().each([dt](auto e, auto& pos, auto& vel) {
        pos.x += vel.dx * dt;
        pos.y += vel.dy * dt;
    });
    
    // Collision (entities with colliders, not triggers)
    registry.view<Position, Collider>()
           .exclude<Trigger>()
           .each([&](Entity e, Position& pos, Collider& col) {
               // Check collisions...
           });
    
    // Lifetime
    registry.view<Lifetime>().each([&](Entity e, Lifetime& life) {
        life.remaining -= dt;
        if (life.remaining <= 0.0f) {
            registry.kill_entity(e);
        }
    });
    
    // Animation (entities with sprite and animator, not frozen)
    registry.view<Sprite, Animator>()
           .exclude<Frozen>()
           .each([dt](auto e, auto& sprite, auto& anim) {
               anim.time += dt;
               sprite.frame = static_cast<int>(anim.time * anim.fps) % anim.frame_count;
           });
}
```

## See Also

- [Parallel Processing](05_parallel_processing.md) - Multi-threaded iteration
- [Groups](06_groups.md) - Cached queries
- [Registry](03_registry.md) - View creation
- [Component Storage](02_component_storage.md) - Underlying storage
