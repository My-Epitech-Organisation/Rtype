# Event System Architecture

Comparative analysis of inter-system communication patterns.

## Executive Summary

We evaluated three approaches for system communication:

1. **Hardcoded Function Calls** - Direct coupling
2. **Event Bus (Observer Pattern)** - Decoupled communication
3. **Command Queue** - Thread-safe message passing

**Final Decision**: Hybrid approach using Command Queue for thread communication and Event Bus for system decoupling.

---

## Approaches Overview

| Pattern | Use Case | Thread-Safe | Coupling |
|---------|----------|-------------|----------|
| **Hardcoded Calls** | Simple control flow | N/A | High |
| **Event Bus** | System-to-system events | Yes | Low |
| **Command Queue** | Thread-to-thread messaging | Yes (mutex) | Low |

---

## Approach 1: Hardcoded Function Calls

### Concept

The Game class directly calls system methods in predetermined order.

```cpp
class Game {
    PhysicsSystem _physics;
    AudioSystem _audio;
    RenderSystem _render;

    void update() {
        _physics.checkCollisions(_registry);
        _audio.update(_registry);
        _render.draw(_registry);
    }
};
```

### Architecture

```
┌──────────────┐
│     Game     │
│              │
│  update()    │─────┬─────────────────┬─────────────────┐
└──────────────┘     │                 │                 │
                     ▼                 ▼                 ▼
              ┌────────────┐   ┌────────────┐   ┌────────────┐
              │  Physics   │   │   Audio    │   │   Render   │
              └────────────┘   └────────────┘   └────────────┘
```

### Evaluation

| Aspect | Assessment |
|--------|------------|
| **Coupling** | High - Game knows all systems |
| **Flexibility** | Low - Adding system = modify Game |
| **Performance** | Excellent - Direct calls, inlinable |
| **Debugging** | Easy - Clear call stack |
| **Testing** | Hard - Cannot isolate systems |

---

## Approach 2: Event Bus (Observer Pattern)

### Concept

Systems communicate through a central Event Bus. Publishers emit events without knowing subscribers.

```cpp
class Game {
    EventBus _eventBus;

    void initialize() {
        _physics.initialize(_eventBus);
        _audio.initialize(_eventBus);
    }
};

// PhysicsSystem publishes events
void PhysicsSystem::checkCollisions() {
    if (collision) {
        _eventBus.publish(CollisionEvent{entityA, entityB});
    }
}

// AudioSystem subscribes and reacts
void AudioSystem::initialize(EventBus& bus) {
    bus.subscribe<CollisionEvent>([this](const CollisionEvent& e) {
        playSound("collision.wav");
    });
}
```

### Architecture

```
┌──────────────────┐        ┌─────────────────┐
│  PhysicsSystem   │        │    EventBus     │
│                  │───────►│   publish()     │
│ checkCollisions()│        │   subscribe()   │◄────┐
└──────────────────┘        └────────┬────────┘     │
                                     │ Event        │
                                     ▼              │
                            ┌─────────────────┐     │
                            │  AudioSystem    │─────┘
                            │  onCollision()  │
                            └─────────────────┘
```

### Evaluation

| Aspect | Assessment |
|--------|------------|
| **Coupling** | Low - Systems only know EventBus |
| **Flexibility** | High - Add/remove systems dynamically |
| **Performance** | Good - Small overhead (~1-5%) |
| **Debugging** | Medium - Indirect control flow |
| **Testing** | Easy - Isolate with mock EventBus |

### Benefits

**1. No Direct Dependencies**

```cpp
// Hardcoded: PhysicsSystem must know AudioSystem
class PhysicsSystem {
    AudioSystem& _audio;  // Direct dependency
};

// Event Bus: PhysicsSystem knows nothing about AudioSystem
class PhysicsSystem {
    EventBus& _bus;  // Fire and forget
};
```

**2. Independent Development**

```
Developer A: Creates PhysicsSystem
  └── Publishes: CollisionEvent
  └── Subscribes: nothing

Developer B: Creates AudioSystem
  └── Publishes: nothing
  └── Subscribes: CollisionEvent

→ No merge conflicts!
```

**3. Runtime Flexibility**

```cpp
// Disable system temporarily
_audioSystem.unsubscribeAll();  // Mute
// ...
_audioSystem.resubscribe();     // Unmute
```

**4. Isolated Testing**

```cpp
TEST(AudioSystem, PlaysCollisionSound) {
    EventBus mockBus;
    AudioSystem audio(mockBus);
    
    mockBus.publish(CollisionEvent{entity1, entity2});
    
    EXPECT_TRUE(soundPlayed);
}
// No Game, no Physics needed!
```

---

## Approach 3: Command Queue

### std::queue Implementation

```cpp
class ACommand {
    std::queue<Message> _commands;
    std::mutex _mutex;

    void addNewCommand(const Message& cmd) {
        std::lock_guard<std::mutex> lock(_mutex);
        _commands.push(cmd);
    }

    void execute(Game& game) {
        std::lock_guard<std::mutex> lock(_mutex);
        while (!_commands.empty()) {
            process(_commands.front());
            _commands.pop();
        }
    }
};
```

