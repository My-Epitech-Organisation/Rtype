# System Scheduler

## Overview

The **SystemScheduler** provides automatic dependency resolution and execution order management for ECS systems. It topologically sorts systems based on dependencies and can execute independent systems in parallel.

## Core Concepts

### System

A system is a function that operates on the registry:

```cpp
using SystemFunc = std::function<void(Registry&)>;

void physics_system(Registry& reg) {
    reg.view<Position, Velocity>().each([](auto e, auto& pos, auto& vel) {
        pos.x += vel.dx;
        pos.y += vel.dy;
    });
}
```

### Dependencies

Systems can depend on other systems, ensuring execution order:

```cpp
// Physics must run before collision
scheduler.addSystem("physics", physics_system);
scheduler.addSystem("collision", collision_system, {"physics"});
```

## Basic Usage

```cpp
#include "ECS/System/SystemScheduler.hpp"

ECS::SystemScheduler scheduler(registry);

// Add systems
scheduler.addSystem("input", input_system);
scheduler.addSystem("physics", physics_system, {"input"});
scheduler.addSystem("collision", collision_system, {"physics"});
scheduler.addSystem("render", render_system, {"physics", "collision"});

// Execute all systems in dependency order
scheduler.run();
```

## API Reference

### System Management

```cpp
// Add system with dependencies
void addSystem(const std::string& name, 
                SystemFunc func,
                const std::vector<std::string>& dependencies = {});

// Remove system
void removeSystem(const std::string& name);

// Clear all systems
void clear();
```

### Execution

```cpp
// Run all enabled systems
void run();

// Run specific system
void runSystem(const std::string& name);
```

### System Control

```cpp
// Enable/disable system
void setSystemEnabled(const std::string& name, bool enabled);

// Check if enabled
bool isSystemEnabled(const std::string& name) const;

// Get execution order
std::vector<std::string> getExecutionOrder() const;
```

## Dependency Resolution

### Topological Sorting

The scheduler automatically orders systems based on dependencies:

```cpp
scheduler.addSystem("A", system_a);
scheduler.addSystem("B", system_b, {"A"});      // B depends on A
scheduler.addSystem("C", system_c, {"A"});      // C depends on A
scheduler.addSystem("D", system_d, {"B", "C"}); // D depends on B and C

// Execution order: A → (B, C in parallel) → D
```

### Dependency Graph

```
    A
   / \
  B   C
   \ /
    D

Valid execution orders:
- A, B, C, D
- A, C, B, D

Both satisfy dependencies. B and C can run in parallel.
```

### Cyclic Dependencies

Cyclic dependencies are detected and cause errors:

```cpp
scheduler.addSystem("A", system_a, {"B"});
scheduler.addSystem("B", system_b, {"C"});
scheduler.addSystem("C", system_c, {"A"}); // ❌ Cycle: A→B→C→A

// Throws: std::runtime_error("Cyclic dependency detected")
```

## Advanced Usage

### Parallel Execution

Independent systems can run in parallel:

```cpp
scheduler.addSystem("physics", physics_system);
scheduler.addSystem("audio", audio_system);     // Independent
scheduler.addSystem("particles", particle_system); // Independent
scheduler.addSystem("render", render_system, {"physics", "audio", "particles"});

// Execution:
// 1. physics, audio, particles run in parallel
// 2. render runs after all complete
```

### Conditional System Execution

```cpp
scheduler.addSystem("editor_ui", editor_ui_system);

if (is_editor_mode) {
    scheduler.setSystemEnabled("editor_ui", true);
} else {
    scheduler.setSystemEnabled("editor_ui", false);
}

scheduler.run(); // Skips disabled systems
```

### Dynamic System Management

```cpp
class GameMode {
    SystemScheduler& scheduler;
    
public:
    void enter_play_mode() {
        scheduler.addSystem("ai", ai_system);
        scheduler.addSystem("combat", combat_system);
        scheduler.setSystemEnabled("editor_tools", false);
    }
    
    void enter_edit_mode() {
        scheduler.removeSystem("ai");
        scheduler.removeSystem("combat");
        scheduler.setSystemEnabled("editor_tools", true);
    }
};
```

### System Groups

```cpp
// Early update systems
scheduler.addSystem("input", input_system);
scheduler.addSystem("ai_decision", ai_decision_system, {"input"});

// Physics systems
scheduler.addSystem("integrate_velocity", integrate_system, {"ai_decision"});
scheduler.addSystem("collision", collision_system, {"integrate_velocity"});
scheduler.addSystem("resolve_collisions", resolve_system, {"collision"});

// Late update systems
scheduler.addSystem("animation", animation_system, {"resolve_collisions"});
scheduler.addSystem("camera", camera_system, {"animation"});
scheduler.addSystem("render", render_system, {"camera"});
```

## Complete Example

