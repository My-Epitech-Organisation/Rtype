# Memory Optimization PoC

This directory contains Proof of Concepts (PoCs) for memory optimization techniques in the R-Type game engine.

## Overview

Two complementary memory optimization techniques were evaluated:

1. **Object Pool** - Reusable object management
2. **Memory Pool** - Linear allocator for fast allocations

## Quick Start

### Building the PoCs

```bash
cd PoC/PoC_Memory_Optimization
mkdir build && cd build
cmake ..
make
```

### Running the Tests

```bash
# Object Pool test
./poc_object_pool

# Memory Pool test
./poc_memory_pool
```

## Directory Structure

```
PoC_Memory_Optimization/
├── CMakeLists.txt           # Build configuration
├── PoC_Report.md            # Comprehensive PoC report
├── README.md                # This file
├── ObjectPool/
│   ├── ObjectPool.hpp       # Object Pool implementation
│   └── test_object_pool.cpp # Object Pool tests & benchmarks
└── MemoryPool/
    ├── MemoryPool.hpp       # Memory Pool implementation
    └── test_memory_pool.cpp # Memory Pool tests & benchmarks
```

## Object Pool

### Purpose
Reuse frequently created/destroyed objects (bullets, particles, enemies) to avoid allocation overhead.

### Key Features
- Zero-allocation reuse after initialization
- 8-15x performance improvement vs. new/delete
- Automatic growth when exhausted
- Statistics tracking

### Example Usage

```cpp
#include "ObjectPool/ObjectPool.hpp"

struct Bullet {
    float x, y;
    float velocityX, velocityY;
    // ...
};

Memory::ObjectPool<Bullet> bulletPool(1000);

// Acquire bullet from pool
Bullet* bullet = bulletPool.acquire(100.0f, 200.0f, 5.0f, 0.0f);

// Use bullet
bullet->update(deltaTime);

// Release back to pool
bulletPool.release(bullet);
```

### Test Results

```
✅ Zero-allocation reuse: 1000 objects acquired/released in ~0.5ms
✅ Performance: 8.5x faster than new/delete
✅ Game scenario: 60 FPS with 300+ concurrent bullets
```

## Memory Pool

### Purpose
Fast linear allocation for temporary, short-lived data (frame allocations, calculations).

### Key Features
- Extremely fast allocation (pointer bumping)
- 10-20x performance improvement vs. new/delete
- O(1) allocation time
- Reset entire pool at once

### Example Usage

```cpp
#include "MemoryPool/MemoryPool.hpp"

Memory::MemoryPool framePool(10 * 1024 * 1024); // 10MB

// Allocate objects
GameObject* obj = framePool.allocate<GameObject>(x, y, id);

// Allocate arrays
Particle* particles = framePool.allocateArray<Particle>(1000);

// Reset at frame end
framePool.reset();
```

### Test Results

```
✅ Large block: 10MB allocated in ~2.5ms
✅ Performance: 19.0x faster than new/delete
✅ Consistent: ~0.185µs per allocation regardless of scale
```

## Performance Comparison

| Technique | Speedup | Best For | Limitations |
|-----------|---------|----------|-------------|
| Object Pool | 8-15x | Bullets, particles, enemies | Requires manual release |
| Memory Pool | 10-20x | Frame temps, calculations | No individual deallocation |

## Documentation

- **[PoC_Report.md](./PoC_Report.md)** - Detailed evaluation with pros/cons
- **[Memory Optimization Decision](../../docs/memory_optimization_decision.md)** - Final implementation decision

## Integration with R-Type

See the [Memory Optimization Decision](../../docs/memory_optimization_decision.md) document for the full integration plan.

### Recommended Use Cases

**Object Pool:**
- Bullet system (1000 bullets)
- Particle system (5000 particles)
- Enemy system (200 enemies)
- Collectible items

**Memory Pool:**
- Per-frame allocations
- Physics collision detection temp data
- Pathfinding calculations
- UI rendering temporary data

## Exit Criteria

### Object Pool ✅
- [x] Implement ObjectPool<Bullet>
- [x] Acquire/Release 1000 objects
- [x] Demonstrate zero-allocation reuse
- [x] Pool class prototype

### Memory Pool ✅
- [x] Allocate 10MB block
- [x] Assign pointers inside it
- [x] Complexity assessment
- [x] Code snippet deliverable

## Dependencies

- C++20 compiler
- CMake 3.10+
- Standard library only (no external dependencies)

## Related Work

- [ECS Implementation](../ECS/) - Entity Component System
- [Network Protocol](../PoC_Network_Protocol/) - Network optimization

## Authors

R-Type Development Team - Epitech 2025

## License

This is part of the R-Type project (Epitech).

---

**Status:** ✅ Complete  
**Timeline:** 29/11/2025 - 01/12/2025  
**Branch:** 65-spike-main-memory-optimization-poc
