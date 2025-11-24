# Movement Systems - Visual Overview

## 📁 Project Structure

```
R-Type/
├── PoC/
│   ├── ECS/                          # ECS Framework (dependency)
│   │   ├── ECS.hpp
│   │   ├── Core/
│   │   ├── Storage/
│   │   └── ...
│   │
│   └── Movement/                     # ✨ NEW: Movement PoCs
│       ├── README.md                 # Quick start guide
│       ├── MOVEMENT_ANALYSIS.md      # Detailed technical analysis
│       ├── CMakeLists.txt            # Build all PoCs
│       │
│       ├── LinearMovement/           # PoC #1
│       │   ├── LinearMovement.hpp    # Components + System
│       │   ├── main.cpp              # Demo program
│       │   └── CMakeLists.txt
│       │
│       ├── SineWaveMovement/         # PoC #2
│       │   ├── SineWaveMovement.hpp
│       │   ├── main.cpp
│       │   └── CMakeLists.txt
│       │
│       ├── BezierMovement/           # PoC #3
│       │   ├── BezierMovement.hpp
│       │   ├── main.cpp
│       │   └── CMakeLists.txt
│       │
│       └── ScriptedMovement/         # PoC #4
│           ├── ScriptedMovement.hpp
│           ├── main.cpp
│           ├── movement_script.txt   # Example script
│           └── CMakeLists.txt
│
└── docs/                             # ✨ NEW: Decision docs
    ├── movement_spike_summary.md     # Spike summary
    └── movement_system_decision.md   # Final decision
```

---

## 🎯 Movement Systems Overview

### 1️⃣ Linear Movement
```
Formula: pos += dir * speed * dt

┌─────────────────────────────────────────────►
│ Start                                    End
│ (0,0)                                   (100,0)
│
└─ Movement Direction

Perfect for: Bullets, projectiles, particles
Performance: ★★★★★ (100k+ entities @ 60 FPS)
Complexity:  ★☆☆☆☆
```

### 2️⃣ Sine Wave Movement
```
Formula: y = center + sin(time * freq) * amp

        ╱╲      ╱╲      ╱╲
    ───╱  ╲────╱  ╲────╱  ╲───  ← Center line
      ╱    ╲  ╱    ╲  ╱    ╲
     ╱      ╲╱      ╲╱      ╲
    │───────────────────────────► X (time)
    Start                    End

Perfect for: Wave formations, classic R-Type enemies
Performance: ★★★★☆ (50k+ entities @ 60 FPS)
Complexity:  ★★☆☆☆
```

### 3️⃣ Bézier Curve Movement
```
Formula (Quadratic): B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2

              P1 (Control)
               ●
              ╱ ╲
             ╱   ╲
            ╱     ╲
           ╱       ╲
          ╱         ╲
P0 ●────╱           ╲────● P2
Start   (Smooth Arc)   End

Perfect for: Boss entrances, dive attacks, cinematic moments
Performance: ★★☆☆☆ (10-20k entities @ 60 FPS)
Complexity:  ★★★☆☆
```

### 4️⃣ Scripted Movement
```
Format: Command(Param=Value, ...)

┌─────────────────────────────────────┐
│ # Movement Script                   │
│ Move(Linear, Speed=50, DirX=1)      │
│ Wait(Duration=1.0)                  │
│ MoveTo(X=100, Y=50, Speed=75)       │
│ Wait(Duration=0.5)                  │
│ Move(Linear, Speed=30, DirY=1)      │
└─────────────────────────────────────┘
           │
           ▼
    ┌──────────────┐
    │ Parser       │
    └──────┬───────┘
           │
           ▼
    ┌──────────────┐
    │ Commands     │  ──► Execute in sequence
    └──────────────┘

Perfect for: Complex patterns, boss behaviors, designer content
Performance: ★★☆☆☆ (5-10k entities @ 60 FPS)
Complexity:  ★★★★☆
```

---

## 📊 Comparison Matrix

### Performance Comparison
```
Entities @ 60 FPS
│
100k│ █ Linear
    │ █
50k │ █ █ Sine
    │ █ █
20k │ █ █ █ Bézier
    │ █ █ █
10k │ █ █ █ █ Scripted
    │ █ █ █ █
0   └─┴─┴─┴─┴────────►
      L S B SC
```

### Feature Comparison
```
                Linear  Sine  Bézier  Script
Performance      ████   ███   ██      █
Simplicity       ████   ███   ██      █
Visual Appeal    ██     ████  █████   ███
Flexibility      █      ██    ████    █████
Designer-Friendly ████  ███   ██      █████
```

---

## 🏗️ ECS Architecture

### Component-Based Design
```
Entity (ID: 12345)
├── Position Component
│   ├── x: 100.0
│   └── y: 50.0
│
├── Velocity Component (Linear)
│   ├── dx: 1.0
│   └── dy: 0.0
│
└── SineWave Component (Optional)
    ├── centerY: 50.0
    ├── frequency: 2.0
    ├── amplitude: 20.0
    └── time: 0.0
```

### System Execution Flow
```
Game Loop (60 FPS, 16.67ms budget)
│
├─► Linear Movement System (<0.1ms)
│   └─ Updates: Position += Velocity * dt
│
├─► Sine Wave System (<0.2ms)
│   └─ Modifies: Position.y based on sine wave
│
├─► Scripted Movement System (<0.5ms)
│   └─ Executes: Current command in script
│
└─► (Optional) Bézier System
    └─ Overrides: Position from curve evaluation

Total Movement Budget: <1.0ms (target <2ms)
```

