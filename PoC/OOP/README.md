# OOP Architecture Proof of Concept

> Traditional Object-Oriented Programming approach for R-Type game engine

## 📋 Overview

This Proof of Concept (PoC) demonstrates the **traditional OOP inheritance approach** for game development, specifically for the R-Type project. It explores the use of class hierarchies, inheritance, and polymorphism to structure game entities.

### 🎯 Purpose

- Test standard OOP inheritance patterns
- Identify complexity and maintainability issues
- Demonstrate the "Diamond Inheritance" problem
- Compare with ECS (Entity Component System) approach
- Answer: **Is standard OOP inheritance easier to understand?**

### 📅 Timeline

- **Start**: 26/11/2025
- **End**: 27/11/2025
- **Related Issue**: #51 - [Spike] Engine Architecture PoC (ECS vs OOP)

---

## 🏗️ Architecture

### Class Hierarchy

```
GameObject
    ├── Movable
    │   ├── Player
    │   ├── Enemy
    │   │   └── Boss
    ├── Shootable
    ├── Damageable
    └── ShootingPowerUp
```

### Components

1. **GameObject.hpp/cpp** - Base class with position, health, velocity
2. **Movable.hpp/cpp** - Adds movement capabilities
3. **Player.hpp/cpp** - Player-specific behavior (shooting, score, lives)
4. **Enemy.hpp/cpp** - Enemy AI and behavior
5. **DiamondProblem.hpp/cpp** - Demonstrates multiple inheritance issues
6. **main.cpp** - Demonstration program with analysis

---

## 🚀 Building and Running

### Prerequisites

- C++17 compatible compiler (g++, clang++)
- Make (optional)
- Linux/macOS/WSL

### Build Methods

#### Option 1: Using build script (Recommended)

```bash
cd PoC/oop_test
chmod +x build.sh
./build.sh          # Debug build
./build.sh release  # Release build
```

#### Option 2: Using Make

```bash
cd PoC/oop_test
make                # Debug build (default)
make release        # Release build
make run            # Build and run
make clean          # Clean build files
```

#### Option 3: Manual compilation

```bash
cd PoC/oop_test
g++ -std=c++17 -Wall -Wextra -g -o build/oop_poc \
    GameObject.cpp Movable.cpp Player.cpp Enemy.cpp \
    DiamondProblem.cpp main.cpp
```

### Running

```bash
./build/oop_poc
```

---

## 📊 What It Demonstrates

### 1. Basic Inheritance Hierarchy
- GameObject → Movable → Player
- GameObject → Movable → Enemy → Boss
- Up to **4 levels deep** (fragile!)

### 2. Code Duplication Problem
- `Player::shoot()` and `Enemy::shoot()` are duplicated
- Can't easily share behavior without complex inheritance

### 3. Diamond Inheritance Problem
```
    GameObject
      /  \
 Shootable  Damageable
      \  /
   PowerUp  ← Can't inherit from both!
```

### 4. Fragile Base Class
- Changes to `GameObject` affect all 10+ subclasses
- Deep inheritance makes debugging difficult

### 5. Runtime Inflexibility
- Can't add/remove behaviors dynamically
- Can't compose: `Player + Shield + DoubleShot + SpeedBoost`
- Must decide ALL capabilities at compile-time

### 6. Memory Layout Issues
- Objects scattered in memory (cache misses)
- Virtual function overhead
- Can't efficiently iterate "all movable objects"

---

## 📈 Complexity Analysis Results

### Metrics

| Metric | Value | Status |
|--------|-------|--------|
| Lines of Code | ~800 | 🟡 Medium |
| Max Inheritance Depth | 4 levels | 🔴 High |
| Code Duplication | shoot() in 2 places | 🔴 High |
| Virtual Functions | 8+ | 🟡 Medium |
| Coupling | Very High | 🔴 Critical |

### Problems Identified

#### ❌ Code Duplication
- `shoot()` method duplicated in Player and Enemy
- Fix bug = change 2+ places

#### ❌ Fragile Base Class
- Changing GameObject affects ALL subclasses
- Boss depends on 3 parent classes
- Changes ripple through entire hierarchy

#### ❌ Diamond Inheritance
- Can't combine Shootable + Damageable
- Virtual inheritance is complex
- Forced to duplicate fields

#### ❌ Inflexibility
- Can't add behaviors at runtime
- Can't make GameObject suddenly Movable
- Must decide capabilities at compile-time

#### ❌ Tight Coupling
- Player → Movable → GameObject
- Hard to test in isolation
- Inheritance = strongest coupling

#### ❌ Memory Layout
- Objects scattered in memory
- Bad cache locality
- Virtual function overhead

---

## 🎓 Key Learnings

### ✅ What Works Well

1. **Initial Intuition**: "Player is-a Movable" is easy to understand
2. **Polymorphism**: Works naturally with `std::vector<GameObject*>`
3. **Familiarity**: Most programmers know OOP

