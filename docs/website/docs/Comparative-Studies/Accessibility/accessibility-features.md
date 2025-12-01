# Accessibility Features

Comparative analysis of accessibility features for inclusive game design.

## Overview

This document synthesizes three Proof of Concept studies on accessibility features to make R-Type playable by the widest possible audience.

## Features Comparison

| Feature | Accessibility Impact | Technical Complexity | Priority |
|---------|---------------------|---------------------|----------|
| **Slow Mode / Time Scaling** | High (15-20% players) | Low | Critical |
| **Custom Control Remapping** | High (10-15% players) | Medium | Critical |
| **Colorblind Support** | Medium (5-8% players) | Medium | High |

---

## Feature 1: Slow Mode and Time Scaling

### Concept

Global time scale system allowing players to slow down gameplay by 30-50% to accommodate slower reaction times or motor disabilities.

### Target Audience

- Players with slower reaction times (aging, cognitive disabilities)
- Players with motor disabilities requiring more time
- Beginners and casual players
- **Estimated reach**: 15-20% of potential player base

### PoC Results

| Metric | Result |
|--------|--------|
| **Technical Validation** | Complete |
| **Physics Consistency** | 0.00% position error across time scales |
| **Performance Overhead** | Approximately 0.1% CPU |
| **Integration Path** | Clear and straightforward |

### Implementation

```cpp
class TimeSystem {
public:
    void setTimeScale(float scale) {
        _timeScale = std::clamp(scale, 0.3f, 2.0f);
    }
    
    float getDeltaTime() const {
        return _rawDeltaTime * _timeScale;
    }
    
    float getRawDeltaTime() const {
        return _rawDeltaTime;  // For audio (keeps normal speed)
    }

private:
    float _timeScale = 1.0f;
    float _rawDeltaTime = 0.0f;
};
```

### Recommended Presets

| Preset | Time Scale | Use Case |
|--------|------------|----------|
| Slow | 50% | Accessibility / Beginner |
| Normal | 100% | Standard gameplay |
| Fast | 150% | Challenge mode |
| Custom | 30-200% | Advanced users |

### Decision

**APPROVED for immediate implementation**

- Low implementation cost (1-2 sprints)
- High accessibility impact
- No significant downsides
- Core differentiator for accessibility

---

## Feature 2: Custom Control Remapping

### Concept

Complete input rebinding system allowing players to remap all game controls to keyboard, mouse, gamepad, or alternative input devices.

### Target Audience

- Players with motor disabilities (one-handed, limited mobility)
- Left-handed players
- Users of adaptive controllers (Xbox Adaptive Controller)
- Different keyboard layouts (AZERTY, QWERTZ, Dvorak)
- **Estimated reach**: 10-15% of player base

### Industry Requirement

Control remapping is **mandatory** for many platform certifications:
- Xbox Accessibility Guidelines
- PlayStation certification requirements
- Steam recommendations

### Architecture

```cpp
class InputMapper {
public:
    void rebind(Action action, InputBinding newBinding) {
        if (hasConflict(newBinding)) {
            resolveConflict(newBinding);
        }
        _bindings[action] = newBinding;
    }
    
    bool isActionPressed(Action action) const {
        return _bindings.at(action).isPressed();
    }
    
    void saveToFile(const std::string& path) const;
    void loadFromFile(const std::string& path);

private:
    std::unordered_map<Action, InputBinding> _bindings;
    bool hasConflict(const InputBinding& binding) const;
    void resolveConflict(const InputBinding& binding);
};
```

### Configuration Format (JSON)

```json
{
    "version": "1.0",
    "bindings": {
        "move_up": { "type": "keyboard", "key": "W" },
        "move_down": { "type": "keyboard", "key": "S" },
        "shoot": { "type": "mouse", "button": "left" },
        "special": { "type": "gamepad", "button": "A" }
    },
    "preset": "custom"
}
```

### Implementation Phases

