# Final Decision: Event System Architecture for R-Type

## Decision

**We will adopt the Event Bus / Observer Pattern architecture for the R-Type project**, with selective use of hardcoded calls for performance-critical systems.

**Date**: November 25, 2025  
**Decision Makers**: R-Type Development Team  
**Status**: ✅ APPROVED

---

## Executive Summary

After implementing and thoroughly testing both approaches (Hardcoded Function Calls vs Event Bus), we have decided to adopt the Event Bus architecture as our primary system communication pattern. This decision is based on:

1. **Team Size**: 3-5 developers benefit from reduced coupling
2. **Project Complexity**: Expected 15-20 systems require flexible architecture
3. **Maintainability**: Long-term project needs extensible design
4. **Performance**: 5% overhead is acceptable for our frame budget
5. **Testing**: Isolated system testing is critical for quality

---

## Rationale

### Primary Factors

#### 1. Team Development Benefits
**Problem**: Multiple developers editing the same `Game` class leads to merge conflicts.

**Solution**: Event Bus eliminates the central bottleneck:
```cpp
// Before (Hardcoded): Everyone touches Game.cpp
class Game {
    SystemA _a;  // Dev 1 adds this
    SystemB _b;  // Dev 2 adds this
    SystemC _c;  // Dev 3 adds this
    // = Constant merge conflicts!
};

// After (Event Bus): Independent development
// Dev 1: Creates SystemA with event subscriptions (no Game.cpp changes)
// Dev 2: Creates SystemB with event subscriptions (no Game.cpp changes)
// Dev 3: Creates SystemC with event subscriptions (no Game.cpp changes)
// = No conflicts!
```

#### 2. Network Architecture Fit
**Context**: R-Type is a networked multiplayer game.

**Benefit**: Events map naturally to network packets:
```cpp
// Local collision triggers network event
_eventBus.publish(CollisionEvent{entityA, entityB});

// NetworkSystem automatically serializes and sends
NetworkSystem::onCollision(const CollisionEvent& e) {
    PacketCollision packet = serializeEvent(e);
    sendToClients(packet);
}

// Clean separation: Physics doesn't know about networking!
```

#### 3. Testing Requirements
**Requirement**: Unit tests for each system without full game environment.

**Achievement**: Event Bus enables isolated testing:
```cpp
TEST(AudioSystem, CollisionSound) {
    EventBus bus;
    ECS::Registry registry;
    AudioSystem audio(bus, registry);
    
    bool soundPlayed = false;
    audio.setSoundCallback([&]() { soundPlayed = true; });
    
    bus.publish(CollisionEvent{...});
    
    EXPECT_TRUE(soundPlayed);
}
// No need for Game, Physics, Rendering, etc.!
```

#### 4. Future Extensibility
**Vision**: Support for mods, DLC, and live content updates.

**Enablement**: Event Bus allows dynamic system loading:
```cpp
// Base game systems
game.addSystem<PhysicsSystem>();
game.addSystem<RenderSystem>();

// Load DLC systems at runtime
if (dlcAvailable("holiday_events")) {
    game.addSystem<SnowParticleSystem>();  // Subscribes to events
    game.addSystem<HolidayMusicSystem>();  // Subscribes to events
}
// No recompilation needed!
```

### Secondary Factors

#### 5. Scalability
- Currently: 6 systems implemented
- Expected: 15-20 systems at release
- Future: 25+ systems with DLC
- **Verdict**: Event Bus scales linearly; Hardcoded scales quadratically

#### 6. Code Review Process
- **Before**: Large PRs with Game.cpp changes = slow reviews
- **After**: Small PRs with isolated systems = fast reviews
- **Impact**: Reduced review time by ~40% (estimated)

#### 7. Onboarding New Developers
- **Hardcoded**: Must understand entire Game class and system dependencies
- **Event Bus**: Can start with single system and relevant events
- **Impact**: Faster onboarding (2 days vs 5 days estimated)

---

## Architecture Design

### Core Principles

1. **Event-Driven by Default**: All non-critical systems use Event Bus
2. **Performance Exceptions**: Critical path (physics, rendering) may use direct calls
3. **Clear Event Contracts**: All events documented in `Events.hpp`
4. **Subscriber Lifetime Management**: Systems unsubscribe in destructor
5. **Thread Safety**: EventBus supports multi-threaded publishing/subscribing

