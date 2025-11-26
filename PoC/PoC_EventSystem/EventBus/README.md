# PoC: Event Bus / Observer Pattern

## Overview
This proof of concept demonstrates the Observer Pattern using an Event Bus to decouple system communication. Systems publish events without knowing who subscribes, and systems subscribe to events without knowing who publishes them.

## Architecture

```
┌──────────────┐
│     Game     │
│              │
│  update()    │
└──────┬───────┘
       │
       ▼
┌──────────────────┐        ┌─────────────────┐
│  PhysicsSystem   │        │    EventBus     │
│                  │        │                 │
│ checkCollisions()│───────►│   publish()     │
└──────────────────┘        │                 │
                            │   subscribe()   │◄────┐
                            └─────────────────┘     │
                                     │              │
                                     │ Event        │
                                     ▼              │
                            ┌─────────────────┐     │
                            │  AudioSystem    │     │
                            │                 │─────┘
                            │  onCollision()  │
                            └─────────────────┘
```

## Key Characteristics

### Coupling
- **LOW**: Systems communicate through events, not direct calls
- PhysicsSystem doesn't know AudioSystem exists
- AudioSystem doesn't know PhysicsSystem exists
- Easy to add/remove systems without modifying existing code

### Control Flow
- **Indirect**: Events are published and subscribers are notified
- Call stack: `main() → Game::update() → PhysicsSystem::checkCollisions() → EventBus::publish() → AudioSystem::onCollision()`
- Requires understanding of Observer Pattern

### Performance
- **Good**: Small overhead from event dispatch (~1-5% typically)
- Function pointer indirection
- Event object allocation
- Still acceptable for most games

## Implementation Details

### Event Bus Class
```cpp
class EventBus {
public:
    template<typename T>
    CallbackId subscribe(std::function<void(const T&)> callback);
    
    template<typename T>
    void publish(const T& event);
    
    template<typename T>
    void unsubscribe(CallbackId id);
};
```

### Event Definition
```cpp
struct CollisionEvent : public Event {
    ECS::Entity entityA;
    ECS::Entity entityB;
    float posX, posY;
};
```

### Publisher (PhysicsSystem)
```cpp
class PhysicsSystem {
public:
    int checkCollisions(ECS::Registry& registry) {
        // Detect collision
        if (collision) {
            // ✅ Publish event - decoupled!
            _eventBus.publish(CollisionEvent(entityA, entityB, x, y));
        }
    }
};
```

### Subscriber (AudioSystem)
```cpp
class AudioSystem {
public:
    void initialize() {
        // ✅ Subscribe to events - decoupled!
        _eventBus.subscribe<CollisionEvent>([this](const CollisionEvent& e) {
            this->onCollision(e);
        });
    }
    
    void onCollision(const CollisionEvent& event) {
        playSound("collision.wav", event.posX, event.posY);
    }
};
```

## Pros

### ✅ Loose Coupling
- Systems don't need to know about each other
- No compile-time dependencies between systems
- True separation of concerns

### ✅ Flexibility
- Easy to add new systems (just subscribe to events)
- Easy to remove systems (just unsubscribe)
- Can dynamically enable/disable systems at runtime

### ✅ Testability
- Systems can be tested in isolation
- Mock event buses for unit testing
- No need for full game environment

### ✅ Extensibility
- New systems can subscribe to existing events
- New events can be added without modifying existing systems
- Follows Open/Closed Principle

### ✅ Multiple Subscribers
- Multiple systems can react to the same event
- Example: Collision → Audio, Particles, Score, Network, etc.
- No need to call each system individually

## Cons

### ❌ Complexity
- More complex than direct calls
- Requires understanding of Observer Pattern
- More boilerplate code (events, subscribers)

### ❌ Indirection
- Harder to trace execution flow
- Debugger call stack is less clear
- Need to search for subscribers

### ❌ Performance Overhead
- Function pointer dispatch
- Event object allocation/deallocation
- Copy overhead when passing events
- Typically 1-5% slower than direct calls

