# Accessibility: Custom Controls & Auto-Fire Strategy

## 1. Problem Statement

Some players cannot comfortably use the standard WASD or Arrow Keys layout to control the game. Additionally, repetitive actions such as pressing the **Spacebar** for firing can cause motor fatigue. This document proposes a technical strategy for supporting custom input remapping and optional auto-fire.

---

## 2. Scope

This investigation covers:

* **Input Remapping:** How the game engine can store and load custom keybindings via the Config file.
* **Toggle vs Hold:** How to implement an "Auto-Fire" mode that can be toggled, rather than requiring repeated keypresses.

---

## 3. Requirements

### Functional Requirements

* Users must be able to customize movement and action keys in a dedicated **Controls** menu.
* Custom bindings must be stored and reloaded from the game's Config file.
* Players must be able to toggle Auto-Fire.

### Non-Functional Requirements

* Accessibility-first design.
* Backward compatibility: default WASD/Arrows should still work.
* Low performance overhead.

---

## 4. Controls Menu Strategy

A dedicated **Controls** menu should allow players to select an action (e.g. "Move Up", "Shoot") and then press a new key. The system must:

1. Capture the next key input.
2. Validate it (avoid duplicates, forbidden keys, engine-specific limitations).
3. Save it to the Config file.
4. Update the runtime Input Manager.

### Recommended Architecture

* **InputManager**: Central component storing active keybindings.
* **InputConfigSerializer**: Loads/saves keybindings from a config file.
* **ControlsMenuUI**: Allows the user to assign new keys.

### Documentation References

* SDL Input Handling: [https://wiki.libsdl.org/SDL3/CategoryKeyboard](https://wiki.libsdl.org/SDL3/CategoryKeyboard)
* Unity Input System (for architectural inspiration): [https://docs.unity3d.com/Packages/com.unity.inputsystem@1.7/manual/index.html](https://docs.unity3d.com/Packages/com.unity.inputsystem@1.7/manual/index.html)
* Godot InputMap: [https://docs.godotengine.org/en/stable/classes/class_inputmap.html](https://docs.godotengine.org/en/stable/classes/class_inputmap.html)

---

## 5. Config File Structure

A suggested Config format:

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

The system must map string key names to the engine's internal keycode representation.

### Reference: JSON Config Storage

* [https://www.json.org/json-en.html](https://www.json.org/json-en.html)

---

## 6. Auto-Fire Logic

Auto-Fire allows a player to continuously shoot without holding or pressing the fire key.

### Modes

* **Hold Mode:** Default; user must keep the key pressed.
* **Toggle Mode:** Press once → auto-fire ON; press again → OFF.

### Auto-Fire Component Responsibilities

* Read the `auto_fire` flag from the Config.
* Implement a timer-based firing loop.
* Allow switching modes via the Controls menu.

### Design Sketch

```
if auto_fire_enabled:
    if toggle_pressed:
        autofire_active = !autofire_active

if autofire_active:
    fire_weapon_every(interval)
```

### Inspiration & Accessibility Guidelines

* Game Accessibility Guidelines (Motor Disabilities): [https://gameaccessibilityguidelines.com/motor-](https://gameaccessibilityguidelines.com/motor-)
* Microsoft Inclusive Input Design Docs: [https://learn.microsoft.com/en-us/gaming/accessibility/apx-input](https://learn.microsoft.com/en-us/gaming/accessibility/apx-input)

---

## 7. Validation Strategy

To meet the exit criteria:

* Verify that keybindings can be modified, serialized, and reloaded.
* Confirm that Auto-Fire toggle behaves consistently.
* Perform user testing with non-standard input devices.

---

## 8. Deliverables

* **Controls Menu Strategy** (UI + Input architecture)
* **Auto-Fire Component Logic**
* **Config File Format for Remapping**

---

## 9. Conclusion

This document defines a strategy enabling customizable keybindings and optional Auto-Fire for improved accessibility. It ensures better support for players with motor impairments, aligns with accessibility guidelines, and provides a scalable architecture for future features.
