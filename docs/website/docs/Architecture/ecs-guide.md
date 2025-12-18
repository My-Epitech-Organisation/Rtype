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
#include "rtype/ecs.hpp"

using namespace ECS;

Registry registry;

// Create a new entity
Entity player = registry.spawnEntity();
```

## Adding Components

Components are plain data structures that can be attached to entities:

```cpp
// Define a component
struct PositionComponent {
    float x;
    float y;
};

// Add component to entity (components are auto-registered on first use)
registry.emplaceComponent<PositionComponent>(player, 100.0f, 200.0f);
```

## Creating Systems

Systems operate on entities that have specific components:

```cpp
class MovementSystem {
public:
    void update(Registry& registry, float deltaTime) {
        // Get all entities with Position and Velocity
        auto view = registry.view<PositionComponent, VelocityComponent>();

        for (auto entity : view) {
            auto& pos = registry.getComponent<PositionComponent>(entity);
            auto& vel = registry.getComponent<VelocityComponent>(entity);

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
if (registry.hasComponent<PositionComponent>(entity)) {
    auto& pos = registry.getComponent<PositionComponent>(entity);
    // Use position...
}

// Remove a component
registry.removeComponent<PositionComponent>(entity);

// Kill an entity
registry.killEntity(entity);
```

## Example: Creating a Spaceship

```cpp
// Create entity
Entity spaceship = registry.spawnEntity();

// Add components
registry.emplaceComponent<PositionComponent>(spaceship, 400.0f, 300.0f);
registry.emplaceComponent<VelocityComponent>(spaceship, 0.0f, 0.0f);
registry.emplaceComponent<SpriteComponent>(spaceship, "spaceship.png");
registry.emplaceComponent<HealthComponent>(spaceship, 100);
registry.emplaceComponent<PlayerControlledComponent>(spaceship);
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

For detailed API documentation, see the [API Reference](/api/).

## Next Steps

- Read about [Network Synchronization](./network-architecture)
- Learn how to create [Custom Systems](./custom-systems)
- Explore [Component Examples](./component-examples)
