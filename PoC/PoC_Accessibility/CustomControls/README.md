# Accessibility PoC: Custom Controls & Auto-Fire

## Overview

This Proof of Concept (PoC) validates the accessibility features described in `accessibility_custom_controls` document. It demonstrates a complete implementation of custom keybinding remapping and auto-fire functionality to support players with motor impairments.

## Features Demonstrated

### 1. **Input Remapping System** (Section 4 of doc)
- Central `InputManager` component that stores active keybindings
- Ability to bind actions (MoveUp, MoveDown, Fire, etc.) to custom keys
- Duplicate key validation to prevent conflicts
- Default WASD controls with support for remapping to Arrow Keys or any other layout

### 2. **Config File Persistence** (Section 5 of doc)
- `InputConfigSerializer` for loading/saving keybindings
- JSON-based config format matching the document specification:
  ```json
  {
    "controls": {
      "move_up": "KeyW",
      "move_left": "KeyA",
      "move_down": "KeyS",
      "move_right": "KeyD",
      "fire": "Space",
      "auto_fire": false
    }
  }
  ```
- Config persistence across sessions

### 3. **Auto-Fire System** (Section 6 of doc)
- `AutoFireComponent` implementing both Hold and Toggle modes
- **Hold Mode**: Fire continuously while key is pressed (default behavior)
- **Toggle Mode**: Press once to start auto-fire, press again to stop
- Configurable fire rate (interval between shots)
- Reduces motor fatigue from repetitive key presses

### 4. **Accessibility Benefits**
- Supports players with limited mobility who can't use WASD
- Reduces strain for players with repetitive stress injuries
- Compatible with adaptive controllers through flexible binding system
- Backward compatible with standard controls

## Architecture

The PoC follows the architecture recommended in the document:

```
┌─────────────────┐
│  InputManager   │ ← Central keybinding storage
└────────┬────────┘
         │
         ├─────────────────────┐
         │                     │
┌────────▼──────────┐  ┌──────▼─────────────┐
│ InputConfigSer... │  │ AutoFireComponent  │
│ (Load/Save JSON)  │  │ (Hold/Toggle mode) │
└───────────────────┘  └────────────────────┘
```

## Building and Running

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- CMake 3.20 or higher

### Build Instructions

```bash
# Navigate to the PoC directory
cd PoC/PoC_Accessibility

# Create build directory
mkdir -p build && cd build

# Configure and build
cmake ..
make

# Run the PoC
./accessibility_poc
```

## Demo Output

The PoC runs four demonstrations:

1. **Input Remapping Demo**: Shows default WASD controls, remaps to Arrow Keys, and validates duplicate key prevention
2. **Config Serialization Demo**: Saves bindings to JSON file and loads them back
3. **Auto-Fire Demo**: Demonstrates both Hold and Toggle modes with simulated gameplay
4. **Accessibility Benefits**: Shows how features help players with different needs

## Files

- `main.cpp` - Main demo program with 4 test scenarios
- `InputManager.hpp/cpp` - Core keybinding management system
- `InputConfigSerializer.hpp/cpp` - JSON config persistence
- `AutoFireComponent.hpp/cpp` - Auto-fire implementation
- `CMakeLists.txt` - Build configuration
- `test_controls.json` - Generated config file (created at runtime)

## Validation Against Document Requirements

| Requirement | Status | Implementation |
|-------------|--------|----------------|
| Custom key bindings | ✅ | `InputManager::bindKey()` |
| Config file storage | ✅ | `InputConfigSerializer::saveToFile()` |
| Config file loading | ✅ | `InputConfigSerializer::loadFromFile()` |
| Auto-fire toggle | ✅ | `AutoFireComponent` with Toggle mode |
| Duplicate key prevention | ✅ | `InputManager::isKeyAlreadyBound()` |
| Backward compatibility | ✅ | Default WASD in constructor |
| Low performance overhead | ✅ | Simple hash maps, O(1) lookups |

## Design Decisions

### Why Manual JSON Parsing?
For this PoC, we used simple manual JSON parsing to avoid external dependencies. In production, use a proper JSON library (nlohmann/json, RapidJSON, etc.).

### Why Raw Time-Based Auto-Fire?
The auto-fire uses `std::chrono` for timing to demonstrate the core concept. In a real game engine, this would integrate with the game loop's delta time.

### Keyboard Codes
The PoC uses an enum for key codes. In production with SDL/SFML/Raylib, you'd use the library's native key codes.

## Future Enhancements

Based on the document, the following could be added:
- Full Controls Menu UI implementation
- Support for mouse buttons and gamepad inputs
- Key capture for "press any key" rebinding
- Multiple control profiles
- Import/export control schemes
- Accessibility testing with adaptive controllers

## References

From the document:
- SDL Input Handling: https://wiki.libsdl.org/SDL3/CategoryKeyboard
- Game Accessibility Guidelines: https://gameaccessibilityguidelines.com/motor-
- Microsoft Inclusive Input Design: https://learn.microsoft.com/en-us/gaming/accessibility/apx-input

## Conclusion

This PoC successfully validates the technical feasibility of the accessibility features described in the `accessibility_custom_controls` document. All core requirements are met:

✅ Custom keybinding remapping
✅ JSON config persistence
✅ Auto-fire (Hold and Toggle modes)
✅ Duplicate key validation
✅ Backward compatibility
✅ Low performance overhead

The implementation provides a solid foundation for integrating these accessibility features into the R-Type game engine.
