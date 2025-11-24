# Movement Systems PoC - Quick Start Guide

## Overview

This directory contains four proof-of-concept implementations for different movement systems in the R-Type game engine, all built using the ECS framework from `PoC/ECS`.

## PoC Directories

1. **LinearMovement/** - Simple directional movement (`pos += dir * speed * dt`)
2. **SineWaveMovement/** - Oscillating wave patterns (`y = center + sin(t * freq) * amp`)
3. **BezierMovement/** - Smooth curved paths using Bézier mathematics
4. **ScriptedMovement/** - Text-based movement scripting system

## Building the PoCs

### Build All PoCs
```bash
cd PoC/Movement
mkdir build && cd build
cmake ..
make all_movement_pocs
```

### Build Individual PoCs
```bash
# Linear Movement
cd PoC/Movement/LinearMovement
mkdir build && cd build
cmake ..
make

# Sine Wave Movement
cd PoC/Movement/SineWaveMovement
mkdir build && cd build
cmake ..
make

# Bézier Movement
cd PoC/Movement/BezierMovement
mkdir build && cd build
cmake ..
make

# Scripted Movement
cd PoC/Movement/ScriptedMovement
mkdir build && cd build
cmake ..
make
```

## Running the PoCs

```bash
# From the build directory of each PoC
./linear_movement_poc
./sine_wave_movement_poc
./bezier_movement_poc
./scripted_movement_poc
```

## Expected Output

Each PoC will:
1. Display the movement formula being demonstrated
2. Create several test entities with different parameters
3. Simulate movement over multiple frames
4. Print position updates to console
5. Show a summary of pros/cons

## Documentation

- **MOVEMENT_ANALYSIS.md** - Comprehensive analysis of all four systems
  - Detailed pros/cons for each system
  - Performance comparisons
  - Use case recommendations
  
- **../../docs/movement_system_decision.md** - Final architectural decision
  - Selected systems for implementation
  - Implementation phases
  - Performance targets
  - Risk analysis

## Quick Comparison

| System | Performance | Complexity | Visual Appeal | Designer Friendly |
|--------|-------------|------------|---------------|-------------------|
| Linear | ⭐⭐⭐⭐⭐ | ⭐ | ⭐⭐ | ⭐⭐⭐⭐⭐ |
| Sine Wave | ⭐⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ |
| Bézier | ⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐ |
| Scripted | ⭐⭐ | ⭐⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |

## Recommended Approach

Based on the PoC results, we recommend implementing:

1. **Linear Movement** (Sprint 1) - Foundation for all basic entities
2. **Sine Wave Movement** (Sprint 1) - Classic shooter patterns
3. **Scripted Movement** (Sprint 2) - Designer empowerment
4. **Bézier Movement** (Optional, Sprint 3+) - Cinematic polish

See `docs/movement_system_decision.md` for full rationale.

## Code Structure

Each PoC follows this structure:
```
<SystemName>/
├── CMakeLists.txt           # Build configuration
├── <SystemName>.hpp         # Component and system definitions
├── main.cpp                 # Test/demo program
└── [optional files]         # Additional resources
```

## Integration with Main Project

To integrate a movement system:

1. Copy the `.hpp` file to `include/rtype/engine/movement/`
2. Update component definitions to match project style
3. Register systems with the main ECS registry
4. Add to system scheduler with appropriate priority

## Performance Notes

All PoCs were tested with the following assumptions:
- Target: 60 FPS (16.67ms per frame)
- Test hardware: Modern CPU (2020+)
- Compiled with: `-O2` optimization

Actual performance may vary based on:
- Number of active entities
- Other systems running concurrently
- Target platform specifications

## Next Steps

1. Review both documentation files
2. Run all PoCs to see them in action
3. Discuss findings with the team
4. Begin implementation of selected systems

## Questions?

Refer to:
- `MOVEMENT_ANALYSIS.md` for technical details
- `../../docs/movement_system_decision.md` for strategic decisions
- `../../docs/ecs/README.md` for ECS framework documentation
