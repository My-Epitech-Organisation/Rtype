---
sidebar_position: 1
---

# Memory Optimization Strategies

## Executive Summary

**Decision:** Implement Both Object Pool + Memory Pool  
**Date:** November 2025  
**Status:** ✅ Approved

After comprehensive evaluation of **Object Pool** and **Memory Pool** approaches, we decided to implement **both techniques** as they serve different purposes and provide complementary benefits.

**Key Finding:** Object Pool provides **8-15x performance** for entity spawning, while Memory Pool enables **frame-based allocations** with 98% time savings and predictable memory usage.

---

## The Problem

**Game entity lifecycle creates allocation pressure:**

```cpp
// Traditional approach (problematic)
while (gameRunning) {
    // Spawn bullets (60 Hz)
    for (int i = 0; i < 10; ++i) {
        bullets.push_back(new Bullet());  // ❌ Allocation
    }
    
    // Update (every frame)
    for (auto* bullet : bullets) {
        bullet->update(dt);
    }
    
    // Remove dead bullets
    for (auto* bullet : bullets) {
        if (bullet->isDead()) {
            delete bullet;  // ❌ Deallocation
        }
    }
}
```

**Issues:**

- 🔴 **Frequent allocations**: 600 allocations/second (10 bullets × 60 FPS)
- 🔴 **Memory fragmentation**: new/delete creates holes in heap
- 🔴 **Unpredictable timing**: Allocator can stall frame
- 🔴 **Cache misses**: Scattered memory locations

---

## Solution 1: Object Pool

### Concept

Pre-allocate a fixed pool of objects, reuse them instead of new/delete:

```cpp
class ObjectPool<Bullet> {
    std::vector<Bullet> pool_;        // Pre-allocated storage
    std::vector<Bullet*> available_;  // Free objects
    std::vector<Bullet*> active_;     // In-use objects
    
public:
    ObjectPool(size_t capacity) {
        pool_.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            pool_.emplace_back();
            available_.push_back(&pool_[i]);
        }
    }
    
    Bullet* acquire() {
        if (available_.empty()) return nullptr;
        
        Bullet* obj = available_.back();
        available_.pop_back();
        active_.push_back(obj);
        return obj;
    }
    
    void release(Bullet* obj) {
        active_.erase(std::find(active_.begin(), active_.end(), obj));
        available_.push_back(obj);
        obj->reset();  // Prepare for reuse
    }
};
```

### Performance Results

**Bullet System (1000 bullets):**

| Operation | Traditional | Object Pool | Improvement |
|-----------|-------------|-------------|-------------|
| **Spawn 100 bullets** | 5.2 ms | 0.5 ms | **10x faster** |
| **Memory allocations** | 100 | 0 | **Zero after init** |
| **Frame time variance** | ±2.5 ms | ±0.1 ms | **25x more stable** |

**Particle System (5000 particles):**

| Operation | Traditional | Object Pool | Improvement |
|-----------|-------------|-------------|-------------|
| **Spawn 500 particles** | 18.3 ms | 1.2 ms | **15x faster** |
| **Memory fragmentation** | High | None | **Eliminated** |

---

### Use Cases for Object Pool

✅ **Perfect for:**

```cpp
// 1. Bullets
ObjectPool<Bullet> bulletPool{1000};
auto* bullet = bulletPool.acquire(x, y, vx, vy);

// 2. Particles
ObjectPool<Particle> particlePool{5000};
auto* particle = particlePool.acquire(x, y, lifetime);

// 3. Enemies
ObjectPool<Enemy> enemyPool{200};
auto* enemy = enemyPool.acquire(type, x, y);

// 4. Power-ups
ObjectPool<PowerUp> powerUpPool{50};
auto* powerUp = powerUpPool.acquire(type, x, y);
```

---

## Solution 2: Memory Pool

### Concept

Linear allocator for temporary allocations, reset after use:

```cpp
class MemoryPool {
    uint8_t* buffer_;
    size_t capacity_;
    size_t offset_ = 0;
    
public:
    MemoryPool(size_t capacity) 
        : capacity_(capacity) {
        buffer_ = new uint8_t[capacity];
    }
    
    void* allocate(size_t size, size_t alignment = 8) {
        // Align offset
        size_t padding = (alignment - (offset_ % alignment)) % alignment;
        size_t alignedOffset = offset_ + padding;
        
        if (alignedOffset + size > capacity_) {
            return nullptr;  // Pool exhausted
        }
        
        void* ptr = buffer_ + alignedOffset;
        offset_ = alignedOffset + size;
        return ptr;
    }
    
    void reset() {
        offset_ = 0;  // Instant "free" of all allocations
    }
};
```

### Frame Allocator Pattern

```cpp
class GameLoop {
    MemoryPool framePool{10 * 1024 * 1024};  // 10 MB
    
    void runFrame() {
        framePool.reset();  // Clear previous frame allocations
        
        // All temporary allocations use framePool
        auto* tempData = framePool.allocate<PathfindingData>();
        auto* collisions = framePool.allocate<CollisionPair[]>(100);
        
        updatePhysics(framePool);
        updateAI(framePool);
        render(framePool);
        
        // framePool.reset() called next frame
    }
};
```

### Performance Results

**Frame Allocations (10 MB pool):**

| Metric | Traditional | Memory Pool | Improvement |
|--------|-------------|-------------|-------------|
| **Allocation time** | 2.5 ms | 0.05 ms | **50x faster** |
| **Deallocation time** | 1.8 ms | 0.001 ms | **1800x faster** |
| **Fragmentation** | High | Zero | **Eliminated** |
| **Frame time variance** | ±3.2 ms | ±0.1 ms | **32x more stable** |

---

### Use Cases for Memory Pool

✅ **Perfect for:**

```cpp
// 1. Per-frame temporary data
MemoryPool framePool{10 * 1024 * 1024};

void updatePhysics(MemoryPool& pool) {
    auto* collisions = pool.allocate<Collision[]>(1000);
    // ... process collisions
    // Automatically "freed" at frame end
}

// 2. Pathfinding scratch space
void findPath(MemoryPool& pool) {
    auto* openSet = pool.allocate<Node[]>(500);
    auto* closedSet = pool.allocate<Node[]>(500);
    // ... A* algorithm
}

// 3. Rendering temporary buffers
void renderUI(MemoryPool& pool) {
    auto* vertices = pool.allocate<Vertex[]>(10000);
    // ... build UI geometry
}
```

---

## Complementary Usage

### Why Both?

```
┌─────────────────────────────────────────────────┐
│           Application Memory                    │
├─────────────────────────────────────────────────┤
│                                                 │
│  ┌──────────────┐    ┌──────────────┐          │
│  │ Object Pools │    │ Memory Pools │          │
│  ├──────────────┤    ├──────────────┤          │
│  │ • Bullets    │    │ • Frame      │          │
│  │ • Particles  │    │ • Physics    │          │
│  │ • Enemies    │    │ • Pathfinding│          │
│  │ • Items      │    │ • UI Render  │          │
│  └──────────────┘    └──────────────┘          │
│         │                    │                  │
│         └────────┬───────────┘                  │
│                  │                              │
│          ┌───────▼───────┐                      │
│          │  ECS Registry │                      │
│          └───────────────┘                      │
│                                                 │
└─────────────────────────────────────────────────┘
```

**Object Pool:** Long-lived game entities (bullets, enemies)  
**Memory Pool:** Short-lived temporary data (collision detection, pathfinding)

---

## Implementation Strategy

### Phase 1: Object Pool (Priority: HIGH)

**Timeline:** December 2025 - Week 1

**Targets:**

```cpp
// 1. Bullet System
class BulletSystem {
    ObjectPool<Bullet> pool_{1000};
    
    void fireBullet(float x, float y, float vx, float vy) {
        auto* bullet = pool_.acquire();
        if (bullet) {
            bullet->init(x, y, vx, vy);
            activeBullets_.push_back(bullet);
        }
    }
    
    void update(float dt) {
        for (auto it = activeBullets_.begin(); it != activeBullets_.end();) {
            auto* bullet = *it;
            bullet->update(dt);
            
            if (bullet->shouldDestroy()) {
                pool_.release(bullet);
                it = activeBullets_.erase(it);
            } else {
                ++it;
            }
        }
    }
};
```

