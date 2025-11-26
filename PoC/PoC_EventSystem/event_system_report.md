# Event System Architecture Report

## Executive Summary

This report analyzes three Proof-of-Concept implementations for inter-system communication in the R-Type game engine:

1. **PoC_CommandQueue** — Command Queue pattern using `std::queue`
2. **PoC_CircularBuffer** — Command Queue with Circular Buffer for memory efficiency
3. **PoC_EventSystem** — Event Bus (Observer Pattern) vs Hardcoded Function Calls

Our goal is to achieve **Decoupled Subsystems** while maintaining performance and thread-safety.

---

## Table of Contents

- [1. Event System Approaches](#1-event-system-approaches)
- [2. Hardcoded Coupling vs Observer Pattern](#2-hardcoded-coupling-vs-observer-pattern)
- [3. Command Queue Implementations](#3-command-queue-implementations)
- [4. How Event Bus Enables Decoupled Subsystems](#4-how-event-bus-enables-decoupled-subsystems)
- [5. Recommendations](#5-recommendations)

---

## 1. Event System Approaches

### Overview of Implemented PoCs

| PoC | Pattern | Use Case | Thread-Safe |
|-----|---------|----------|-------------|
| CommandQueue | Command Queue + `std::queue` | Thread-to-thread messaging | ✅ Yes (mutex) |
| CircularBuffer | Command Queue + Circular Buffer | Memory-efficient messaging | ✅ Yes (mutex) |
| EventSystem/EventBus | Observer Pattern | System-to-system events | ✅ Yes |
| EventSystem/Hardcoded | Direct Function Calls | Simple control flow | N/A |

---

## 2. Hardcoded Coupling vs Observer Pattern

### Hardcoded Function Calls (Direct Coupling)

**Concept**: The Game class directly calls system methods in a predetermined order.

```cpp
class Game {
    PhysicsSystem _physics;   // Direct dependency
    AudioSystem _audio;       // Direct dependency
    RenderSystem _render;     // Direct dependency

    void update() {
        _physics.checkCollisions(_registry);  // Hardcoded call
        _audio.update(_registry);             // Hardcoded call
        _render.draw(_registry);              // Hardcoded call
    }
};
```

**Architecture**:
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

| Aspect | Assessment |
|--------|------------|
| Coupling | 🔴 HIGH — Game knows all systems |
| Flexibility | 🔴 LOW — Adding system = modify Game |
| Performance | 🟢 EXCELLENT — Direct calls, inlinable |
| Debugging | 🟢 EASY — Clear call stack |
| Testing | 🔴 HARD — Cannot isolate systems |

---

### Observer Pattern (Event Bus)

**Concept**: Systems communicate through a central Event Bus. Publishers emit events without knowing subscribers.

```cpp
class Game {
    EventBus _eventBus;  // Single dependency

    void initialize() {
        _physics.initialize(_eventBus);  // Subscribes to events
        _audio.initialize(_eventBus);    // Subscribes to events
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

**Architecture**:
```
┌──────────────┐
│     Game     │
└──────┬───────┘
       │
       ▼
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

| Aspect | Assessment |
|--------|------------|
| Coupling | 🟢 LOW — Systems only know EventBus |
| Flexibility | 🟢 HIGH — Add/remove systems dynamically |
| Performance | 🟡 GOOD — Small overhead (~1-5%) |
| Debugging | 🟡 MEDIUM — Indirect control flow |
| Testing | 🟢 EASY — Isolate with mock EventBus |

---

### Side-by-Side Comparison

| Criteria | Hardcoded Calls | Event Bus |
|----------|-----------------|-----------|
| Add new system | Modify Game class | Just subscribe |
| Remove system | Modify Game class | Just unsubscribe |
| Runtime enable/disable | Conditional logic needed | Subscribe/unsubscribe |
| Compile-time deps | All systems | Only EventBus |
| Performance overhead | 0% | ~1-5% |
| Merge conflicts | High (Game.cpp bottleneck) | Low (independent files) |
| Network integration | Manual wiring | Events map to packets |

---

## 3. Command Queue Implementations

### PoC_CommandQueue (std::queue)

**Purpose**: Thread-safe message passing between threads (e.g., Network → Graphics).

**Implementation**:
```cpp
class ACommand {
    std::queue<Message> _commands;  // FIFO queue
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

**Pros**: Simple, dynamic memory allocation handles any message count.

**Cons**: Memory allocations on push, potential fragmentation over time.

---

### PoC_CircularBuffer (Fixed-size buffer)

**Purpose**: Same as CommandQueue but with bounded memory and better cache performance.

**Implementation**:
```cpp
class ACommand {
    rtype::network::CircularBuffer _buffer;  // Fixed-size circular buffer
    std::mutex _mutex;

    void addNewCommand(const Message& cmd) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto serialized = cmd.serialize();
        uint32_t size = serialized.size();
        _buffer.write(sizeBytes);
        _buffer.write(serialized);
    }

    void execute(Game& game) {
        std::lock_guard<std::mutex> lock(_mutex);
        while (_buffer.size() >= sizeof(uint32_t)) {
            auto sizeBytes = _buffer.read(sizeof(uint32_t));
            uint32_t msgSize = deserialize(sizeBytes);
            auto msgData = _buffer.read(msgSize);
            Message msg = Message::deserialize(msgData);
            process(msg);
        }
    }
};
```

**Architecture**:
```
┌─────────────────┐    serialize    ┌──────────────────────────────────┐
│     Message     │ ──────────────► │        CircularBuffer            │
│ (type, content, │                 │  [size][data][size][data]...     │
│  uid)           │ ◄────────────── │  ▲ tail              head ▲      │
└─────────────────┘   deserialize   └──────────────────────────────────┘
```

**Pros**:
- No dynamic allocation after init
- Better cache locality
- Bounded memory usage
- Faster than std::queue for high throughput

**Cons**:
- Fixed capacity (must choose wisely)
- Requires serialization/deserialization
- Oldest messages dropped if buffer full

---

### Command Queue Comparison

| Criteria | std::queue | CircularBuffer |
|----------|------------|----------------|
| Memory allocation | Per message | None (pre-allocated) |
| Memory bound | Unlimited | Fixed capacity |
| Cache performance | Poor (fragmented) | Excellent (contiguous) |
| Complexity | Simple | Requires serialization |
| Overflow handling | None (grows) | Overwrites oldest |
| Best for | Variable load | Steady high throughput |

---

## 4. How Event Bus Enables Decoupled Subsystems

### The "Decoupled Subsystems" Requirement

For R-Type, we need:
1. **Independent development** — Multiple developers can work on systems without conflicts
2. **Runtime flexibility** — Enable/disable systems without recompilation
3. **Testability** — Unit test each system in isolation
4. **Network integration** — Events translate naturally to network packets

### How Event Bus Achieves This

#### 1. No Direct Dependencies

```cpp
// ❌ Hardcoded: PhysicsSystem must know about AudioSystem
class PhysicsSystem {
    AudioSystem& _audio;  // Direct dependency!
    void onCollision() {
        _audio.playSound("hit.wav");  // Tight coupling
    }
};

// ✅ Event Bus: PhysicsSystem knows nothing about AudioSystem
class PhysicsSystem {
    EventBus& _bus;
    void onCollision() {
        _bus.publish(CollisionEvent{...});  // Fire and forget
    }
};
```

#### 2. Independent Development

```
Developer A: Creates PhysicsSystem
  └── Publishes: CollisionEvent, GravityEvent
  └── Subscribes to: nothing

Developer B: Creates AudioSystem
  └── Publishes: nothing
  └── Subscribes to: CollisionEvent, ExplosionEvent

Developer C: Creates NetworkSystem
  └── Publishes: PlayerJoinEvent, PacketReceivedEvent
  └── Subscribes to: CollisionEvent (to sync clients)

→ No merge conflicts! Each developer works in their own files.
```

#### 3. Runtime Flexibility

```cpp
// Add DLC systems at runtime
if (dlcAvailable("expansion")) {
    auto newSystem = std::make_unique<ExpansionSystem>(_eventBus);
    newSystem->initialize();  // Subscribes to existing events
    _systems.push_back(std::move(newSystem));
}

// Disable system temporarily
_audioSystem.unsubscribeAll();  // Mute the game
// ...
_audioSystem.resubscribe();     // Unmute
```

#### 4. Isolated Testing

```cpp
TEST(AudioSystem, PlaysCollisionSound) {
    EventBus mockBus;
    AudioSystem audio(mockBus);

    bool soundPlayed = false;
    audio.onSoundPlayed = [&](auto) { soundPlayed = true; };

    mockBus.publish(CollisionEvent{entity1, entity2});

    EXPECT_TRUE(soundPlayed);
}
// No Game, no Physics, no Rendering needed!
```

#### 5. Network Integration

```cpp
// Events naturally map to network packets
class NetworkSystem {
    void initialize(EventBus& bus) {
        bus.subscribe<CollisionEvent>([this](const CollisionEvent& e) {
            Packet p = serialize(e);
            broadcastToClients(p);
        });
    }
};

// Remote events become local events
void onPacketReceived(Packet& p) {
    if (p.type == COLLISION) {
        CollisionEvent e = deserialize(p);
        _eventBus.publish(e);  // Local systems react
    }
}
```

---

## 5. Recommendations

### Final Architecture Decision

| Component | Chosen Approach | Rationale |
|-----------|-----------------|-----------|
| System-to-System | **Event Bus** | Decoupling, testability, team dev |
| Thread-to-Thread | **CircularBuffer** | Performance, bounded memory |
| Critical path | **Hardcoded** (selective) | Zero overhead for hot loops |

### Hybrid Architecture

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

### Summary

- **Event Bus** for system communication → enables decoupled subsystems
- **CircularBuffer** for network→graphics thread messaging → bounded memory, high performance
- **Hardcoded calls** only for performance-critical inner loops (e.g., physics solver)

This hybrid approach gives us the best of both worlds: **flexibility where we need it, performance where it matters**.

---

## References

- `PoC/PoC_EventSystem/CommandQueue/` — Command Queue with std::queue
- `PoC/PoC_EventSystem/CircularBuffer/` — Command Queue with Circular Buffer
- `PoC/PoC_EventSystem/EventBus/` — Event Bus implementation
- `PoC/PoC_EventSystem/HardcodedFunctionCalls/` — Hardcoded approach
- `PoC/PoC_EventSystem/event_system_comparison.md` — Detailed comparison
- `PoC/PoC_EventSystem/event_system_decision.md` — Final decision document
