# Collision Detection Strategy

Comparative analysis of collision detection algorithms and spatial partitioning.

## Executive Summary

The collision detection system uses a **two-phase hybrid approach**:

1. **Broad Phase**: QuadTree spatial partitioning (reduces O(n²) to O(n log n))
2. **Narrow Phase**: AABB collision detection (O(1) per pair)

---

## Goals

| Objective | Target |
|-----------|--------|
| Detect collisions accurately | Player, enemies, bullets, power-ups |
| Maintain frame rate | 60 FPS (16.67ms per frame) |
| Scale to many entities | 200-500+ simultaneous |
| Minimize false positives | While ensuring no false negatives |
| ECS integration | Seamless with Entity-Component-System |

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                    COLLISION DETECTION                  │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  Phase 1: BROAD PHASE (Spatial Partitioning)            │
│  ┌─────────────────────────────────┐                    │
│  │         QuadTree                │                    │
│  │  Quickly eliminate distant pairs│                    │
│  │  Reduces O(n²) to O(n log n)    │                    │
│  └─────────────────────────────────┘                    │
│                    ↓                                    │
│  Phase 2: NARROW PHASE (Precise Detection)              │
│  ┌─────────────────────────────────┐                    │
│  │         AABB Collision          │                    │
│  │  Fast, accurate overlap check   │                    │
│  │  O(1) per pair check            │                    │
│  └─────────────────────────────────┘                    │
│                    ↓                                    │
│              Collision Response                         │
└─────────────────────────────────────────────────────────┘
```

---

## Algorithm 1: AABB Collision

### What is AABB?

**AABB** (Axis-Aligned Bounding Box) treats all objects as rectangles aligned with coordinate axes. Detects overlaps by comparing boundaries on X and Y axes.

### Mathematical Foundation

Based on the **Separating Axis Theorem**:

```
Two rectangles collide if they overlap on BOTH axes:
  - X-axis: (A.right > B.left) AND (A.left < B.right)
  - Y-axis: (A.bottom > B.top) AND (A.top < B.bottom)
```

### Visual Representation

```
Collision Detected:              No Collision:
┌─────┐                          ┌─────┐     ┌─────┐
│  A  │                          │  A  │     │  B  │
│  ┌──┼─┐                        └─────┘     └─────┘
│  │B │ │                        Separated on X axis
└──┼──┘ │
   └────┘
```

### Implementation

```cpp
struct Rect {
    float x, y;       // Top-left corner
    float w, h;       // Width and height
    
    float left()   const { return x; }
    float right()  const { return x + w; }
    float top()    const { return y; }
    float bottom() const { return y + h; }
};

bool checkCollision(const Rect& a, const Rect& b) {
    // Check if separated on X axis
    if (a.right() < b.left() || b.right() < a.left()) {
        return false;
    }
    
    // Check if separated on Y axis
    if (a.bottom() < b.top() || b.bottom() < a.top()) {
        return false;
    }
    
    // Not separated = collision
    return true;
}
```

### Performance Characteristics

| Metric | Value |
|--------|-------|
| **Time per Check** | Approximately 0.001 µs (1 nanosecond) |
| **Throughput** | 1,000,000,000+ checks/second |
| **CPU Instructions** | 8-12 instructions |
| **Memory per Object** | 16 bytes (4 floats) |
| **Cache Friendliness** | Excellent |

### AABB Pros and Cons

| Advantages | Limitations |
|------------|-------------|
| Extreme speed (O(1)) | Axis-aligned only |
| Simple (4 conditions) | No rotation support |
| Zero false negatives | Conservative approximation |
| Deterministic | May have false positives |
| Cache-friendly | |

---

## Algorithm 2: QuadTree Spatial Partitioning

### What is a QuadTree?

A tree structure that recursively subdivides 2D space into four quadrants (NW, NE, SW, SE). Organizes objects by spatial location for efficient queries.

### Core Principle

```
Level 0 (Root):                Level 1:
+-------------------+          +--------+--------+
|                   |          |   NW   |   NE   |
|                   |   -->    |        |        |
|                   |          +--------+--------+
|                   |          |   SW   |   SE   |
+-------------------+          +--------+--------+
```

### Subdivision Formula

For node with bounds `B = {x, y, width, height}`:

| Quadrant | Bounds |
|----------|--------|
| NW | `{x, y, width/2, height/2}` |
| NE | `{x + width/2, y, width/2, height/2}` |
| SW | `{x, y + height/2, width/2, height/2}` |
| SE | `{x + width/2, y + height/2, width/2, height/2}` |

### Implementation

```cpp
template<typename T>
class QuadTree {
private:
    Rect _bounds;
    std::vector<Object<T>> _objects;
    std::unique_ptr<QuadTree> _northwest;
    std::unique_ptr<QuadTree> _northeast;
    std::unique_ptr<QuadTree> _southwest;
    std::unique_ptr<QuadTree> _southeast;
    size_t _maxObjects = 10;
    size_t _maxDepth = 5;
    size_t _depth;
    bool _divided = false;

public:
    bool insert(const Object<T>& obj) {
        if (!_bounds.contains(obj)) return false;
        
        if (_divided) {
            // Try to insert in children
            for (auto& child : {_northwest, _northeast, _southwest, _southeast}) {
                if (child->canFullyContain(obj)) {
                    return child->insert(obj);
                }
            }
            // Spans multiple children, store here
            _objects.push_back(obj);
        } else {
            _objects.push_back(obj);
            if (_objects.size() > _maxObjects && _depth < _maxDepth) {
                subdivide();
            }
        }
        return true;
    }
    