### ❌ What Doesn't Work Well

1. **Scales Poorly**: Complexity grows exponentially
2. **Diamond Problem**: Confusing and hard to solve
3. **Deep Hierarchies**: Hard to trace behavior (Boss → Enemy → Movable → GameObject)
4. **No Runtime Composition**: Can't add behaviors dynamically
5. **Performance**: Virtual calls, cache misses, scattered memory
6. **Testing**: Must mock entire hierarchy

### 🎯 Answer to Key Question

> **Is standard OOP inheritance easier to understand?**

**Yes for simple hierarchies (1-2 levels), but NO for complex games.**

- ✓ Easy to start
- ✗ Becomes maintenance nightmare
- ✗ Diamond problem is unintuitive
- ✗ Deep hierarchies are hard to follow
- ✗ Runtime flexibility is impossible

---

## 📁 File Structure

```
PoC/oop_test/
├── GameObject.hpp          # Base class
├── GameObject.cpp
├── Movable.hpp            # Movement behavior
├── Movable.cpp
├── Player.hpp             # Player entity
├── Player.cpp
├── Enemy.hpp              # Enemy entities + Boss
├── Enemy.cpp
├── DiamondProblem.hpp     # Demonstrates diamond inheritance
├── DiamondProblem.cpp
├── main.cpp               # Demo program + analysis
├── build.sh               # Build script
├── Makefile               # Make build system
├── OOP_ANALYSIS.md        # Detailed analysis with diagrams
└── README.md              # This file
```

---

## 🔍 Example Output

```
╔══════════════════════════════════════════════════════════════╗
║                                                              ║
║        R-TYPE: OOP ARCHITECTURE PROOF OF CONCEPT            ║
║                                                              ║
║  Testing traditional Object-Oriented Programming approach   ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝

============================================================
  1. BASIC INHERITANCE HIERARCHY
============================================================

Creating a GameObject...
[GameObject] Created at (100, 100)
[GameObject] Update called for GameObject
[GameObject] Rendering GameObject at (100, 100)

✓ Basic inheritance works fine for simple hierarchies
✗ But GameObject has movement data even if it never moves!

[... more demonstrations ...]

============================================================
  COMPLEXITY ANALYSIS - OOP APPROACH
============================================================

📊 METRICS:
   - Lines of Code: ~500+ for basic hierarchy
   - Inheritance Depth: Up to 4 levels
   - Code Duplication: shoot() duplicated in Player and Enemy

❌ PROBLEMS IDENTIFIED:
   1. CODE DUPLICATION
   2. FRAGILE BASE CLASS
   3. DIAMOND INHERITANCE
   4. INFLEXIBILITY
   [... detailed analysis ...]
```

---

## 📚 Documentation

For detailed class diagrams and analysis, see:
- **[OOP_ANALYSIS.md](./OOP_ANALYSIS.md)** - Complete analysis with Mermaid diagrams

---

## 🔄 Next Steps

1. ✅ OOP PoC completed
2. 🔜 Implement ECS PoC for comparison
3. 🔜 Performance benchmarking (OOP vs ECS)
4. 🔜 Evaluate hybrid approaches
5. 🔜 Make architectural decision for R-Type

---

## 💡 Recommendations

Based on this PoC:

### For R-Type Project:

**❌ NOT RECOMMENDED: Pure OOP inheritance**
- Too rigid for game with many entity types
- Diamond problem will occur with power-ups
- Performance issues with 1000+ entities

**✅ RECOMMENDED: Consider ECS instead**
- Composition over inheritance
- Runtime behavior changes
- Better cache locality
- Easier to add new enemies/behaviors
- Proven in game engines (Unity DOTS, Bevy, EnTT)

### When OOP Works:

Use OOP inheritance for:
- Simple hierarchies (1-2 levels)
- Fixed entity types (< 5 types)
- No runtime composition needed
- Small-scale projects

### When ECS Works Better:

Use ECS for:
- Many entity types (10+ types)
- Dynamic behavior composition
- Large entity counts (100+ active)
- Performance-critical systems
- **Games like R-Type** ✅

---

## 🤝 Contributing

This is a spike/PoC for learning purposes. See the main R-Type repository for contribution guidelines.

---

## 📝 Notes

- This PoC intentionally shows the **problems** with OOP
- Code duplication and complexity are **by design** to demonstrate issues
- Not production code - educational purposes only
- See ECS PoC for comparison

---

## 📖 References

- **Related Issue**: #51 - Engine Architecture PoC (ECS vs OOP)
- **Documentation**: See `docs/architecture/` in main repository
- **ECS Resources**: 
  - [Entity Component System FAQ](https://github.com/SanderMertens/ecs-faq)
  - [Data-Oriented Design](https://www.dataorienteddesign.com/dodbook/)

---

*This PoC was created as part of the R-Type Engine Architecture spike (26/11-27/11/2025)*
