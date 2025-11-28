---
sidebar_position: 1
---

# Technology Choices Overview

This section documents all comparative studies and technical justifications for technology choices made in the R-Type project. Each decision is backed by practical proof-of-concept implementations and quantitative benchmarks.

---

## Quick Reference

| Category | Decision | Key Benefit | Performance Gain |
|----------|----------|-------------|------------------|
| **Language** | C++ | Deterministic execution | 46x faster than Python |
| **Compiler** | Clang + Ninja | Better diagnostics | 18x faster builds |
| **Package Manager** | Vcpkg | Binary caching | 14x faster builds |
| **Code Quality** | Clang-Tidy + Format | Professional grade | Free tools |
| **Architecture** | ECS | Flexible composition | 20x faster updates |
| **Collision** | QuadTree + AABB | Spatial partitioning | 125x fewer checks |
| **Event System** | Hybrid (EventBus + Queue) | Decoupled systems | Thread-safe |
| **Movement** | Multi-system | Designer empowerment | Flexible patterns |
| **Graphics Library** | SFML | Native C++ OOP | Clean API |
| **Network Library** | ASIO Standalone | Zero dependencies | 347 KB binaries |
| **Network Protocol** | UDP | No head-of-line blocking | 15% lower latency |
| **Serialization** | Custom Binary | Minimal bandwidth | 317% smaller than JSON |
| **Memory** | Object Pool + Memory Pool | Zero fragmentation | 8-15x faster spawning |
| **Storage** | JSON + Binary + SQLite | Right tool for each job | 500x faster saves |
| **Accessibility** | Slow Mode + Custom Controls | Inclusive design | 15-20% more players |

---

## Development Tools

### Programming Languages

**Primary: C++**

- Performance: 2.1M entity updates/sec vs 45K for Python
- Latency: 45μs network latency vs 180μs for Python
- Deterministic: No GC pauses for stable 60 FPS
- Industry standard for AAA games

**Complementary:**

- **Go**: Server infrastructure (goroutines for concurrency)
- **Python**: Development tools and automation

[Read detailed comparison →](./Development/programming-languages.md)

---

### Package Management

**Choice: Vcpkg**

- Build time: 1.74s vs 24.4s for CMake CPM (14x faster)
- Binary caching: 98% time savings on incremental builds
- Zero system dependencies (no X11/graphics lib hell)
- Cross-platform consistency

**Annual impact:** 158 developer-hours saved (4 weeks)

[Read detailed comparison →](./Development/package-managers.md)

---

### Compiler and Build System

**Choice: Clang + Ninja**

- Clang: Better error messages, tooling ecosystem
- Ninja: 18x faster incremental builds than Make
- Professional diagnostics save debugging hours
- Full C++20 support with sanitizers

[Read detailed comparison →](./Development/compiler-selection.md)

---

### Code Quality Tools

**Choice: Clang-Tidy + Clang-Format + CppLint**

- Static analysis: 100+ checks for modern C++
- Auto-formatting: Eliminates style debates
- 95% feature-equivalent to commercial tools ($0 cost)

[Read detailed comparison →](./Development/code-quality.md)

---

### Graphics Library

**Choice: SFML**

- Native C++ (no C wrapper overhead)
- Object-oriented API (clean, intuitive)
- 2D-optimized (perfect for side-scroller)
- Comprehensive (graphics + audio + input)

[Read detailed comparison →](./Development/graphics-library.md)

---

## Architecture

### ECS vs OOP

**Choice: Entity-Component-System**

**Why NOT OOP:**

- ❌ Diamond inheritance problem
- ❌ Deep hierarchies (4+ levels)
- ❌ Code duplication (shoot() in multiple classes)
- ❌ Fragile base class

**Why ECS:**

- ✅ Composition over inheritance
- ✅ Zero code duplication
- ✅ Runtime flexibility (add/remove components)
- ✅ 20x faster performance (cache-friendly)

[Read detailed comparison →](./Architecture/ecs-vs-oop.md)

---

### Collision Detection

**Choice: QuadTree + AABB Hybrid**

- Broad phase: QuadTree reduces O(n²) to O(n log n)
- Narrow phase: AABB for O(1) precise checks
- 125x fewer collision checks for 1000 entities

[Read detailed comparison →](./Architecture/collision-detection.md)

---

### Event System

**Choice: Hybrid (EventBus + Command Queue)**

- EventBus: Decoupled system-to-system communication
- Command Queue: Thread-safe network-to-game messaging
- CircularBuffer: Zero allocations, bounded memory

[Read detailed comparison →](./Architecture/event-system.md)

---

### Movement Systems

**Choice: Multi-system Architecture**

- Linear: Projectiles (Critical)
- Sine Wave: Classic R-Type patterns (High)
- Scripted: Boss behaviors (High)
- Bezier: Cinematics (Optional)

[Read detailed comparison →](./Architecture/movement-systems.md)

---

## Networking

### Protocol: TCP vs UDP

**Choice: UDP**

