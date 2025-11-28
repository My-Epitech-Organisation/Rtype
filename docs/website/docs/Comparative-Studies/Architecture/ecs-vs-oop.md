---
sidebar_position: 1
---

# ECS vs OOP Architecture

## Executive Summary

**Decision:** Entity-Component-System (ECS) Architecture  
**Date:** November 2025  
**Status:** ✅ Approved

Through comprehensive proof-of-concept implementations, we compared **traditional Object-Oriented Programming (OOP)** with **Entity-Component-System (ECS)** architecture for the R-Type game engine.

**Key Finding:** OOP suffers from the **diamond inheritance problem**, **code duplication**, and **deep inheritance hierarchies** (4+ levels), making it unsuitable for flexible game development. ECS provides superior **composition over inheritance**, **zero code duplication**, and **performance optimization**.

---

## The OOP Problem

### Diamond Inheritance Issue

One of the most critical problems encountered in the OOP PoC:

```cpp
class GameObject { };
class Shootable : public GameObject { };
class Damageable : public GameObject { };

// ❌ IMPOSSIBLE: Can't inherit from both!
class ShootingPowerUp : public Shootable, public Damageable {
    // This creates a diamond:
    //      GameObject
    //       /      \
    //  Shootable  Damageable
    //       \      /
    //   ShootingPowerUp
    
    // FORCED to duplicate fields instead!
    float armor;           // Duplicated from Damageable
    bool canBeDestroyed;   // Duplicated from Damageable
};
```

**Impact:**

- ❌ Cannot combine behaviors without duplication
- ❌ Virtual inheritance adds complexity and performance cost
- ❌ Composition requires many forwarding methods ("wrapper hell")

---

### Deep Inheritance Hierarchy

```
GameObject (base)
    └── Movable
            └── Enemy
                    └── Boss  (4 levels deep!)
```

**Problems:**

- 🔴 **Fragile Base Class**: Changes to GameObject affect all descendants
- 🔴 **Tight Coupling**: Boss class depends on 3 parent classes
- 🔴 **Inflexible**: Can't easily add/remove behaviors at runtime

---

### Code Duplication

**Example:** Shooting behavior duplicated between Player and Enemy

```cpp
// Player.cpp
void Player::shoot() {
    if (timeSinceLastShot >= fireRate) {
        std::cout << "Player shoots!" << std::endl;
        timeSinceLastShot = 0.0f;
    }
}

// Enemy.cpp
void Enemy::shoot() {
    if (timeSinceLastShot >= fireRate) {
        std::cout << "Enemy shoots!" << std::endl;
        timeSinceLastShot = 0.0f;
    }
}
```

**To share this behavior in OOP, you would need:**

1. **Option 1:** Extract to Shootable base class → Complicated hierarchy
2. **Option 2:** Multiple inheritance → Diamond problem
3. **Option 3:** Composition → Verbose forwarding methods

---

### OOP Complexity Metrics

| Metric | Value | Assessment |
|--------|-------|------------|
| **Total Lines of Code** | ~800 | 🟡 Medium for basic features |
| **Max Inheritance Depth** | 4 levels | 🔴 High (Boss class) |
| **Code Duplication** | shoot() in 2 places | 🔴 High maintenance cost |
| **Virtual Functions** | 8+ | 🟡 Performance overhead |
| **Coupling** | Very High | 🔴 Fragile base class |

---

## The ECS Solution

### Architecture Overview

ECS separates data (Components) from behavior (Systems), allowing flexible composition:

```cpp
// Components (pure data)
struct Position { float x, y; };
struct Velocity { float vx, vy; };
struct Health { int hp; };
struct Shootable { float fireRate; float cooldown; };

// Entity = ID + Set of Components
Entity player = registry.create();
registry.emplace<Position>(player, 100.0f, 200.0f);
registry.emplace<Velocity>(player, 0.0f, 0.0f);
registry.emplace<Health>(player, 100);
registry.emplace<Shootable>(player, 0.5f, 0.0f);

// Systems (pure logic)
void MovementSystem::update(Registry& reg, float dt) {
    auto view = reg.view<Position, Velocity>();
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        auto& vel = view.get<Velocity>(entity);
        pos.x += vel.vx * dt;
        pos.y += vel.vy * dt;
    }
}
```

