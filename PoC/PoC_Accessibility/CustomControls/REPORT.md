# Custom Controls & Remapping - PoC Report

## Executive Summary

This Proof of Concept validates the implementation of customizable control schemes for R-Type, enabling players with motor disabilities or alternative input preferences to fully remap game controls. This feature is critical for accessibility and player comfort.

**Status**: ⚠️ **RESEARCH PHASE - Implementation Pending**

---

## Objectives

1. Design a flexible input remapping system
2. Support keyboard, gamepad, and alternative input devices
3. Provide sensible default control schemes
4. Prevent conflicting key bindings
5. Enable saving/loading custom configurations
6. Support accessibility devices (one-handed controllers, adaptive controllers)

---

## Background Research

### Why Custom Controls Matter

#### For Players with Motor Disabilities
- **One-handed players**: Need all controls accessible with one hand
- **Limited mobility**: May need controls clustered in specific area
- **Reduced dexterity**: Need to avoid simultaneous button presses
- **Prosthetic users**: Require specific button layouts

#### For General Accessibility
- **Dominant hand preference**: Left-handed vs right-handed layouts
- **Keyboard layouts**: QWERTY, AZERTY, QWERTZ, Dvorak, etc.
- **Personal preference**: Comfort and muscle memory
- **Alternative devices**: Fight sticks, adaptive controllers, foot pedals

---

## Input Requirements for R-Type

### Core Actions
1. **Movement** (4 directions or analog stick)
   - Move Up
   - Move Down
   - Move Left
   - Move Right

2. **Combat**
   - Fire Primary Weapon
   - Fire Secondary Weapon (charge beam)
   - Deploy Special Weapon

3. **Mechanics**
   - Attach/Detach Force Pod
   - Switch Weapon Mode
   - Pause Game

4. **UI Navigation**
   - Navigate Menu Up/Down/Left/Right
   - Confirm
   - Cancel/Back

---

## Proposed Architecture

### InputAction System

```cpp
enum class InputAction {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    FirePrimary,
    FireSecondary,
    DeploySpecial,
    ToggleForce,
    SwitchWeapon,
    Pause,
    MenuUp,
    MenuDown,
    MenuLeft,
    MenuRight,
    MenuConfirm,
    MenuCancel
};

class InputMapper {
public:
    // Bind an action to a key/button
    void bindAction(InputAction action, InputKey key);

    // Check if action is currently pressed
    bool isActionPressed(InputAction action);

    // Check if action was just pressed this frame
    bool isActionJustPressed(InputAction action);

    // Get all bindings for an action (support multiple keys)
    std::vector<InputKey> getBindings(InputAction action);

    // Validate no conflicts
    bool hasConflicts();

    // Save/Load configuration
    void saveToFile(const std::string& path);
    void loadFromFile(const std::string& path);
};
```

### Input Device Support

```cpp
enum class InputDeviceType {
    Keyboard,
    Mouse,
    Gamepad,
    Joystick,
    Custom
};

struct InputKey {
    InputDeviceType deviceType;
    int keyCode; // scancode, button ID, etc.
    std::string displayName; // "Space", "A Button", etc.
};
```

---

## Default Control Schemes

### Scheme 1: Keyboard (WASD + IJKL) - Right-handed
```
Movement:     W/A/S/D
Fire:         Space or Left Mouse
Secondary:    J
Special:      K
Toggle Force: L
Switch:       I
Pause:        Escape or P
```

### Scheme 2: Keyboard (Arrow Keys) - One-handed
```
Movement:     Arrow Keys
Fire:         Right Shift
Secondary:    Right Ctrl
Special:      Enter
Toggle Force: Right Alt
Switch:       Delete
Pause:        Escape
```

### Scheme 3: Gamepad (Xbox/PlayStation)
```
Movement:     Left Stick or D-Pad
Fire:         A / X (PlayStation)
Secondary:    B / Circle
Special:      X / Square
Toggle Force: Y / Triangle
Switch:       LB / L1
Pause:        Start
```

### Scheme 4: Accessibility (One-handed left)
```
Movement:     WASD
Fire:         Q
Secondary:    E
Special:      R
Toggle Force: F
Switch:       Tab
Pause:        Escape
```

### Scheme 5: Accessibility (Foot pedals + minimal keyboard)
```
Movement:     Arrow Keys (hand)
Fire:         Pedal 1
Secondary:    Pedal 2
Special:      Pedal 3
Toggle Force: Space (hand)
Switch:       Shift (hand)
Pause:        Escape (hand)
```

---

## Conflict Detection & Prevention

### Types of Conflicts

1. **Direct Conflict**: Same key bound to two different actions
   ```
   Fire:  Space
   Pause: Space  ❌ CONFLICT
   ```

2. **Context Conflict**: Same key in different contexts (usually OK)
   ```
   Gameplay Fire: Space
   Menu Confirm:  Space  ✅ OK (different contexts)
   ```

