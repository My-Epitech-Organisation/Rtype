# Movement System Architecture

Comparative analysis of movement patterns for game entities.

## Executive Summary

After evaluating four movement prototypes, we selected a **hybrid multi-system architecture** combining:

1. **Linear Movement** - Critical (bullets, simple enemies)
2. **Sine Wave Movement** - High priority (classic R-Type patterns)
3. **Scripted Movement** - High priority (bosses, complex behaviors)
4. **Bezier Curves** - Optional (cinematics, polish)

---

## Movement Systems Comparison

| System | Priority | Complexity | Performance | Use Case |
|--------|----------|------------|-------------|----------|
| **Linear** | Critical | Low | Excellent | Projectiles |
| **Sine Wave** | High | Low | Excellent | Enemy patterns |
| **Scripted** | High | Medium | Good | Boss behaviors |
| **Bezier** | Optional | High | Moderate | Cinematics |

---

## System 1: Linear Movement

### Priority: Critical

**Timeline**: Sprint 1

### Description

Foundation for all projectile-based gameplay. Simple velocity-based movement.

### Use Cases

- All bullet types
- Projectiles
- Particle effects
- Simple enemy charges
- Background elements

### Implementation

```cpp
struct Velocity {
    float dx, dy;  // Direction * Speed combined
};

void LinearMovementSystem::update(Registry& registry, float dt) {
    for (auto [entity, pos, vel] : registry.view<Position, Velocity>()) {
        pos.x += vel.dx * dt;
        pos.y += vel.dy * dt;
    }
}
```

### Performance

| Metric | Value |
|--------|-------|
| Complexity | O(n) |
| Operations per entity | 4 multiplications, 2 additions |
| Cache efficiency | Excellent |
| Memory per entity | 8 bytes (2 floats) |

### Rationale

- Highest performance characteristics
- Simple, predictable, reliable
- Essential for core gameplay

---

## System 2: Sine Wave Movement

### Priority: High

**Timeline**: Sprint 1

### Description

Creates the classic R-Type aesthetic with oscillating movement patterns.

### Use Cases

- Wave formation enemies
- Floating power-ups
- Background animations
- Enemy patrol patterns
- Classic shooter feel

### Implementation

```cpp
struct SineWave {
    float centerY;
    float frequency;
    float amplitude;
    float phase;
    float time;
};

void SineWaveMovementSystem::update(Registry& registry, float dt) {
    for (auto [entity, pos, wave] : registry.view<Position, SineWave>()) {
        wave.time += dt;
        pos.y = wave.centerY + wave.amplitude * std::sin(wave.frequency * wave.time + wave.phase);
    }
}
```

### Configuration Examples

| Pattern | Frequency | Amplitude | Effect |
|---------|-----------|-----------|--------|
| Gentle float | 1.0 | 20 | Power-ups |
| Standard wave | 2.0 | 40 | Normal enemies |
| Aggressive | 4.0 | 60 | Fast enemies |
| Tight wobble | 6.0 | 10 | Nervous movement |

### Performance

| Metric | Value |
|--------|-------|
| Complexity | O(n) |
| Operations | 1 sin(), 3 multiplications, 2 additions |
| Cache efficiency | Good |
| Memory per entity | 20 bytes (5 floats) |

### Rationale

- Classic R-Type aesthetic
- Excellent performance-to-visual ratio
- Simple to tune and adjust

---

## System 3: Scripted Movement

### Priority: High

**Timeline**: Sprint 2

### Description

Data-driven movement system using command sequences for complex patterns.

### Use Cases

- Boss movement patterns
- Multi-phase enemy behaviors
- Cutscene movements
- Tutorial sequences
- Complex attack choreography

### Implementation

```cpp
struct MovementScript {
    std::vector<std::unique_ptr<ICommand>> commands;
    size_t currentIndex;
};

// Command interface
class ICommand {
public:
    virtual bool execute(Position& pos, float dt) = 0;  // Returns true when done
    virtual void reset() = 0;
};

// Example: Move to point
class MoveToCommand : public ICommand {
    Vec2 target;
    float speed;
public:
    bool execute(Position& pos, float dt) override {
        Vec2 direction = normalize(target - Vec2{pos.x, pos.y});
        pos.x += direction.x * speed * dt;
        pos.y += direction.y * speed * dt;
        return distanceTo(target) < EPSILON;
    }
};
```