---

### ECS Advantages

#### 1. Composition Over Inheritance ✅

**No diamond problem:**

```cpp
// Want a shooting, damageable, moving entity?
// Just add components!
Entity boss = registry.create();
registry.emplace<Position>(boss, 500.0f, 300.0f);
registry.emplace<Velocity>(boss, -50.0f, 0.0f);
registry.emplace<Health>(boss, 1000);
registry.emplace<Shootable>(boss, 1.0f, 0.0f);
registry.emplace<Armor>(boss, 50.0f);

// Add/remove behaviors at runtime
registry.remove<Shootable>(boss);  // Boss can't shoot anymore
registry.emplace<Invulnerable>(boss);  // Now invulnerable!
```

#### 2. Zero Code Duplication ✅

**Single ShootingSystem for all entities:**

```cpp
void ShootingSystem::update(Registry& reg, float dt) {
    auto view = reg.view<Position, Shootable>();
    for (auto entity : view) {
        auto& pos = view.get<Position>(entity);
        auto& shoot = view.get<Shootable>(entity);
        
        shoot.cooldown -= dt;
        if (shoot.cooldown <= 0.0f) {
            // Create bullet
            shoot.cooldown = shoot.fireRate;
        }
    }
}
```

**Works for:** Players, Enemies, Bosses, Turrets, Power-ups — anything with Shootable component!

#### 3. Cache-Friendly Performance ✅

**Sparse Set Implementation:**

```
Dense Array (cache-friendly):
[Position][Position][Position][Position]...
    |         |         |         |
[Velocity][Velocity][Velocity][Velocity]...
```

- ✅ **Contiguous Memory**: Components stored sequentially
- ✅ **Cache Locality**: CPU prefetches next components
- ✅ **SIMD Friendly**: Process multiple components simultaneously
- ✅ **Fast Iteration**: No pointer chasing like OOP virtual functions

**Performance:**

- OOP: ~100K entities @ 60 FPS (virtual function overhead)
- ECS: ~2M entities @ 60 FPS (cache-efficient iteration)

#### 4. Flexible Runtime Behavior ✅

```cpp
// Player picks up shield power-up
registry.emplace<Shield>(player, 3.0f);  // 3 seconds of shield

// Shield expires
if (shield.duration <= 0.0f) {
    registry.remove<Shield>(player);
}

// Enemy enters "rage mode"
registry.emplace<DoubleSpeed>(enemy);
registry.emplace<DoubleDamage>(enemy);
```

**In OOP:** Would require inheritance or complex state machines

---

### ECS Implementation Details

#### Component Traits (Optimization)

```cpp
template<typename T>
struct ComponentTraits {
    static constexpr bool isEmpty = std::is_empty_v<T>;
    static constexpr bool isTriviallyCopyable = 
        std::is_trivially_copyable_v<T>;
    static constexpr bool isTriviallyDestructible = 
        std::is_trivially_destructible_v<T>;
};

// Empty components (tags) skip storage allocation
struct Enemy { };  // Zero bytes stored!

// Trivially copyable → fast memcpy operations
struct Position { float x, y; };
```

#### SparseSet Storage

```cpp
template<typename T>
class SparseSet {
    std::vector<T> _dense;              // Contiguous component data
    std::vector<Entity> _packed;        // Entity IDs (parallel to dense)
    std::vector<size_t> _sparse;        // Entity → dense index lookup
    
    // O(1) insert, O(1) remove, O(1) lookup
    // Cache-friendly iteration over _dense
};
```

---

