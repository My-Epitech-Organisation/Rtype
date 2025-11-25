#include "InputManager.hpp"
#include <iostream>

InputManager::InputManager() {
    // Initialize with default WASD controls as specified in the doc
    bindings_[Action::MoveUp] = KeyCode::KeyW;
    bindings_[Action::MoveLeft] = KeyCode::KeyA;
    bindings_[Action::MoveDown] = KeyCode::KeyS;
    bindings_[Action::MoveRight] = KeyCode::KeyD;
    bindings_[Action::Fire] = KeyCode::Space;
}

bool InputManager::bindKey(Action action, KeyCode key) {
    // Validate: avoid duplicate keys (as per doc requirements)
    if (isKeyAlreadyBound(key, action)) {
        std::cerr << "Key already bound to another action!" << std::endl;
        return false;
    }

    bindings_[action] = key;
    std::cout << "Bound " << actionToString(action)
              << " to " << keyCodeToString(key) << std::endl;
    return true;
}

KeyCode InputManager::getKeyForAction(Action action) const {
    auto it = bindings_.find(action);
    if (it != bindings_.end()) {
        return it->second;
    }
    return KeyCode::Unknown;
}

bool InputManager::isActionPressed(Action action, KeyCode pressedKey) const {
    auto it = bindings_.find(action);
    return (it != bindings_.end() && it->second == pressedKey);
}

const std::unordered_map<Action, KeyCode>& InputManager::getBindings() const {
    return bindings_;
}

void InputManager::loadBindings(const std::unordered_map<Action, KeyCode>& bindings) {
    bindings_ = bindings;
}

bool InputManager::isKeyAlreadyBound(KeyCode key, Action excludeAction) const {
    for (const auto& [action, boundKey] : bindings_) {
        if (action != excludeAction && boundKey == key) {
            return true;
        }
    }
    return false;
}

// Static conversion functions
KeyCode InputManager::stringToKeyCode(const std::string& keyName) {
    if (keyName == "KeyW") return KeyCode::KeyW;
    if (keyName == "KeyA") return KeyCode::KeyA;
    if (keyName == "KeyS") return KeyCode::KeyS;
    if (keyName == "KeyD") return KeyCode::KeyD;
    if (keyName == "Space") return KeyCode::Space;
    if (keyName == "ArrowUp") return KeyCode::ArrowUp;
    if (keyName == "ArrowDown") return KeyCode::ArrowDown;
    if (keyName == "ArrowLeft") return KeyCode::ArrowLeft;
    if (keyName == "ArrowRight") return KeyCode::ArrowRight;
    return KeyCode::Unknown;
}

std::string InputManager::keyCodeToString(KeyCode key) {
    switch (key) {
        case KeyCode::KeyW: return "KeyW";
        case KeyCode::KeyA: return "KeyA";
        case KeyCode::KeyS: return "KeyS";
        case KeyCode::KeyD: return "KeyD";
        case KeyCode::Space: return "Space";
        case KeyCode::ArrowUp: return "ArrowUp";
        case KeyCode::ArrowDown: return "ArrowDown";
        case KeyCode::ArrowLeft: return "ArrowLeft";
        case KeyCode::ArrowRight: return "ArrowRight";
        default: return "Unknown";
    }
}

Action InputManager::stringToAction(const std::string& actionName) {
    if (actionName == "move_up") return Action::MoveUp;
    if (actionName == "move_down") return Action::MoveDown;
    if (actionName == "move_left") return Action::MoveLeft;
    if (actionName == "move_right") return Action::MoveRight;
    if (actionName == "fire") return Action::Fire;
    return Action::MoveUp; // default
}

std::string InputManager::actionToString(Action action) {
    switch (action) {
        case Action::MoveUp: return "move_up";
        case Action::MoveDown: return "move_down";
        case Action::MoveLeft: return "move_left";
        case Action::MoveRight: return "move_right";
        case Action::Fire: return "fire";
        default: return "unknown";
    }
}
