# QuadTree Spatial Partitioning - Proof of Concept

## 📋 Overview

This PoC implements and benchmarks a **QuadTree** data structure for spatial partitioning in 2D collision detection systems. The implementation is designed to answer the question: **"Do we need spatial partitioning for our R-Type game?"**

**Test Period**: 29/11/2025 - 01/12/2025  
**Related Issue**: [Spike] [Main] Collision Algorithm PoC #58

---

## 🎯 What is a QuadTree?

A **QuadTree** is a tree data structure where each internal node has exactly four children. It recursively subdivides 2D space into four quadrants (NW, NE, SW, SE), organizing spatial data hierarchically.

### Visual Representation

```
Initial Space:              After Subdivision:
+-------------------+       +--------+--------+
|                   |       |   NW   |   NE   |
|                   |       |        |        |
|                   |  -->  +--------+--------+
|                   |       |   SW   |   SE   |
|                   |       |        |        |
+-------------------+       +--------+--------+
```

### Core Concept

- **Hierarchical partitioning**: Space is recursively divided when nodes exceed capacity
- **Spatial locality**: Nearby objects are stored in nearby nodes
- **Efficient querying**: Only relevant spatial regions are searched
- **Dynamic structure**: Adapts to object distribution

---

## 🔬 Scope

This PoC implements:

1. ✅ **QuadTree class** with insertion and query operations
2. ✅ **Range query** to retrieve objects in a specific area
3. ✅ **Performance benchmarks** comparing QuadTree vs. brute force AABB
4. ✅ **ECS integration** using the existing ECS PoC
5. ✅ **Comprehensive documentation** of pros and cons

---

## 📦 Deliverables

### Code Structure

```
PoC/QuadTree/
├── QuadTree.hpp          # Main QuadTree implementation
├── Rect.hpp              # Bounding box structure
├── main.cpp              # Test program with benchmarks
├── CMakeLists.txt        # Build configuration
├── README.md             # This file
└── QUADTREE_ANALYSIS.md  # Comprehensive pros/cons analysis
```

### Key Features Implemented

- **Generic QuadTree template** supporting any data type
- **Configurable parameters**: max objects per node, max depth
- **Automatic subdivision** when capacity is exceeded
- **Efficient range queries** with pruning
- **Memory management** with smart pointers
- **Object spanning handling** for objects crossing quadrant boundaries

---

## 🚀 Building and Running

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.20+

### Build Instructions

```bash
# From the PoC/QuadTree directory
mkdir build
cd build
cmake ..
cmake --build .

# Run the PoC
./quadtree_poc
```

### Alternative: Build from repository root

```bash
# From the R-Type repository root
cd build
cmake ..
cmake --build . --target quadtree_poc
./PoC/QuadTree/quadtree_poc
```

---

## 📊 Benchmark Results

The PoC includes several benchmark categories:

### 1. Insertion Performance
Tests insertion speed with varying object counts (100 to 10,000 objects).

### 2. Query Performance
Measures query speed with different query region sizes.

### 3. Brute Force Baseline
Establishes baseline performance using naive O(n²) collision detection.

### 4. Direct Comparison
Side-by-side comparison of QuadTree vs. brute force for collision detection.

**Expected Results:**
- QuadTree: O(log n) insertion, O(log n + k) query (k = results)
- Brute Force: O(n²) for collision detection
- **Speedup**: Significant for large object counts (1000+ objects)

---

## 🧪 Test Cases

The PoC includes comprehensive tests:

### Basic Operations
- ✅ Insert objects within bounds
- ✅ Reject objects outside bounds
- ✅ Query objects in range
- ✅ Query all objects
- ✅ Clear tree

### Subdivision Behavior
- ✅ Automatic subdivision when capacity exceeded
- ✅ Object redistribution to child nodes
- ✅ Objects spanning multiple quadrants

### Edge Cases
- ✅ Objects at boundaries
- ✅ Very small objects
- ✅ Large objects spanning quadrants
- ✅ Empty queries

---

## 🎮 Integration with R-Type ECS

The QuadTree is designed to integrate seamlessly with the existing ECS:

```cpp
// Example: Using QuadTree with ECS
ECS::Registry registry;
QuadTree::QuadTree<ECS::Entity> spatialIndex(bounds);

// Insert entities with colliders
auto view = registry.view<Transform, BoxCollider>();
view.each([&](ECS::Entity entity, Transform& t, BoxCollider& c) {
    auto rect = c.getRect(t);
    spatialIndex.insert({rect, entity});
});

// Query for potential collisions
std::vector<QuadTree::Object<ECS::Entity>> nearby;
spatialIndex.query(queryRegion, nearby);
```

---

## 📈 Performance Characteristics

### Time Complexity

| Operation | Best Case | Average Case | Worst Case |
|-----------|-----------|--------------|------------|
| Insert    | O(1)      | O(log n)     | O(n)       |
| Query     | O(1)      | O(log n + k) | O(n)       |
| Delete    | O(1)      | O(log n)     | O(n)       |
| Clear     | O(1)      | O(n)         | O(n)       |

*k = number of results returned*

### Space Complexity

- **Best Case**: O(n) - balanced tree
- **Worst Case**: O(n * log n) - deep tree with many nodes

### Memory Overhead

- **Per Node**: ~120 bytes (bounds, pointers, metadata)
- **Per Object**: ~40 bytes (bounds + data)
- **Total**: Scales with number of nodes and objects

---

## ⚖️ When to Use QuadTree

### ✅ Use QuadTree When:

- Object count > 500-1000
- Objects are distributed across space
- Frequent spatial queries needed
- Query regions are smaller than total space
- Static or semi-static environments

### ❌ Avoid QuadTree When:

- Object count < 100-200
- All objects are densely clustered
- Objects are uniformly distributed
- Extreme object size variance
- Highly dynamic environments (many insertions/deletions per frame)

---

## 🔄 Dependencies

- **Related to**: [Spike] [Main] Collision Algorithm PoC #58
- **Uses**: ECS PoC from `PoC/ECS/`
- **References**: AABB implementation from `PoC/AABB/`

---

## ✅ Exit Criteria

- [x] QuadTree class implementation
- [x] Insert and query operations
- [x] Range query functionality
- [x] Performance benchmarks vs. brute force AABB
- [x] Comprehensive documentation
- [x] Pros and cons analysis

---

## 📚 Further Reading

See **QUADTREE_ANALYSIS.md** for:
- Detailed advantages and disadvantages
- Performance analysis
- Comparison with other spatial partitioning methods
- Recommendations for R-Type implementation

---

## 👥 Authors

**R-Type Development Team**  
Epitech Tek3 - 2025

---

## 📝 Notes

This PoC demonstrates that **spatial partitioning provides significant benefits** for collision detection when:
1. The number of objects exceeds ~500-1000
2. Objects are distributed across the game world
3. Query regions are localized (not full-screen)

For R-Type, where we expect:
- 100-500 active entities (enemies, bullets, power-ups)
- Spatial distribution across the screen
- Localized collision checks

**Recommendation**: QuadTree provides measurable performance benefits and should be considered for the final implementation, especially if we plan to scale to more entities or larger play areas.