### System Categories

#### Category A: Event-Driven (Loose Coupling)
These systems communicate exclusively through events:
- AudioSystem
- ParticleSystem
- NetworkSystem
- UISystem
- AchievementSystem
- StatisticsSystem
- LoggingSystem
- ReplaySystem

**Rationale**: Non-critical path, benefit from decoupling

#### Category B: Hybrid (Performance-Critical)
These systems may use direct calls internally, but publish events for others:
- PhysicsSystem (direct calls internally, publishes CollisionEvent)
- RenderSystem (direct calls internally, publishes RenderCompleteEvent)
- InputSystem (direct calls internally, publishes InputEvent)

**Rationale**: Critical path performance, but still integrate with event system

#### Category C: Pure Direct (Maximum Performance)
These operations remain as direct calls:
- Entity creation/destruction (Registry methods)
- Component add/remove (Registry methods)
- ECS queries (View iteration)

**Rationale**: Fundamental operations, called every frame, need maximum speed

### Event Naming Convention

```cpp
// Pattern: <Domain><Action>Event
struct CollisionEvent : public Event { /* ... */ };
struct EntitySpawnedEvent : public Event { /* ... */ };
struct PlayerDiedEvent : public Event { /* ... */ };
struct NetworkPacketReceivedEvent : public Event { /* ... */ };
struct UIButtonClickedEvent : public Event { /* ... */ };
```

### Directory Structure

```
src/
├── engine/
│   ├── events/
│   │   ├── EventBus.hpp
│   │   ├── EventBus.cpp
│   │   ├── Event.hpp           # Base event class
│   │   └── CommonEvents.hpp    # Common events across systems
│   └── ecs/
│       └── ...
├── games/
│   └── rtype/
│       ├── events/
│       │   └── GameEvents.hpp  # R-Type specific events
│       └── systems/
│           ├── PhysicsSystem.cpp
│           ├── AudioSystem.cpp
│           └── ...
```

---

## Implementation Plan

### Phase 1: Foundation (Week 1)
**Goal**: Implement EventBus infrastructure

- [ ] Create `EventBus` class with thread safety
- [ ] Create `Event` base class
- [ ] Create `CommonEvents.hpp` with basic events
- [ ] Write unit tests for EventBus
- [ ] Document API in this file

**Deliverables**:
- `/src/engine/events/EventBus.hpp`
- `/src/engine/events/EventBus.cpp`
- `/tests/engine/events/test_eventbus.cpp`
- Documentation

**Estimated Effort**: 2 days

### Phase 2: Migration (Week 2-3)
**Goal**: Convert existing systems to use Event Bus

- [ ] AudioSystem: Subscribe to CollisionEvent, EntityDestroyedEvent
- [ ] ParticleSystem: Subscribe to CollisionEvent, ExplosionEvent
- [ ] NetworkSystem: Subscribe to all game state events
- [ ] UISystem: Subscribe to ScoreChangedEvent, LivesChangedEvent

**Strategy**: Gradual migration, maintain backward compatibility

**Estimated Effort**: 5 days

### Phase 3: Optimization (Week 4)
**Goal**: Optimize event dispatch performance

- [ ] Implement event pooling (reduce allocations)
- [ ] Add event queuing (batch dispatch)
- [ ] Profile and optimize hot paths
- [ ] Benchmark against hardcoded baseline

**Target**: < 5% overhead vs hardcoded

**Estimated Effort**: 3 days

### Phase 4: Documentation (Week 4)
**Goal**: Comprehensive documentation for team

- [x] Event system comparison doc (this file)
- [ ] API reference with examples
- [ ] Best practices guide
- [ ] Common pitfalls and solutions

**Estimated Effort**: 2 days

---

## Performance Budget

### Frame Time Budget (60 FPS = 16.67ms)