```cpp
#include "ECS/ECS.hpp"
#include "ECS/System/SystemScheduler.hpp"

// Define systems
void input_system(ECS::Registry& reg) {
    // Process input...
}

void physics_system(ECS::Registry& reg) {
    reg.view<Position, Velocity>().each([](auto e, auto& pos, auto& vel) {
        pos.x += vel.dx * 0.016f;
        pos.y += vel.dy * 0.016f;
    });
}

void collision_system(ECS::Registry& reg) {
    // Check collisions...
}

void render_system(ECS::Registry& reg) {
    reg.view<Position, Sprite>().each([](auto e, auto& pos, auto& sprite) {
        draw_sprite(sprite, pos.x, pos.y);
    });
}

int main() {
    ECS::Registry registry;
    ECS::SystemScheduler scheduler(registry);
    
    // Register systems with dependencies
    scheduler.addSystem("input", input_system);
    scheduler.addSystem("physics", physics_system, {"input"});
    scheduler.addSystem("collision", collision_system, {"physics"});
    scheduler.addSystem("render", render_system, {"collision"});
    
    // Game loop
    while (running) {
        scheduler.run(); // Execute all systems in order
    }
    
    return 0;
}
```

## Execution Order Inspection

```cpp
// Get and print execution order
auto order = scheduler.getExecutionOrder();
std::cout << "System execution order:\n";
for (size_t i = 0; i < order.size(); ++i) {
    std::cout << (i + 1) << ". " << order[i] << "\n";
}

// Output:
// System execution order:
// 1. input
// 2. physics
// 3. collision
// 4. render
```

## Performance Considerations

### System Registration

- Registration: O(1) insertion
- Dependency resolution: O(V + E) where V = systems, E = dependencies
- Cached execution order (computed once)

### Runtime Execution

```cpp
// Sequential execution
for (auto& system_name : _executionOrder) {
    if (systems[system_name].enabled) {
        systems[system_name].func(registry); // O(1) lookup
    }
}
```

### Parallel Execution

Currently, systems run sequentially. For parallel execution of independent systems:

```cpp
// Future feature: automatic parallelization
// Independent systems (no shared dependencies) run in thread pool
```

## Best Practices

### ✅ Do

- Name systems descriptively
- Declare all dependencies explicitly
- Keep systems focused (single responsibility)
- Use enable/disable for conditional logic
- Group related systems logically
- Document system dependencies

### ❌ Don't

- Don't create cyclic dependencies
- Don't assume execution order without dependencies
- Don't perform heavy initialization in systems (do it once)
- Don't modify the scheduler during `run()`
- Don't rely on undefined execution order

## Common Patterns

### Fixed vs Variable Update

```cpp
class GameLoop {
    SystemScheduler fixed_scheduler;  // Physics, etc.
    SystemScheduler variable_scheduler; // Render, etc.
    
    float accumulator = 0.0f;
    const float fixed_dt = 0.016f;
    
    void update(float dt) {
        accumulator += dt;
        
        // Fixed timestep
        while (accumulator >= fixed_dt) {
            fixed_scheduler.run();
            accumulator -= fixed_dt;
        }
        
        // Variable timestep
        variable_scheduler.run();
    }
};
```

### Phased Execution

```cpp
SystemScheduler early_update(registry);
SystemScheduler update(registry);
SystemScheduler late_update(registry);

void game_frame() {
    early_update.run();   // Input, AI decisions
    update.run();         // Physics, gameplay
    late_update.run();    // Animation, rendering
}
```

### Debug Systems

```cpp
#ifdef DEBUG_MODE
scheduler.addSystem("debug_draw", debug_draw_system, {"render"});
scheduler.addSystem("profiler", profiler_system, {"render"});
#else
scheduler.setSystemEnabled("debug_draw", false);
scheduler.setSystemEnabled("profiler", false);
#endif
```

### System Hot-Reloading

```cpp
void reload_system(const std::string& name, SystemFunc new_func) {
    // Get current enabled state
    bool was_enabled = scheduler.isSystemEnabled(name);
    
    // Remove old system
    scheduler.removeSystem(name);
    
    // Add new implementation
    scheduler.addSystem(name, new_func, /* same dependencies */);
    
    // Restore enabled state
    scheduler.setSystemEnabled(name, was_enabled);
}
```

## Thread Safety

- ❌ Scheduler is NOT thread-safe
- ❌ Don't call `run()` from multiple threads
- ❌ Don't modify scheduler during execution

For thread-safe system execution, use external synchronization:

```cpp
std::mutex scheduler_mutex;

void safe_run() {
    std::lock_guard lock(scheduler_mutex);
    scheduler.run();
}
```

## Debugging

### Print Execution Order

```cpp
void debug_print_schedule(const SystemScheduler& scheduler) {
    auto order = scheduler.getExecutionOrder();
    std::cout << "=== System Schedule ===\n";
    for (const auto& name : order) {
        std::cout << "  - " << name;
        if (!scheduler.isSystemEnabled(name)) {
            std::cout << " (disabled)";
        }
        std::cout << "\n";
    }
}
```

### Execution Timing

```cpp
#include <chrono>

void timed_run(SystemScheduler& scheduler) {
    auto start = std::chrono::high_resolution_clock::now();
    scheduler.run();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Systems executed in " << duration.count() << " μs\n";
}
```

## See Also

- [Registry](03_registry.md) - ECS coordinator
- [Views](04_views.md) - Component queries
- [Benchmarking](13_benchmarking.md) - Performance measurement