- 15% lower latency (120μs vs 140μs)
- No head-of-line blocking (critical for real-time)
- TCP packet loss = 200ms+ delay for all subsequent packets
- UDP packet loss = only that packet affected

**Implementation:**

- UDP for gameplay packets (60 Hz)
- Custom reliability layer for critical events

[Read detailed comparison →](./Networking/tcp-vs-udp.md)

---

### Network Library

**Choice: ASIO Standalone**

- Zero dependencies (vs full Boost framework)
- 50% faster configuration (17.7s vs 26.7s)
- 13% smaller binaries (347 KB vs 391 KB)
- Header-only (no linking)

**Rejected:**

- Boost.Asio (requires entire Boost)
- Qt Network (incompatible event loop)
- ACE (deprecated, config failed)

[Read detailed comparison →](./Networking/library-selection.md)

---

### Serialization

**Choice: Custom Binary Packets**

- 317% smaller than JSON (205 bytes vs 856 bytes for 10 entities)
- 39% smaller than Protobuf (205 bytes vs 285 bytes)
- Bandwidth: 98.4 Kbps @ 60Hz (vs 410.88 Kbps for JSON)
- 80x faster serialization than JSON

**Use cases:**

- Binary: Gameplay packets (position, velocity)
- Protobuf: Tools, cross-language needs
- JSON: Configuration, debugging

[Read detailed comparison →](./Networking/serialization.md)

---

## Performance

### Memory Optimization

**Choice: Object Pool + Memory Pool** (both)

**Object Pool:**

- 8-15x faster entity spawning
- Zero allocations after initialization
- Zero memory fragmentation
- Perfect for: bullets, particles, enemies

**Memory Pool:**

- 50x faster frame allocations
- 1800x faster deallocation (reset)
- Predictable memory usage
- Perfect for: temporary data, pathfinding, collision

**Business impact:** 28% frame budget reclaimed for features

[Read detailed comparison →](./Performance/memory-optimization.md)

---

## Storage

### Data Persistence

**Choice: Hybrid Strategy**

**JSON for Configuration:**

- Human-readable, easy to edit
- Git-friendly version control
- Examples: settings, levels, asset metadata

**Binary for Game Saves:**

- 500x faster read (87μs vs 510ms)
- 5.6x smaller files
- Examples: player progress, world state

**SQLite for Structured Data:**

- Complex queries (leaderboards, rankings)
- ACID transactions
- Examples: highscores, player profiles

[Read detailed comparison →](./Storage/data-persistence.md)

---

## Accessibility

### Inclusive Design Features

**Approved Features:**

| Feature | Impact | Priority |
|---------|--------|----------|
| **Slow Mode** | 15-20% of players | Critical |
| **Custom Controls** | 10-15% of players | Critical |
| **Colorblind Support** | 5-8% of players | High |

- Slow Mode: 0.00% physics error, 0.1% CPU overhead
- Custom Controls: Industry standard requirement
- Colorblind: Shape-based distinctions + palette swapping

[Read detailed comparison →](./Accessibility/accessibility-features.md)

---

## Research Methodology

All decisions based on:

1. **Practical PoC implementations** (`/PoC` directory)
2. **Quantitative benchmarks** (measured on real hardware)
3. **Real-world requirements** (60 FPS, less than 100ms latency)
4. **Industry best practices** (AAA game development)

**Test environment:**

- Hardware: ASUS Vivobook M1502QA (AMD Ryzen 5 5600H @ 4.280GHz, 8GB RAM)
- OS: Kali GNU/Linux Rolling x86_64 (Kernel 6.16.8)
- Compiler: GCC 15.1.0 with C++20
- Build: Release/optimized configurations

---

## Key Principles

**1. Performance First**

- Real-time gaming requires deterministic execution
- Measured: All claims backed by benchmarks
- Target: 60 FPS (16.67ms frame budget)

**2. Developer Experience**

- Fast iteration cycles (Vcpkg caching)
- Clean APIs (SFML OOP, ECS composition)
- Minimal dependencies (ASIO standalone)

**3. Right Tool for Job**

- Binary for speed, JSON for readability
- Object Pool for entities, Memory Pool for temporaries
- UDP for gameplay, TCP/Reliable for critical events

**4. Industry Proven**

- ECS: Unity, Unreal, Overwatch
- UDP: Valorant, CS:GO, Quake
- C++: All AAA game engines

---

## Navigation

Explore detailed comparisons:

- [Development Tools](./Development/programming-languages.md)
- [Compiler Selection](./Development/compiler-selection.md)
- [Code Quality Tools](./Development/code-quality.md)
- [Architecture Decisions](./Architecture/ecs-vs-oop.md)
- [Collision Detection](./Architecture/collision-detection.md)
- [Event System](./Architecture/event-system.md)
- [Movement Systems](./Architecture/movement-systems.md)
- [Networking Choices](./Networking/tcp-vs-udp.md)
- [Performance Optimization](./Performance/memory-optimization.md)
- [Storage Strategy](./Storage/data-persistence.md)
- [Accessibility Features](./Accessibility/accessibility-features.md)

All PoC implementations available in `/PoC` directory.