**Expected Gains:**

- ✅ Bullet spawning < 0.5 ms per 100 bullets
- ✅ Zero allocations after initialization
- ✅ 60 FPS maintained with 500+ concurrent bullets

---

### Phase 2: Memory Pool (Priority: MEDIUM)

**Timeline:** December 2025 - Week 2

**Targets:**

```cpp
class GameLoop {
    MemoryPool framePool_{10 * 1024 * 1024};  // 10 MB
    MemoryPool physicsPool_{5 * 1024 * 1024}; // 5 MB
    
    void runFrame(float dt) {
        framePool_.reset();
        
        updateInput(framePool_);
        updatePhysics(framePool_, physicsPool_);
        updateAI(framePool_);
        updateRendering(framePool_);
        
        physicsPool_.reset();
    }
};
```

**Expected Gains:**

- ✅ Frame allocation overhead < 5%
- ✅ Consistent frame times (no allocation spikes)
- ✅ Memory usage predictable and bounded

---

## Code Organization

```text
include/rtype/engine/memory/
├── ObjectPool.hpp          # Object pool implementation
├── MemoryPool.hpp          # Linear allocator
├── FrameAllocator.hpp      # Per-frame wrapper
└── PooledAllocator.hpp     # STL allocator adapter

src/engine/memory/
├── ObjectPool.cpp          # (if needed)
└── MemoryPool.cpp          # (if needed)

docs/memory/
├── object_pool.md          # Object pool docs
├── memory_pool.md          # Memory pool docs
└── integration_guide.md    # Integration guide
```

---

## Performance Visualization

### Allocation Timeline Comparison

**Traditional (Problematic):**

```text
Frame 1: new ████ new ██ delete ███ new ████ delete ██
         |
         └─> Fragmented, unpredictable timing

Frame 2: new ██ delete ████ new ███ delete ██ new ████
         |
         └─> More fragmentation, cache misses
```

**Object Pool (Optimized):**

```text
Initialization: allocate ████████████████████████
                |
                └─> One-time cost

Frame 1: acquire ▪ acquire ▪ release ▪ acquire ▪
         |
         └─> Zero allocations, O(1) operations

Frame 2: acquire ▪ release ▪ acquire ▪ release ▪
         |
         └─> Consistent, predictable
```

---

## Business Impact

**With 500 bullets spawned per second:**

| Metric | Traditional | Object Pool | Savings |
|--------|-------------|-------------|---------|
| **Allocations/sec** | 500 | 0 | 500 saved |
| **CPU time/frame** | 5.2 ms | 0.5 ms | 4.7 ms saved |
| **Frame budget @60FPS** | 31% | 3% | 28% reclaimed |

**Reclaimed budget used for:**

- More enemies on screen
- Better particle effects
- Complex AI pathfinding
- Enhanced visual effects

---

## Final Recommendation

✅ **Implement both Object Pool and Memory Pool**.

**Rationale:**

1. **Object Pool**: 8-15x faster entity spawning, zero fragmentation
2. **Memory Pool**: 50x faster frame allocations, predictable memory
3. **Complementary**: Different use cases, no overlap
4. **Maximum Performance**: Combined benefits across all systems
5. **Industry Standard**: Used in all AAA game engines

**Implementation:**

- Object Pool for bullets, particles, enemies, items
- Memory Pool for per-frame temporary allocations
- STL allocator adapters for containers

---

## References

- PoC implementations: `/PoC/PoC_Memory_Optimization/`
- Decision document: `/PoC/PoC_Memory_Optimization/memory_optimization_decision.md`
- Object Pool PoC: `/PoC/PoC_Memory_Optimization/ObjectPool/`
- Memory Pool PoC: `/PoC/PoC_Memory_Optimization/MemoryPool/`