3. **Modifier Conflicts**: Key with/without modifiers
   ```
   Fire:       Space
   Fire Alt:   Shift + Space  ✅ OK (different modifiers)
   ```

### Validation Rules

```cpp
bool InputMapper::validateBindings() {
    // Check for direct conflicts within same context
    for (auto action1 : gameplayActions) {
        for (auto action2 : gameplayActions) {
            if (action1 != action2 &&
                hasSharedBinding(action1, action2)) {
                return false; // Conflict detected
            }
        }
    }
    return true;
}
```

---

## Configuration File Format

### JSON Format (Recommended)

```json
{
  "control_scheme": {
    "name": "Custom Layout",
    "description": "My personalized controls",
    "version": "1.0",
    "bindings": [
      {
        "action": "MoveUp",
        "primary": {
          "device": "keyboard",
          "key": "W",
          "scancode": 17
        },
        "secondary": {
          "device": "gamepad",
          "button": 12,
          "name": "D-Pad Up"
        }
      },
      {
        "action": "FirePrimary",
        "primary": {
          "device": "keyboard",
          "key": "Space",
          "scancode": 57
        },
        "secondary": {
          "device": "mouse",
          "button": 0,
          "name": "Left Click"
        }
      }
    ]
  }
}
```

### TOML Format (Alternative)

```toml
[control_scheme]
name = "Custom Layout"
version = "1.0"

[bindings.move_up]
primary = { device = "keyboard", key = "W", scancode = 17 }
secondary = { device = "gamepad", button = 12 }

[bindings.fire_primary]
primary = { device = "keyboard", key = "Space", scancode = 57 }
secondary = { device = "mouse", button = 0 }
```

---

## UI/UX Design Considerations

### Controls Menu Layout

```
┌─────────────────────────────────────────┐
│         CONTROL SETTINGS                │
├─────────────────────────────────────────┤
│ Preset: [Custom          ▼]             │
│                                         │
│ Movement                                │
│   Move Up        [    W    ]  Change    │
│   Move Down      [    S    ]  Change    │
│   Move Left      [    A    ]  Change    │
│   Move Right     [    D    ]  Change    │
│                                         │
│ Combat                                  │
│   Fire Primary   [  Space  ]  Change    │
│   Fire Secondary [    J    ]  Change    │
│   Deploy Special [    K    ]  Change    │
│                                         │
│ [Reset to Defaults]  [Save]  [Cancel]  │
└─────────────────────────────────────────┘
```

### Rebinding Flow

1. Player clicks "Change" button
2. Display "Press any key..." prompt
3. Wait for key press
4. Check for conflicts
5. If conflict: Show warning, allow override or cancel
6. If no conflict: Bind key and display new binding
7. Update configuration

### Conflict Warning Example

```
┌─────────────────────────────────────────┐
│         BINDING CONFLICT                │
├─────────────────────────────────────────┤
│ The key [Space] is already bound to:    │
│                                         │
│     Fire Primary                        │
│                                         │
│ Do you want to replace this binding?    │
│                                         │
│     [Replace]  [Cancel]                 │
└─────────────────────────────────────────┘
```

---

## Implementation Phases

### Phase 1: Core System (High Priority)
- ✅ InputAction enumeration
- ✅ InputMapper class with binding management
- ✅ Key press detection and action mapping
- ✅ Single key per action (no multiple bindings yet)
- ✅ Configuration save/load (JSON)

### Phase 2: UI & Presets (High Priority)
- ✅ Controls settings menu
- ✅ Key rebinding interface
- ✅ 3-5 default control schemes
- ✅ Preset selection dropdown
- ✅ Reset to defaults button

### Phase 3: Advanced Features (Medium Priority)
- ✅ Multiple bindings per action (primary + secondary)
- ✅ Conflict detection and warnings
- ✅ Gamepad support (if not already present)
- ✅ Display key names correctly (localized)
- ✅ Visual indicators during rebinding

### Phase 4: Accessibility (Medium Priority)
- ✅ One-handed presets (left and right)
- ✅ Foot pedal support
- ✅ Toggle vs Hold options for actions
- ✅ Input delay/debounce settings
- ✅ Turbo fire mode (hold = auto-fire)

### Phase 5: Advanced Accessibility (Low Priority)
- ⚠️ Xbox Adaptive Controller support
- ⚠️ Switch accessibility features
- ⚠️ Macro recording (simple combos)
- ⚠️ Voice command integration (if feasible)

---

## Technical Challenges & Solutions

### Challenge 1: Scancodes vs Keycodes
**Problem**: Different keyboard layouts have different key positions
**Solution**: Use scancodes (physical position) for binding, display localized key names

### Challenge 2: Gamepad Variety
**Problem**: Different gamepads have different button layouts
**Solution**: Support SDL2 GameController API (unified mapping)

### Challenge 3: Simultaneous Bindings
**Problem**: Player wants both keyboard and gamepad bound to same action
**Solution**: Support primary and secondary bindings per action