### ❌ Memory Management
- Need to manage subscriber lifetimes
- Risk of dangling callbacks if not unsubscribed
- Event object memory overhead

### ❌ Order Uncertainty
- Subscribers are notified in registration order
- No guaranteed execution order
- Can be problematic for dependencies between systems

## Use Cases

### When to Use
- **Large projects**: Many systems with complex interactions
- **Team development**: Multiple developers working independently
- **Moddable games**: Need to add systems without recompiling
- **Complex interactions**: Many systems react to same events

### When to Avoid
- **Prototypes**: Overhead not worth it for quick tests
- **Performance-critical**: Every cycle counts
- **Simple projects**: < 5 systems, clear structure
- **Deterministic simulation**: Need precise execution order

## Build Instructions

```bash
cd PoC/EventBus
mkdir build && cd build
cmake ..
make
./poc_eventbus
```

## Expected Output

```
=== Event Bus / Observer Pattern PoC ===
Systems communicate through EventBus

[PhysicsSystem] Initialized
[AudioSystem] Initialized
[Game] Setting up systems...
[AudioSystem] Subscribing to CollisionEvent
[AudioSystem] Ready to handle collision sounds
[Game] Setting up entities...
[Game] Created Entity 0 at (0, 0)
[Game] Created Entity 1 at (1.5, 0)
[Game] Created Entity 2 at (10, 10)

[Game] EventBus has 1 subscriber(s) to CollisionEvent

[Game] Running simulation for 5 frames

--- Frame 1 ---
[PhysicsSystem] Collision detected between Entity 0 and Entity 1
[AudioSystem] Received CollisionEvent for entities 0 and 1
[AudioSystem] 🔊 Playing 'collision.wav' at position (0.75, 0)
[AudioSystem] Sound ID: 1000
[Game] Total collisions this frame: 1

...
```

## Assessment

### Coupling Level: LOW
Systems communicate through events, maintaining independence.

### Maintainability: HIGH
Easy to add/remove systems without touching existing code.

### Extensibility: HIGH
New systems can be added by subscribing to existing events.

### Performance: GOOD
Small overhead (~1-5%) is acceptable for most games.

## Advanced Features

### Thread Safety
The EventBus implementation is thread-safe:
- Subscribers can be added/removed from any thread
- Events can be published from any thread
- Callbacks execute without holding locks (prevents deadlocks)

### Error Handling
```cpp
try {
    _eventBus.publish(event);
} catch (const std::exception& e) {
    // Handle subscriber errors gracefully
}
```

### Event Queuing (Future Enhancement)
```cpp
// Queue events for next frame instead of immediate dispatch
_eventBus.queueEvent(CollisionEvent(...));
_eventBus.dispatchQueuedEvents(); // Call at frame end
```

### Event Filtering
```cpp
// Subscribe with predicate
_eventBus.subscribe<CollisionEvent>([](const CollisionEvent& e) {
    return e.entityA.getId() < 100; // Only handle certain entities
}, callback);
```

## Design Patterns Used

### Observer Pattern
- **Subject**: EventBus
- **Observer**: Systems subscribing to events
- **Event**: Notification data

### Mediator Pattern
- **Mediator**: EventBus
- **Colleagues**: Systems
- **Communication**: Through mediator only

## Conclusion

Event Bus / Observer Pattern is recommended for:
- Medium to large projects
- Games with many interacting systems
- Team development environments
- When flexibility and extensibility matter more than raw performance

The small performance overhead is well worth the architectural benefits.

## Related PoCs
- [Hardcoded Function Calls](../HardcodedFunctionCalls/README.md) - Alternative tightly-coupled approach
- [ECS System](../ECS/README.md) - Base architecture used by both PoCs

## Timeline
- **Duration**: 30/11/2025 - 01/12/2025
- **Status**: ✅ Complete

## Dependencies
- Related to: [Spike] [Main] Event System PoC #68
