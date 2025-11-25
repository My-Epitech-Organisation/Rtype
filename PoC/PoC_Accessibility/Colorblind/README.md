# Colorblind Accessibility PoC

## Overview

This Proof of Concept demonstrates the colorblind accessibility features described in the `colorblind_and_visual_sound_cues` document. It validates the following requirements:

1. **Colorblind-Safe Palette**: High-contrast colors that remain distinguishable across different types of color vision deficiency (CVD)
2. **High-Contrast Outlines**: All projectiles have thick, contrasting outlines
3. **Shape-Based Differentiation**: Different projectile types use different shapes (diamond, square, triangle)
4. **Visual Sound Cues**: Visual alternatives for important audio events
5. **Real-Time CVD Simulation**: Test visuals under different colorblind conditions

## Features Demonstrated

### Colorblind Simulation

The PoC implements transformation matrices for three main types of CVD:

- **Protanopia** (Red-blind): ~1% of males
- **Deuteranopia** (Green-blind): ~1% of males  
- **Tritanopia** (Blue-blind): ~0.001% of population

Press keys `1-4` to cycle through Normal, Protanopia, Deuteranopia, and Tritanopia modes.

### Visual Cue System

Three types of visual cues are implemented:

1. **Hit Indicator** (Press H)
   - Red flash on screen edges
   - Pulsing borders
   - Duration: 0.3 seconds
   - Simulates: Damage taken sound

2. **Missile Warning** (Press M)
   - Yellow warning triangle with "!" symbol
   - Pulsing animation
   - Directional indicator
   - Duration: 1.0 seconds
   - Simulates: Missile lock sound

3. **Power-Up Spawn** (Press P)
   - Expanding cyan rings
   - Central sparkle effect
   - Duration: 0.8 seconds
   - Simulates: Power-up appearance sound

### Projectile Differentiation

All projectiles use multiple visual cues:

- **Player Bullet**: Cyan diamond with white outline
- **Enemy Bullet**: Orange square with dark red outline
- **Missile**: Yellow triangle with dark red outline + pulsing glow effect

Each projectile type is distinguishable by:
1. Color (with CVD-safe palette)
2. Shape (geometric differentiation)
3. Outline (high-contrast borders)
4. Animation (missiles pulse)

## Building

### Prerequisites

- CMake 3.20+
- C++20 compatible compiler
- SFML 2.5+

### Build Instructions

```bash
cd PoC/PoC_Accessibility/Colorblind
mkdir build
cd build
cmake ..
make
./colorblind_poc
```

Or use the provided build script from the project root:

```bash
./build.sh
./build/PoC/PoC_Accessibility/Colorblind/colorblind_poc
```

## Controls

### Movement
- **Arrow Keys**: Move player ship
- **Space**: Fire player bullet

### Spawning
- **E**: Spawn enemy bullets (5 at random Y positions)
- **M**: Spawn missile with warning visual cue

### Visual Cues (Simulating Audio Events)
- **H**: Trigger hit indicator (damage taken)
- **P**: Trigger power-up spawn cue

### Colorblind Modes
- **1**: Normal Vision
- **2**: Protanopia (Red-blind)
- **3**: Deuteranopia (Green-blind)
- **4**: Tritanopia (Blue-blind)

### UI
- **F1**: Toggle help overlay
- **ESC**: Exit

## Validation Results

### Accessibility Compliance

✅ **4.5:1 Contrast Ratio**: All projectiles meet WCAG AA standards for contrast
✅ **CVD Testing**: Projectiles remain distinguishable in all CVD modes
✅ **Shape Differentiation**: Projectiles identifiable by shape alone
✅ **Visual Sound Cues**: All critical audio events have visual equivalents
✅ **No Color-Only Information**: Multiple visual channels used for all information

### Technical Implementation

- **ColorblindSimulator**: Applies mathematical CVD transformations using industry-standard matrices (Brettel, Viénot and Mollon algorithm)
- **VisualCueSystem**: Event-driven system for triggering visual cues with lifetime management
- **Projectile**: Template for accessible game objects with multiple differentiation cues

## Design Decisions

### Outline Thickness

3-pixel outlines chosen based on:
- Visibility at 1080p resolution
- Performance considerations
- WCAG contrast requirements

### Color Palette

Base colors selected for:
- Maximum distinction in CVD modes
- High luminance contrast with dark backgrounds
- Alignment with R-Type aesthetic

| Element | Color | Rationale |
|---------|-------|-----------|
| Player Bullet | Cyan (0, 200, 255) | High visibility, distinct from enemies |
| Enemy Bullet | Orange (255, 120, 0) | Warning color, distinct from player |
| Missile | Yellow (255, 255, 0) | Alert color, highest urgency |
| Player Ship | Green (0, 255, 100) | Life/friendly indicator |

### Visual Cue Timing

- **Hit Indicator**: 0.3s - Brief flash, non-intrusive
- **Missile Warning**: 1.0s - Longer duration for reaction time
- **Power-Up Spawn**: 0.8s - Noticeable but not distracting

## References

Implementation based on:

1. **Brettel, Viénot and Mollon (1997)**: "Computerized simulation of color appearance for dichromats"
2. **WCAG 2.1**: Contrast requirements (Section 1.4.3)
3. **Game Accessibility Guidelines**: Color blindness section
4. **Adobe Color Accessibility**: CVD simulation tools

## Future Enhancements

Potential improvements for production:

1. **Shader-Based CVD Simulation**: Apply transformation as post-processing for better performance
2. **Additional CVD Types**: Anomalous trichromacy variants
3. **User Settings**: Adjustable outline thickness, cue duration
4. **Sound Integration**: Play actual sounds with visual cues
5. **Subtitle System**: For narrative elements (as described in doc Section 7)

## Conclusion

This PoC validates that the accessibility guidelines in the documentation are:

1. **Technically Feasible**: All features implemented successfully
2. **Performant**: Runs at 60 FPS with multiple projectiles and visual cues
3. **Effective**: Projectiles remain distinguishable in all CVD modes
4. **User-Friendly**: Visual cues are clear and non-intrusive

The implementation demonstrates that R-Type can be made accessible to colorblind players without compromising visual quality or gameplay.