    void query(const Rect& range, std::vector<Object<T>>& found) const {
        if (!_bounds.intersects(range)) return;
        
        for (const auto& obj : _objects) {
            if (range.intersects(obj.bounds)) {
                found.push_back(obj);
            }
        }
        
        if (_divided) {
            _northwest->query(range, found);
            _northeast->query(range, found);
            _southwest->query(range, found);
            _southeast->query(range, found);
        }
    }
};
```

### QuadTree Performance

| Entity Count | Brute Force | QuadTree | Speedup |
|--------------|-------------|----------|---------|
| 100 | 4,950 checks | ~400 checks | 12x |
| 500 | 124,750 checks | ~2,000 checks | 62x |
| 1000 | 499,500 checks | ~4,000 checks | 125x |

### QuadTree Pros and Cons

| Advantages | Limitations |
|------------|-------------|
| O(n log n) complexity | Rebuild overhead |
| Scales to many entities | Memory overhead |
| Spatial locality | Complex implementation |
| Dynamic subdivision | Object movement handling |

---

## Hybrid Strategy Comparison

### Approach Comparison

| Approach | Complexity | Best For |
|----------|------------|----------|
| **Brute Force** | O(n²) | Less than 50 entities |
| **Grid** | O(n) | Uniform distribution |
| **QuadTree** | O(n log n) | Variable density |
| **AABB Only** | O(1) per pair | Narrow phase |

### Why Hybrid?

| Phase | Algorithm | Purpose |
|-------|-----------|---------|
| **Broad** | QuadTree | Eliminate distant pairs quickly |
| **Narrow** | AABB | Precise collision detection |

### Performance Budget

For 500 entities at 60 FPS:

| Metric | Target | Our System |
|--------|--------|------------|
| Frame budget | 16.67ms | - |
| Collision budget | 2ms max | - |
| Brute force checks | 124,750 | - |
| QuadTree checks | ~2,000 | ~125x reduction |
| AABB time | 2µs total | Well under budget |

---

## ECS Integration

### Components

```cpp
namespace RType::Collision {
    struct BoundingBox {
        float width;
        float height;
        float offsetX = 0;
        float offsetY = 0;
    };
    
    struct CollisionLayer {
        uint32_t layer;      // What layer this entity is on
        uint32_t mask;       // What layers it collides with
    };
    
    struct CollisionResult {
        Entity other;
        Vec2 overlap;
        Vec2 normal;
    };
}
```

### System

```cpp
class CollisionSystem {
public:
    void update(Registry& registry) {
        // 1. Build QuadTree
        _quadTree.clear();
        for (auto [entity, pos, box] : registry.view<Position, BoundingBox>()) {
            _quadTree.insert({entity, computeAABB(pos, box)});
        }
        
        // 2. Query and check collisions
        for (auto [entity, pos, box, layer] : 
             registry.view<Position, BoundingBox, CollisionLayer>()) {
            
            Rect queryRect = computeAABB(pos, box).expand(QUERY_MARGIN);
            auto candidates = _quadTree.query(queryRect);
            
            for (const auto& candidate : candidates) {
                if (candidate.entity == entity) continue;
                if (!layersMatch(layer, candidate.layer)) continue;
                
                if (checkAABB(computeAABB(pos, box), candidate.bounds)) {
                    handleCollision(entity, candidate.entity);
                }
            }
        }
    }
};
```

---

## Final Decision

### Selected Approach: QuadTree + AABB Hybrid

| Component | Choice | Rationale |
|-----------|--------|-----------|
| **Broad Phase** | QuadTree | Handles variable density |
| **Narrow Phase** | AABB | Fastest precise check |
| **Rebuild** | Per frame | Handles moving objects |
| **Max Depth** | 5 levels | Balance memory/performance |
| **Node Capacity** | 10 objects | Optimal for our scale |

### Why This Combination?

1. **QuadTree** eliminates 95%+ of unnecessary checks
2. **AABB** is the fastest precise collision algorithm
3. **Hybrid** gives us O(n log n) overall complexity
4. **ECS-friendly** with component-based design

---

## References

- PoC implementations: `/PoC/PoC_Collision/`
- AABB implementation: `/PoC/PoC_Collision/AABB/`
- QuadTree implementation: `/PoC/PoC_Collision/QuadTree/`
- Decision document: `/PoC/PoC_Collision/collision_detection_strategy.md`