### CircularBuffer Implementation

```cpp
class ACommand {
    CircularBuffer _buffer;  // Fixed-size
    std::mutex _mutex;

    void addNewCommand(const Message& cmd) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto serialized = cmd.serialize();
        _buffer.write(serialized);
    }

    void execute(Game& game) {
        std::lock_guard<std::mutex> lock(_mutex);
        while (_buffer.hasData()) {
            auto msgData = _buffer.read();
            Message msg = Message::deserialize(msgData);
            process(msg);
        }
    }
};
```

### Architecture

```
┌─────────────────┐                      ┌─────────────────┐
│  Network Thread │                      │  Game Thread    │
│                 │                      │                 │
│  receive()      │                      │  update()       │
│       │         │                      │       │         │
│       ▼         │                      │       ▼         │
│  addNewCommand()│─────────────────────►│  execute()      │
│                 │    CircularBuffer    │                 │
└─────────────────┘    (thread-safe)     └─────────────────┘
```

### Queue Comparison

| Criteria | std::queue | CircularBuffer |
|----------|------------|----------------|
| **Memory allocation** | Per message | None (pre-allocated) |
| **Memory bound** | Unlimited | Fixed capacity |
| **Cache performance** | Poor | Excellent |
| **Complexity** | Simple | Requires serialization |
| **Overflow** | Grows | Overwrites oldest |
| **Best for** | Variable load | High throughput |

---

## Side-by-Side Comparison

| Criteria | Hardcoded | Event Bus | Command Queue |
|----------|-----------|-----------|---------------|
| **Add new system** | Modify Game | Subscribe | N/A |
| **Runtime enable/disable** | Conditional | Subscribe/Unsubscribe | N/A |
| **Compile-time deps** | All systems | Only EventBus | Only Message |
| **Performance overhead** | 0% | ~1-5% | ~1-2% |
| **Thread-safety** | Manual | Built-in | Built-in |
| **Merge conflicts** | High | Low | Low |
| **Network integration** | Manual | Events map to packets | Natural fit |

---

## Final Decision

### Hybrid Architecture

| Component | Chosen Approach | Rationale |
|-----------|-----------------|-----------|
| **System-to-System** | Event Bus | Decoupling, testability |
| **Thread-to-Thread** | CircularBuffer | Performance, bounded memory |
| **Critical path** | Hardcoded (selective) | Zero overhead for hot loops |

### Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                          GAME                               │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────┐    EventBus    ┌─────────────┐            │
│  │   Physics   │◄──────────────►│    Audio    │            │
│  └─────────────┘                └─────────────┘            │
│         │                              ▲                    │
│         │ publish(CollisionEvent)      │ subscribe         │
│         ▼                              │                    │
│  ┌─────────────────────────────────────┴───┐               │
│  │              Event Bus                   │               │
│  └─────────────────────────────────────────┘               │
│         ▲                              │                    │
│         │ subscribe                    │ publish            │
│         │                              ▼                    │
│  ┌─────────────┐    CircularBuffer  ┌─────────────┐        │
│  │  Graphics   │◄───────────────────│   Network   │        │
│  │  (Main)     │    (thread-safe)   │  (Thread)   │        │
│  └─────────────┘                    └─────────────┘        │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Why Command Queue for Network?

1. **Thread-safety by design** - Producer-consumer pattern
2. **Simplicity** - Event Bus overkill for thread messaging
3. **CircularBuffer performance** - No allocations, cache-friendly
4. **Network-friendly** - Already works with serialized bytes

### Why Event Bus for Systems?

1. **Decoupled subsystems** - Independent development
2. **Runtime flexibility** - Enable/disable dynamically
3. **Testability** - Mock EventBus for unit tests
4. **Network integration** - Events map to packets

---

## Implementation Summary

### Command Queue (Network to Game)

```cpp
class ACommand : public ICommand {
    CircularBuffer _buffer;  // 4096 bytes default
    std::mutex _mutex;

    void addNewCommand(const Message& cmd);  // Thread-safe push
    void execute(Game& game);                // Thread-safe consume
    bool isEmpty();                          // Thread-safe check
};
```

### Event Bus (System to System)

```cpp
class EventBus {
    template<typename Event>
    void subscribe(std::function<void(const Event&)> handler);
    
    template<typename Event>
    void publish(const Event& event);
    
    void unsubscribeAll();
};
```

---

## References

- PoC implementations: `/PoC/PoC_EventSystem/`
- Command Queue: `/PoC/PoC_EventSystem/CommandQueue/`
- Circular Buffer: `/PoC/PoC_EventSystem/CircularBuffer/`
- Event Bus: `/PoC/PoC_EventSystem/EventBus/`
- Hardcoded: `/PoC/PoC_EventSystem/HardcodedFunctionCalls/`
- Decision document: `/PoC/PoC_EventSystem/event_system_report.md`