| Phase | Priority | Content | Timeline |
|-------|----------|---------|----------|
| Phase 1 | Critical | Core rebinding, keyboard | Sprint 1 |
| Phase 2 | Critical | UI, presets, persistence | Sprint 2 |
| Phase 3 | High | Gamepad, conflict detection | Sprint 3 |
| Phase 4 | Medium | Accessibility presets (one-handed) | Sprint 4 |
| Phase 5 | Low | Macros, voice control | Future |

### Decision

**APPROVED** (Phases 1-3 immediately)

---

## Feature 3: Colorblind Support

### Concept

Alternative color palettes and shape-based distinctions for players with color vision deficiencies.

### Target Audience

| Type | Prevalence | Colors Affected |
|------|------------|-----------------|
| **Protanopia** | 1% males | Red-green confusion |
| **Deuteranopia** | 6% males | Red-green confusion |
| **Tritanopia** | 0.01% | Blue-yellow confusion |
| **Total Impact** | 8-10% males, 0.5% females | ~5-8% of players |

### Solution Approaches

**1. Shape-Based Distinctions**

```
Power-ups with unique shapes:
┌─────┐   ┌─────┐   ┌─────┐
│  ★  │   │  ●  │   │  ◆  │
│Speed│   │Fire │   │Shield│
└─────┘   └─────┘   └─────┘
```

**2. Palette Swapping**

```cpp
class ColorPalette {
public:
    enum class Mode {
        Normal,
        Protanopia,
        Deuteranopia,
        Tritanopia,
        HighContrast
    };
    
    Color transform(Color original) const {
        switch (_mode) {
            case Mode::Protanopia:
                return applyProtanopiaMatrix(original);
            case Mode::HighContrast:
                return increaseContrast(original);
            default:
                return original;
        }
    }

private:
    Mode _mode = Mode::Normal;
};
```

**3. High Contrast Mode**

Essential for low vision players:
- Maximum contrast between game elements
- Clear outlines on all interactive objects
- Simplified backgrounds

### Implementation Phases

| Phase | Priority | Content |
|-------|----------|---------|
| Phase 1 | High | Shape distinctions, high contrast |
| Phase 2 | Medium | Three colorblind palettes |
| Phase 3 | Low | Custom color picker |

### Decision

**APPROVED** (Phase 1 for next update)

---

## Combined Implementation Priority

### Milestone 1: Core Accessibility (Sprints 1-3)

1. **Slow Mode** - Full implementation
2. **Custom Controls** - Phases 1-2

### Milestone 2: Enhanced Accessibility (Sprints 4-6)

3. **Custom Controls** - Phase 3
4. **Colorblind Support** - Phase 1

### Milestone 3: Advanced Features (Sprints 7-10)

5. **Colorblind Support** - Phase 2
6. **Custom Controls** - Phase 4
7. Polish and testing

---

## Success Metrics

| Feature | Target |
|---------|--------|
| **Slow Mode** | 90%+ testers with slow reactions complete level 1 |
| **Custom Controls** | 100% actions rebindable without conflicts |
| **Colorblind** | 90%+ colorblind testers distinguish all critical elements |

---

## Final Decision Summary

| Feature | Status | Impact | Cost |
|---------|--------|--------|------|
| **Slow Mode** | Approved | High | Low |
| **Custom Controls** | Approved (Phases 1-3) | High | Medium |
| **Colorblind Support** | Approved (Phase 1) | Medium | Medium |

**All three features are approved** because:
1. They transform "unplayable" to "playable" for many users
2. Industry standard expectations
3. Reasonable implementation cost
4. Demonstrates commitment to inclusivity

---

## References

- PoC implementations: `/PoC/PoC_Accessibility/`
- Slow Mode PoC: `/PoC/PoC_Accessibility/SlowMode/`
- Custom Controls PoC: `/PoC/PoC_Accessibility/CustomControls/`
- Colorblind PoC: `/PoC/PoC_Accessibility/Colorblind/`
- Decision document: `/PoC/PoC_Accessibility/ACCESSIBILITY_FINAL_DECISION.md`