---

## 🎮 Entity Type Mapping

### Recommended System per Entity Type
```
Bullets (2,000)
└─► Linear Movement
    • High performance needed
    • Simple straight-line pattern
    • Cost: ~0.05ms

Particles (5,000)
└─► Linear Movement
    • Maximum performance required
    • Short lifetime
    • Cost: ~0.1ms

Basic Enemies (100)
├─► Linear Movement (forward)
└─► Sine Wave Movement (oscillation)
    • Classic shooter patterns
    • Visual variety
    • Cost: ~0.15ms

Elite Enemies (20)
└─► Scripted Movement
    • Complex patterns
    • Unique behaviors
    • Cost: ~0.4ms

Bosses (1-3)
└─► Scripted Movement + Optional Bézier
    • Cinematic movements
    • Multi-phase patterns
    • Cost: ~0.5ms
```

---

## 🚀 Implementation Timeline

### Sprint 1 (Weeks 1-2) - Foundation
```
Week 1          Week 2
├────┬────┤    ├────┬────┤
│ Linear  │    │ Sine    │
│ System  │    │ System  │
└─────────┘    └─────────┘
     │              │
     └──────┬───────┘
            ▼
    [Basic Enemies]
    [Bullet System]
```

### Sprint 2 (Weeks 3-4) - Content Tools
```
Week 3          Week 4
├────┬────┤    ├────┬────┤
│ Script │    │ Designer│
│ Parser │    │ Docs    │
└─────────┘    └─────────┘
     │              │
     └──────┬───────┘
            ▼
    [Example Scripts]
    [Content Library]
```

### Sprint 3+ (Weeks 5-6) - Polish
```
Week 5          Week 6
├────┬────┤    ├────┬────┤
│Optimize│    │Evaluate │
│Systems │    │Bézier?  │
└─────────┘    └─────────┘
     │              │
     └──────┬───────┘
            ▼
   [Performance Tuning]
   [Advanced Features]
```

---

## ✅ Final Recommendation

### Core Implementation (MUST HAVE)
```
┌──────────────────────────────────┐
│  Linear Movement System          │ ← Sprint 1
│  + Highest performance            │
│  + Foundation for all entities    │
└──────────────────────────────────┘

┌──────────────────────────────────┐
│  Sine Wave Movement System       │ ← Sprint 1
│  + Classic arcade feel            │
│  + Good performance/appeal ratio  │
└──────────────────────────────────┘

┌──────────────────────────────────┐
│  Scripted Movement System        │ ← Sprint 2
│  + Designer empowerment           │
│  + Maximum flexibility            │
└──────────────────────────────────┘
```

### Optional Enhancement (NICE TO HAVE)
```
┌──────────────────────────────────┐
│  Bézier Movement System          │ ← Sprint 3+ (Conditional)
│  + Cinematic quality              │
│  + Visual polish                  │
│  ⚠ Requires visual editor tool   │
└──────────────────────────────────┘
```

---

## 📈 Success Metrics

### Performance Target
```
Total Entities: ~7,000 @ 60 FPS
┌─────────────────────────────────────┐
│ Bullets:        2,000  (Linear)     │
│ Particles:      5,000  (Linear)     │
│ Basic Enemies:    100  (Lin+Sine)   │
│ Elite Enemies:     20  (Scripted)   │
│ Bosses:          1-3   (Scripted)   │
├─────────────────────────────────────┤
│ Movement Budget: <1.0ms per frame   │
│ Memory Usage:    ~300 KB            │
└─────────────────────────────────────┘
```

### Gameplay Target
```
Enemy Variety
├─ 10+ distinct patterns
├─ Pattern creation <30 min
└─ Player engagement high

Visual Quality
├─ Classic arcade feel ✓
├─ Smooth animations ✓
└─ Impressive boss patterns ✓

Developer Experience
├─ Easy to add new enemies ✓
├─ Designer-friendly tools ✓
└─ Clear documentation ✓
```

---

## 🔗 Quick Links

### Code
- [All PoC Implementations](../../PoC/Movement/)
- [Linear PoC](../../PoC/Movement/LinearMovement/)
- [Sine Wave PoC](../../PoC/Movement/SineWaveMovement/)
- [Bézier PoC](../../PoC/Movement/BezierMovement/)
- [Scripted PoC](../../PoC/Movement/ScriptedMovement/)

### Documentation
- [📊 Movement Analysis](../../PoC/Movement/MOVEMENT_ANALYSIS.md)
- [📋 Final Decision](./movement_system_decision.md)
- [📝 Spike Summary](./movement_spike_summary.md)
- [🏗️ ECS Architecture](./ecs/README.md)

### Build & Run
```bash
cd PoC/Movement && mkdir build && cd build
cmake .. && make all_movement_pocs
./LinearMovement/linear_movement_poc
./SineWaveMovement/sine_wave_movement_poc
./BezierMovement/bezier_movement_poc
./ScriptedMovement/scripted_movement_poc
```

---

**Created:** November 24, 2025  
**Status:** ✅ Spike Complete  
**Next:** Implementation Sprint 1