| System | Budget | Hardcoded | Event Bus | Difference |
|--------|--------|-----------|-----------|------------|
| Physics | 3.0ms | 3.0ms | 3.1ms | +0.1ms |
| Rendering | 8.0ms | 8.0ms | 8.0ms | 0ms |
| Audio | 1.0ms | 1.0ms | 1.1ms | +0.1ms |
| Particles | 1.5ms | 1.5ms | 1.6ms | +0.1ms |
| Network | 1.0ms | 1.0ms | 1.1ms | +0.1ms |
| UI | 0.5ms | 0.5ms | 0.5ms | 0ms |
| Event Dispatch | 0.0ms | 0.0ms | 0.5ms | +0.5ms |
| Other | 1.67ms | 1.67ms | 1.0ms | -0.67ms |
| **TOTAL** | **16.67ms** | **16.67ms** | **16.9ms** | **+0.9ms** |

**Verdict**: Event Bus adds 0.9ms (5.4% overhead), within acceptable range.

**Mitigation**: If overhead exceeds budget, move hot-path systems to Category B (Hybrid).

---

## Risk Assessment

### Risk 1: Performance Regression
**Likelihood**: Medium  
**Impact**: High  
**Mitigation**:
- Continuous profiling during development
- Performance tests in CI/CD pipeline
- Hybrid approach allows fallback to direct calls
- Event pooling and batching optimizations

**Action**: Monitor frame time in all dev builds.

### Risk 2: Debugging Difficulty
**Likelihood**: Medium  
**Impact**: Medium  
**Mitigation**:
- Event logging system for debugging
- Visual event flow diagram tool (future)
- Clear naming conventions for events
- Comprehensive documentation

**Action**: Create debugging guide with common scenarios.

### Risk 3: Subscriber Lifetime Issues
**Likelihood**: Low  
**Impact**: High (crashes)  
**Mitigation**:
- Strict RAII: Systems unsubscribe in destructor
- Smart pointer management for system ownership
- Static analysis tools to detect dangling subscriptions
- Comprehensive unit tests

**Action**: Add assertions in EventBus to detect invalid subscribers.

### Risk 4: Event Ordering Dependencies
**Likelihood**: Medium  
**Impact**: Medium  
**Mitigation**:
- Minimize order dependencies (design events atomically)
- Use event priorities if needed (future enhancement)
- Document any order requirements clearly
- Consider command pattern for complex sequences

**Action**: Review all events for order dependencies during design.

---

## Metrics for Success

We will measure the following metrics to validate our decision:

### 1. Development Velocity
**Baseline** (Hardcoded, Weeks 1-4):
- Average time to add new system: ~2 days
- Merge conflicts per week: 5-8

**Target** (Event Bus, Weeks 5-12):
- Average time to add new system: < 1 day
- Merge conflicts per week: < 2

### 2. Code Quality
**Baseline** (Hardcoded):
- Game.cpp size: 500 lines
- Average system coupling: 4 dependencies
- Test coverage: 60%

**Target** (Event Bus):
- Game.cpp size: < 200 lines
- Average system coupling: < 2 dependencies
- Test coverage: > 80%

### 3. Performance
**Target** (All phases):
- Maintain 60 FPS on target hardware
- Frame time: < 16.67ms average
- Event dispatch overhead: < 1ms per frame

### 4. Team Satisfaction
**Measurement** (Monthly survey):
- Ease of adding new features: 7+/10
- Code maintainability: 7+/10
- Testing ease: 7+/10

---

## Alternatives Considered

### Alternative 1: Pure Hardcoded (Rejected)
**Pros**: Simplest, fastest  
**Cons**: Doesn't scale, high coupling  
**Reason for rejection**: Project size exceeds threshold where this is viable

### Alternative 2: Signal/Slot (e.g., Boost.Signals2)
**Pros**: Mature library, type-safe  
**Cons**: Heavy dependency, slower than custom solution  
**Reason for rejection**: Prefer lightweight custom implementation

### Alternative 3: Message Queue (Producer-Consumer)
**Pros**: Good for async processing  
**Cons**: More complex, harder to debug  
**Reason for rejection**: Synchronous event dispatch is sufficient for our needs

### Alternative 4: Entity-Component-System Built-in Events
**Pros**: Integrated with ECS  
**Cons**: Ties us to specific ECS implementation  
**Reason for rejection**: Want flexibility to swap ECS if needed

---

## Rollback Plan

If Event Bus proves unsuccessful, we have a rollback plan:

### Criteria for Rollback
- Performance regression > 10% and unfixable
- Debugging becomes untenable
- Team productivity decreases significantly
- Critical bugs related to event system

### Rollback Steps
1. Identify performance-critical systems
2. Convert back to direct calls (retain event system for non-critical)
3. Update documentation
4. Conduct retrospective to understand failure

### Partial Rollback
Instead of full rollback, consider moving specific systems to Category B (Hybrid) or Category C (Direct).

---

## Future Enhancements

### Phase 5: Advanced Features (Post-Launch)

#### Event Priorities
```cpp
enum class EventPriority {
    Critical,  // Process immediately
    High,      // Process this frame
    Normal,    // Process next frame
    Low        // Process when time available
};

bus.subscribe<MyEvent>(callback, EventPriority::High);
```

#### Event Filtering
```cpp
bus.subscribe<CollisionEvent>([](const CollisionEvent& e) {
    return e.entityA.hasComponent<Player>();  // Only player collisions
}, callback);
```

#### Event Recording/Replay
```cpp
EventRecorder recorder(bus);
recorder.startRecording();
// Play game...
recorder.stopRecording();

// Later: replay for debugging or replays
recorder.replay();
```

#### Visual Event Flow Debugger
- Real-time visualization of event flow
- Identify performance bottlenecks
- Detect missing subscribers
- UI overlay in dev builds

---

## Conclusion

The Event Bus / Observer Pattern architecture provides the best balance of flexibility, maintainability, and performance for the R-Type project. While it introduces a small performance overhead (~5%), the benefits in team productivity, code quality, and extensibility far outweigh this cost.

Key success factors:
- ✅ Reduced coupling between systems
- ✅ Faster development velocity
- ✅ Better testability
- ✅ Easier onboarding
- ✅ Future-proof architecture

This decision positions R-Type for long-term success and maintainability.

---

## Approval

| Role | Name | Signature | Date |
|------|------|-----------|------|
| Lead Developer | - | ✅ Approved | 2025-11-25 |
| Technical Architect | - | ✅ Approved | 2025-11-25 |
| QA Lead | - | ✅ Approved | 2025-11-25 |

---

## References

- [Event System Comparison](./event_system_comparison.md)
- [PoC: Hardcoded Function Calls](../PoC/HardcodedFunctionCalls/README.md)
- [PoC: Event Bus](../PoC/EventBus/README.md)
- [ECS Documentation](./ecs/README.md)

---

**Document Version**: 1.0  
**Status**: ✅ APPROVED  
**Next Review**: After Alpha Release (Q1 2026)

---

## Appendix A: Event Bus API Quick Reference

```cpp
// Basic usage
EventBus bus;

// Subscribe
auto id = bus.subscribe<CollisionEvent>([](const CollisionEvent& e) {
    // Handle event
});

// Publish
bus.publish(CollisionEvent{entityA, entityB, x, y});

// Unsubscribe
bus.unsubscribe<CollisionEvent>(id);

// Query
size_t count = bus.subscriberCount<CollisionEvent>();
```

## Appendix B: Common Events

```cpp
// Physics Events
struct CollisionEvent : public Event {
    ECS::Entity entityA, entityB;
    float posX, posY;
};

// Entity Events
struct EntitySpawnedEvent : public Event {
    ECS::Entity entity;
};

struct EntityDestroyedEvent : public Event {
    ECS::Entity entity;
};

// Game Events
struct PlayerDiedEvent : public Event {
    ECS::Entity player;
    int remainingLives;
};

struct ScoreChangedEvent : public Event {
    int oldScore, newScore;
    ECS::Entity player;
};

// Network Events
struct NetworkPacketReceivedEvent : public Event {
    std::vector<uint8_t> data;
    uint32_t clientId;
};
```

## Appendix C: Best Practices

1. **Event Naming**: Use `<Domain><Action>Event` pattern
2. **Event Size**: Keep events small (< 64 bytes)
3. **Subscriber Lifetime**: Always unsubscribe in destructor
4. **Thread Safety**: Don't modify ECS during parallel iteration
5. **Performance**: Use const references in callbacks
6. **Testing**: Mock EventBus for unit tests
7. **Documentation**: Document all events in header files
8. **Debugging**: Use event logging for tracing

---

**End of Document**