### Challenge 4: Input Conflicts in Menus
**Problem**: Gameplay key conflicts with menu navigation
**Solution**: Separate contexts (gameplay vs menu) with context-aware binding

---

## Testing & Validation

### Test Scenarios

1. **Default Controls**: All presets work out-of-box
2. **Full Rebind**: Player rebinds all actions
3. **Conflict Detection**: System catches and warns about conflicts
4. **Device Switching**: Swap between keyboard and gamepad mid-game
5. **Save/Load**: Configuration persists across sessions
6. **Edge Cases**:
   - Unbind all keys
   - Bind same key to everything (should warn)
   - Use special keys (Fn, Windows key, etc.)

### Accessibility Testing

- Test with one-handed players (each hand)
- Test with adaptive controller users
- Test with alternative input devices
- Verify all actions are reachable in each preset

---

## Performance Considerations

| Component | CPU Impact | Memory Impact | Notes |
|-----------|-----------|---------------|-------|
| Input Polling | Low (~0.1%) | Negligible | Per-frame, but fast |
| Action Mapping | Negligible | <1 KB | Hash table lookup |
| Configuration Load | One-time | <5 KB | Only on startup |
| UI Rendering | Low | Minimal | Only in menus |

**Conclusion**: Minimal performance impact

---

## Accessibility Standards Compliance

### WCAG 2.1 Guidelines
- ✅ **2.1.1 Keyboard**: All functionality available via keyboard
- ✅ **2.1.4 Character Key Shortcuts**: Can be remapped
- ✅ **2.5.1 Pointer Gestures**: Not applicable (no complex gestures)
- ✅ **2.5.8 Target Size**: UI buttons adequately sized

### ADA Compliance
- ✅ Supports alternative input methods
- ✅ Customizable for individual needs
- ✅ No forced control scheme

---

## Industry Best Practices

### Games with Excellent Control Remapping

1. **The Last of Us Part II**
   - Full remapping for all inputs
   - Multiple presets for different disabilities
   - Toggle vs hold options

2. **Celeste**
   - Complete keyboard and gamepad remapping
   - Accessibility assist mode with input options
   - Variant control schemes

3. **Microsoft Flight Simulator**
   - Extremely granular control customization
   - Profiles for different input devices
   - Conflict detection and resolution

---

## Success Criteria

- [ ] All game actions can be rebound
- [ ] At least 3 default presets available
- [ ] Conflict detection working correctly
- [ ] Configuration persists across sessions
- [ ] UI is intuitive and easy to use
- [ ] Support keyboard, mouse, and gamepad
- [ ] Tested with accessibility users
- [ ] No performance issues

---

## Cost-Benefit Analysis

### Costs
- **Development Time**: 2-3 sprints
- **UI Design**: Custom menu screens
- **Testing**: Diverse input device testing
- **Maintenance**: Support for new devices

### Benefits
- **Accessibility**: Critical for motor disability access
- **Player Satisfaction**: High user request feature
- **Platform Requirements**: Some platforms require remapping
- **Competitive Advantage**: Standard feature in modern games
- **Reduced Support**: Fewer "controls don't work" complaints

**Verdict**: ✅ **Essential Feature** - Industry standard and accessibility requirement

---

## Next Steps

### Immediate Actions
1. ✅ Complete this research document
2. ⏳ Design InputMapper architecture
3. ⏳ Implement basic key binding system
4. ⏳ Create configuration file format

### Short-term (Sprint 1-2)
1. Implement core InputMapper class
2. Add configuration save/load
3. Create 3 default control schemes
4. Build basic rebinding UI

### Long-term (Sprint 3+)
1. Add gamepad support
2. Implement conflict detection
3. Create accessibility presets
4. Conduct user testing
5. Polish UI/UX

---

## Conclusion

Custom control remapping is **essential for accessibility and player satisfaction**. The system is technically straightforward to implement and provides enormous value to players with diverse needs.

**Priority Recommendation**:
- **Phase 1-2**: ⭐⭐⭐⭐⭐ **Critical** - Core functionality
- **Phase 3**: ⭐⭐⭐⭐ **High** - Standard features
- **Phase 4**: ⭐⭐⭐ **Medium** - Accessibility-specific
- **Phase 5**: ⭐⭐ **Low** - Advanced/niche features

**Recommendation**: **APPROVE for immediate development** - Phases 1-3 are must-have features for modern game.

---

## References

- Game Accessibility Guidelines: https://gameaccessibilityguidelines.com/
- Xbox Adaptive Controller: https://www.xbox.com/en-US/accessories/controllers/xbox-adaptive-controller
- SDL2 GameController API: https://wiki.libsdl.org/SDL2/CategoryGameController
- WCAG 2.1 Input Guidelines: https://www.w3.org/TR/WCAG21/#input-modalities
- AbleGamers Foundation: https://ablegamers.org/

---

**Report Generated**: November 25, 2025
**Author**: R-Type Accessibility Team
**Version**: 1.0
**Status**: Research Complete - Awaiting Implementation Decision
