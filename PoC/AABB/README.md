# AABB Collision Detection - Proof of Concept

## 📖 Overview

This directory contains a complete Proof of Concept for **AABB (Axis-Aligned Bounding Box)** collision detection, integrated with the R-Type ECS framework.

## 📂 Structure

```
PoC/AABB/
├── Rect.hpp                      # AABB rectangle structure
├── Collision.hpp                 # Collision detection algorithms
├── main.cpp                      # Test program and benchmarks
├── CMakeLists.txt                # Build configuration
├── AABB_COMPARATIVE_STUDY.md     # Comprehensive analysis document
└── README.md                     # This file
```

## 🚀 Quick Start

### Building

```bash
# From PoC/AABB directory
mkdir build
cd build
cmake ..
cmake --build .

# Run the PoC
./aabb_collision_poc
```

### Alternative: Build from root

```bash
# From R-Type root directory
cd build
cmake ..
make aabb_collision_poc

# Run
./PoC/AABB/aabb_collision_poc
```

## 🧪 What's Included

### Core Implementation

1. **Rect.hpp** - Rectangle structure with:
   - Position (x, y) and dimensions (w, h)
   - Helper methods (left, right, top, bottom, center)
   - Constexpr support for compile-time evaluation

2. **Collision.hpp** - Collision algorithms:
   - `checkCollision()` - Basic overlap detection
   - `checkCollisionStrict()` - Excludes edge touching
   - `containsPoint()` - Point-in-rectangle test
   - `contains()` - Rectangle containment
   - `intersection()` - Computes overlap area
   - `unionBounds()` - Computes bounding box
   - `getOverlapDepth()` - Calculates penetration depth

### Test Suite

The `main.cpp` includes:
- ✅ 8 basic collision test cases
- ✅ Advanced function tests
- ✅ ECS integration demonstration
- ✅ Performance benchmarks (1M+ checks)

### Documentation

**AABB_COMPARATIVE_STUDY.md** provides:
- Algorithm explanation
- Pros and cons analysis
- Performance benchmarks
- Use case recommendations
- R-Type-specific guidance

## 🎯 Test Results

All tests should pass with output similar to:

```
╔════════════════════════════════════════════════════════════╗
║         AABB Collision Detection - Proof of Concept       ║
║                      R-Type Project                        ║
╚════════════════════════════════════════════════════════════╝

============================================================
  Basic AABB Collision Tests
============================================================
Overlapping rectangles                            : ✓ PASS
Separated on X axis                               : ✓ PASS
Separated on Y axis                               : ✓ PASS
...

Performance Benchmark:
  Total time: ~1000 µs
  Time per check: ~0.001 µs
  Checks per second: 1,000,000,000
```

## 💡 Usage Examples

### Basic Collision Check

```cpp
#include "PoC/AABB/Rect.hpp"
#include "PoC/AABB/Collision.hpp"

AABB::Rect player{100.0f, 100.0f, 32.0f, 32.0f};
AABB::Rect enemy{120.0f, 120.0f, 32.0f, 32.0f};

if (AABB::checkCollision(player, enemy)) {
    // Handle collision
}
```

### ECS Integration

```cpp
// Components
struct Transform { float x, y; };
struct BoxCollider { 
    float width, height; 
    AABB::Rect getRect(const Transform& t) const {
        return AABB::Rect{t.x, t.y, width, height};
    }
};

// System
void collisionSystem(ECS::Registry& registry) {
    auto view = registry.view<Transform, BoxCollider>();
    
    view.each([&](Entity a, auto& tA, auto& cA) {
        AABB::Rect rectA = cA.getRect(tA);
        
        view.each([&](Entity b, auto& tB, auto& cB) {
            if (a.id >= b.id) return;
            
            AABB::Rect rectB = cB.getRect(tB);
            if (AABB::checkCollision(rectA, rectB)) {
                onCollision(a, b);
            }
        });
    });
}
```

### Collision Response

```cpp
AABB::Rect a{0.0f, 0.0f, 10.0f, 10.0f};
AABB::Rect b{5.0f, 5.0f, 10.0f, 10.0f};

float overlapX, overlapY;
if (AABB::getOverlapDepth(a, b, overlapX, overlapY)) {
    // Resolve collision by moving along minimum overlap axis
    if (overlapX < overlapY) {
        // Move horizontally
        a.x -= overlapX;
    } else {
        // Move vertically
        a.y -= overlapY;
    }
}
```

## 📊 Performance Characteristics

| Metric | Value |
|--------|-------|
| **Time Complexity** | O(1) constant |
| **Space Complexity** | O(1) no allocations |
| **Memory per Rect** | 16 bytes (4 floats) |
| **Checks per Second** | 1,000,000,000+ |
| **Cache Friendly** | ✅ Yes |

## ⚠️ Limitations

1. **No Rotation Support** - Only works for axis-aligned rectangles
2. **Approximate for Non-Rectangles** - Conservative bounding box for circles/polygons
3. **Broad Phase Only** - Best used as first-pass culling
4. **Requires Spatial Partitioning** - O(n²) naive implementation

See **AABB_COMPARATIVE_STUDY.md** for detailed analysis.

## 🎮 Recommended Use Cases

### ✅ Excellent For:
- Broad-phase collision culling
- Grid-based games
- UI hit detection
- Fast-moving small objects (bullets)

### ⚠️ Use with Caution:
- Objects with rotation
- Organic/irregular shapes
- Precision-critical gameplay

### ❌ Not Recommended:
- Fighting games
- Physics simulations
- Bullet hell shooters (as sole method)

## 🔗 Related

- **Issue**: [Spike] [Main] Collision Algorithm PoC #58
- **Dependencies**: ECS framework (`PoC/ECS/`)
- **Next Steps**: QuadTree spatial partitioning (PoC/QuadTree/)

## 📝 Exit Criteria

- [x] Implement `Rect` structure
- [x] Implement `checkCollision()` function
- [x] Successful overlap test
- [x] Collision logic code delivered
- [x] Comprehensive documentation
- [x] Performance validation
- [x] ECS integration

**Status**: ✅ Complete

## 👥 Authors

R-Type Team - Epitech Tek3

## 📅 Timeline

**Start**: 28/11/2025  
**End**: 29/11/2025  
**Actual Completion**: 24/11/2025 ✅