### Script Example (JSON)

```json
{
    "name": "boss_phase1",
    "commands": [
        { "type": "moveTo", "x": 800, "y": 300, "speed": 200 },
        { "type": "wait", "duration": 1.0 },
        { "type": "moveTo", "x": 600, "y": 100, "speed": 300 },
        { "type": "wait", "duration": 0.5 },
        { "type": "loop", "count": 3, "commands": [
            { "type": "moveTo", "x": 700, "y": 200, "speed": 400 },
            { "type": "moveTo", "x": 500, "y": 400, "speed": 400 }
        ]}
    ]
}
```

### Available Commands

| Command | Parameters | Description |
|---------|------------|-------------|
| `moveTo` | x, y, speed | Move to position |
| `wait` | duration | Pause movement |
| `loop` | count, commands | Repeat sequence |
| `setVelocity` | dx, dy | Direct velocity |
| `accelerate` | ax, ay, duration | Gradual speed change |

### Rationale

- Empowers designers to create content
- Rapid prototyping and iteration
- Data-driven for maintainability

---

## System 4: Bezier Curves (Optional)

### Priority: Medium

**Timeline**: Sprint 3-4 (Polish Phase)

### Description

Smooth curved paths using cubic Bezier curves for cinematic movement.

### Use Cases

- Boss entrance/exit cinematics
- Dramatic enemy entrances
- Key story moments
- Visual polish

### Implementation

```cpp
struct BezierPath {
    Vec2 p0, p1, p2, p3;  // Control points
    float t;              // Progress [0, 1]
    float duration;
};

Vec2 cubicBezier(const BezierPath& path, float t) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    
    return uuu * path.p0 +
           3 * uu * t * path.p1 +
           3 * u * tt * path.p2 +
           ttt * path.p3;
}
```

### Decision Criteria

Implement only if:
- Performance budget allows
- Visual editor tool available
- Art direction requires curves
- Core gameplay is complete

### Performance Consideration

| Metric | Bezier | Linear |
|--------|--------|--------|
| Operations | ~20 | ~4 |
| Complexity | Medium | Simple |
| Authoring | Complex | Easy |

---

## System Execution Order

```cpp
void GameLoop::updateMovement(float deltaTime) {
    // 1. Base movement (Linear)
    linearMovementSystem.update(registry, deltaTime);
    
    // 2. Modifiers (Sine Wave)
    sineWaveMovementSystem.update(registry, deltaTime);
    
    // 3. Scripted overrides
    scriptedMovementSystem.update(registry, deltaTime);
    
    // 4. Optional (Bezier) - Only if implemented
    // bezierMovementSystem.update(registry, deltaTime);
}
```

---

## Component Design

```cpp
namespace RType::Movement {
    // Shared base
    struct Position {
        float x, y;
    };

    // Linear movement
    struct Velocity {
        float dx, dy;
    };

    // Sine wave movement
    struct SineWave {
        float centerY;
        float frequency;
        float amplitude;
        float phase;
        float time;
    };

    // Scripted movement
    struct MovementScript {
        std::vector<std::unique_ptr<ICommand>> commands;
        size_t currentIndex;
    };
}
```

---

## Final Decision Summary

| System | Status | Sprint | Rationale |
|--------|--------|--------|-----------|
| **Linear** | Approved | 1 | Foundation for projectiles |
| **Sine Wave** | Approved | 1 | Classic R-Type aesthetic |
| **Scripted** | Approved | 2 | Designer empowerment |
| **Bezier** | Deferred | 3-4 | Polish phase only |

### Implementation Order

1. **Sprint 1**: Linear + Sine Wave (core gameplay)
2. **Sprint 2**: Scripted (boss patterns)
3. **Sprint 3-4**: Bezier (if time permits)

---

## References

- PoC implementations: `/PoC/PoC_Movement/`
- Linear Movement: `/PoC/PoC_Movement/LinearMovement/`
- Sine Wave: `/PoC/PoC_Movement/SineWaveMovement/`
- Scripted: `/PoC/PoC_Movement/ScriptedMovement/`
- Bezier: `/PoC/PoC_Movement/BezierMovement/`
- Decision document: `/PoC/PoC_Movement/movement_system_decision.md`
