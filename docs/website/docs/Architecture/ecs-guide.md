---
sidebar_position: 2
---

# ECS Guide

Learn how to use the Entity Component System in R-Type.

## What is ECS?

The Entity Component System (ECS) is an architectural pattern that separates data from behavior:

- **Entities** are unique identifiers
- **Components** are data containers
- **Systems** implement behavior

## Creating Entities

```cpp
#include "rtype/engine/Registry.hpp"

Registry registry;

// Create a new entity
Entity player = registry.spawn_entity();
```

## Adding Components

Components are plain data structures that can be attached to entities:

```cpp
// Define a component
struct PositionComponent {
    float x;
    float y;
};

// Register the component type
registry.register_component<PositionComponent>();

// Add component to entity
registry.emplace_component<PositionComponent>(player, 100.0f, 200.0f);
```

## Creating Systems

Systems operate on entities that have specific components:

```cpp
class MovementSystem {
public:
    void update(Registry& registry, float deltaTime) {
        // Get all entities with Position and Velocity
        auto entities = registry.get_entities_with<PositionComponent, VelocityComponent>();

        for (auto entity : entities) {
            auto& pos = registry.get_component<PositionComponent>(entity);
            auto& vel = registry.get_component<VelocityComponent>(entity);

            // Update position based on velocity
            pos.x += vel.x * deltaTime;
            pos.y += vel.y * deltaTime;
        }
    }
};
```

## Querying Components

```cpp
// Check if entity has a component
if (registry.has_component<PositionComponent>(entity)) {
    auto& pos = registry.get_component<PositionComponent>(entity);
    // Use position...
}

// Remove a component
registry.remove_component<PositionComponent>(entity);

// Kill an entity
registry.kill_entity(entity);
```

## Example: Creating a Spaceship

```cpp
// Create entity
Entity spaceship = registry.spawn_entity();

// Add components
registry.emplace_component<PositionComponent>(spaceship, 400.0f, 300.0f);
registry.emplace_component<VelocityComponent>(spaceship, 0.0f, 0.0f);
registry.emplace_component<SpriteComponent>(spaceship, "spaceship.png");
registry.emplace_component<HealthComponent>(spaceship, 100);
registry.emplace_component<PlayerControlledComponent>(spaceship);
```

## Best Practices

### Keep Components Simple
- Components should be plain data structures
- No logic in components
- Prefer composition over inheritance

### System Independence
- Systems should not depend on each other
- Use the Registry as the communication layer
- Systems can run in any order (with exceptions for dependencies)

### Component Granularity
- Small, focused components are better than large ones
- Easier to reuse and combine
- Better cache performance

## Common Patterns

### Tagging Components
Empty components used as flags:

```cpp
struct EnemyTag {};
struct PlayerTag {};

registry.emplace_component<EnemyTag>(enemy);
```

### Optional Components
Check for component existence to enable optional behavior:

```cpp
if (registry.has_component<AIComponent>(entity)) {
    // This entity has AI
}
```

### Component Events
Use callbacks to react to component changes:

```cpp
registry.on_component_added<HealthComponent>([](Entity entity, HealthComponent& health) {
    // Initialize health bar UI
});
```

## Performance Tips

1. **Batch Operations**: Process all entities of the same type together
2. **Cache Components**: Store references when accessing multiple times
3. **Minimize Component Size**: Smaller components = better cache utilization
4. **Use Sparse Sets**: The Registry uses sparse sets for efficient storage

## API Reference

For detailed API documentation, see:
- [IRegistry Interface](/api/classIRegistry.html)
- [Entity Class](/api/classEntity.html)
- [Registry Implementation](/api/classRegistry.html)

## Next Steps

- Read about [Network Synchronization](./network-architecture)
- Learn how to create [Custom Systems](./custom-systems)
- Explore [Component Examples](./component-examples)
