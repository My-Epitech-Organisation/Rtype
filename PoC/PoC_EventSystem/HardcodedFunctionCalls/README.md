# PoC: Hardcoded Function Calls

## Overview
This proof of concept demonstrates the traditional approach where the game's main update loop directly calls system methods. In this case, `Game::update()` directly calls `PhysicsSystem::checkCollisions()`.

## Architecture

```
┌──────────────┐
│     Game     │
│              │
│  update()    │─────┐
└──────────────┘     │
                     │ Direct Call
                     ▼
              ┌──────────────────┐
              │  PhysicsSystem   │
              │                  │
              │ checkCollisions()│
              └──────────────────┘
```

## Key Characteristics

### Coupling
- **HIGH**: Game class has direct compile-time dependency on PhysicsSystem
- Game must `#include "PhysicsSystem.hpp"`
- Changes to PhysicsSystem interface require recompiling Game
- Cannot dynamically add/remove systems at runtime

### Control Flow
- **Direct and Clear**: Easy to trace in debugger
- Call stack is straightforward: `main() → Game::update() → PhysicsSystem::checkCollisions()`
- No indirection or hidden behavior

### Performance
- **Excellent**: Zero runtime overhead
- Direct function calls can be inlined by compiler
- No virtual dispatch or function pointers
- Optimal cache usage

## Implementation Details

### Game Class
```cpp
class Game {
private:
    ECS::Registry _registry;
    PhysicsSystem _physicsSystem;  // Direct dependency
    
public:
    void update(float deltaTime) {
        // ⚠️ Hardcoded call - tight coupling
        _physicsSystem.checkCollisions(_registry);
    }
};
```

### PhysicsSystem Class
```cpp
class PhysicsSystem {
public:
    int checkCollisions(ECS::Registry& registry) {
        // Detect collisions and tag entities
        // No events, just direct action
    }
};
```

## Pros

### ✅ Simplicity
- Straightforward to understand and implement
- No need for event systems or observers
- Clear, linear code flow

### ✅ Performance
- No runtime overhead from event dispatching
- Compiler can optimize across system boundaries
- Predictable execution time

### ✅ Debugging
- Easy to debug with breakpoints
- Clear call stack
- No hidden indirection

### ✅ Minimal Boilerplate
- No event classes needed
- No subscription management
- Less code to maintain

## Cons

### ❌ Tight Coupling
- Game class depends on every system it uses
- Adding a new system requires modifying Game
- Hard to test systems in isolation

### ❌ Lack of Flexibility
- Cannot add/remove systems at runtime
- Difficult to enable/disable systems conditionally
- System execution order is hardcoded

### ❌ Poor Scalability
- Game class becomes bloated with many systems
- Every new system adds another dependency
- Violates Open/Closed Principle

### ❌ Code Reuse
- Systems cannot be easily reused in different contexts
- Difficult to create alternative game modes
- Testing requires full game environment

## Use Cases

### When to Use
- **Prototypes**: Quick iteration without architectural overhead
- **Small projects**: < 5 systems, clear structure
- **Performance-critical**: Where every microsecond counts
- **Simple games**: Linear gameplay, few interactions

### When to Avoid
- **Large projects**: Many systems with complex interactions
- **Team development**: Multiple developers working on different systems
- **Moddable games**: Need to add systems dynamically
- **Complex interactions**: Many systems need to react to same events

## Build Instructions

```bash
cd PoC/HardcodedFunctionCalls
mkdir build && cd build
cmake ..
make
./poc_hardcoded
```

## Expected Output

```
=== Hardcoded Function Calls PoC ===
Game directly calls PhysicsSystem::checkCollisions()

[Game] Setting up entities...
[Game] Created Entity 0 at (0, 0)
[Game] Created Entity 1 at (1.5, 0)
[Game] Created Entity 2 at (10, 10)

[Game] Running simulation for 5 frames

--- Frame 1 ---
[Physics] Collision detected between Entity 0 and Entity 1
[Game] Total collisions this frame: 1

--- Frame 2 ---
[Physics] Collision detected between Entity 0 and Entity 1
[Game] Total collisions this frame: 1

...
```

## Assessment

### Coupling Level: HIGH
The Game class has explicit knowledge of and dependencies on every system it uses.

### Maintainability: MEDIUM
Simple to understand but becomes harder to maintain as systems grow.

### Extensibility: LOW
Adding new systems requires modifying the Game class and recompiling.

### Performance: EXCELLENT
Direct function calls with no overhead make this the fastest approach.

## Conclusion

Hardcoded function calls are acceptable for:
- Small, focused projects
- Performance-critical code sections
- Prototypes and MVPs
- When the system architecture is stable

However, for larger projects or when flexibility is needed, consider more decoupled approaches like Event Bus or ECS-based system scheduling.

## Related PoCs
- [Event Bus / Observer Pattern](../EventBus/README.md) - Alternative decoupled approach
- [ECS System](../ECS/README.md) - Base architecture used by both PoCs

## Timeline
- **Duration**: 29/11/2025 - 30/11/2025
- **Status**: ✅ Complete