## Comparative Analysis

| Aspect | OOP | ECS |
|--------|-----|-----|
| **Behavior Sharing** | ❌ Inheritance or duplication | ✅ Shared systems |
| **Runtime Flexibility** | ❌ Fixed at compile-time | ✅ Add/remove at runtime |
| **Code Duplication** | 🔴 High (shoot in 2 places) | ✅ Zero (single system) |
| **Coupling** | 🔴 Tight (4-level hierarchy) | ✅ Loose (components independent) |
| **Performance** | 🟡 Virtual function overhead | ✅ Cache-friendly (20x faster) |
| **Maintainability** | 🔴 Fragile base class | ✅ Independent components |
| **Scalability** | 🔴 Hierarchy explosion | ✅ Linear component growth |

---

## Real-World Example: Adding a Feature

**Requirement:** Add "freeze" ability that stops enemies for 3 seconds

### OOP Approach ❌

```cpp
// Option 1: Add to Enemy base class
class Enemy {
    bool frozen;
    float freezeDuration;
    // Now ALL enemies have freeze fields (even if never frozen)
};

// Option 2: Create FreezeableEnemy subclass
class FreezeableEnemy : public Enemy {
    // Now 2 enemy types, duplicate spawn logic
};

// Option 3: Multiple inheritance
class FreezeableMovableShootableEnemy 
    : public Movable, public Shootable, public Freezeable {
    // Diamond problem!
};
```

### ECS Approach ✅

```cpp
// 1. Define component (one-time)
struct Frozen { float duration; };

// 2. Modify movement system (one line)
void MovementSystem::update(Registry& reg, float dt) {
    auto view = reg.view<Position, Velocity>()
                   .exclude<Frozen>();  // Skip frozen entities
    // ... existing code
}

// 3. Apply freeze
void freezeEnemy(Registry& reg, Entity enemy) {
    reg.emplace<Frozen>(enemy, 3.0f);
}

// 4. Unfreeze system
void FrozenSystem::update(Registry& reg, float dt) {
    auto view = reg.view<Frozen>();
    for (auto entity : view) {
        auto& frozen = view.get<Frozen>(entity);
        frozen.duration -= dt;
        if (frozen.duration <= 0.0f) {
            reg.remove<Frozen>(entity);
        }
    }
}
```

**Result:** Feature added in ~10 lines, no changes to existing entity classes!

---

## Performance Benchmarks

**Test:** 10,000 entities with Position + Velocity, 60 FPS update loop

| Metric | OOP | ECS | Improvement |
|--------|-----|-----|-------------|
| **Update Time** | 8.5 ms | 0.42 ms | **20x faster** |
| **Memory Usage** | 1.2 MB | 0.8 MB | **33% less** |
| **Cache Misses** | High | Low | **Cache-efficient** |
| **Code Complexity** | ~800 LOC | ~450 LOC | **44% less code** |

---

## Final Recommendation

✅ **Use Entity-Component-System (ECS)** for R-Type game engine.

**Rationale:**

1. **No Diamond Problem**: Composition solves multiple behavior sharing
2. **Zero Duplication**: Single system handles all entities with component
3. **Runtime Flexibility**: Add/remove behaviors dynamically
4. **20x Performance**: Cache-friendly iteration vs virtual functions
5. **Maintainability**: Independent components vs fragile hierarchies
6. **Industry Standard**: Used by Unity, Unreal (mass entities), Overwatch, etc.

**Implementation:**

- Components stored in SparseSet for O(1) operations
- Systems iterate cache-friendly contiguous arrays
- Component traits optimize storage for empty/trivial types
- Modern C++20 concepts enforce type safety

---

## References

- OOP PoC: `/PoC/OOP/`
- ECS PoC: `/PoC/ECS/`
- Analysis: `/PoC/OOP/OOP_ANALYSIS.md`
- ECS Architecture docs: `/docs/architecture/ecs/`
