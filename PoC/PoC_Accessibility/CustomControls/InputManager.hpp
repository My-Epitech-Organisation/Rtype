#pragma once
#include <string>
#include <unordered_map>
#include <functional>

// Simulated key codes for the PoC
enum class KeyCode {
    KeyW,
    KeyA,
    KeyS,
    KeyD,
    Space,
    ArrowUp,
    ArrowDown,
    ArrowLeft,
    ArrowRight,
    Unknown
};

// Action types that can be bound to keys
enum class Action {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Fire
};

/**
 * @brief Central component storing active keybindings
 * 
 * This class manages the mapping between actions and key codes,
 * allowing for custom input remapping as described in the accessibility doc.
 */
class InputManager {
public:
    InputManager();
    
    /**
     * @brief Bind an action to a key code
     * @param action The action to bind
     * @param key The key code to bind to the action
     * @return true if binding was successful, false if duplicate key detected
     */
    bool bindKey(Action action, KeyCode key);
    
    /**
     * @brief Get the key bound to an action
     * @param action The action to query
     * @return The key code bound to the action
     */
    KeyCode getKeyForAction(Action action) const;
    
    /**
     * @brief Check if a key is pressed for a specific action
     * @param action The action to check
     * @param pressedKey The key that was pressed
     * @return true if the pressed key matches the action's binding
     */
    bool isActionPressed(Action action, KeyCode pressedKey) const;
    
    /**
     * @brief Get all current bindings
     * @return Map of actions to key codes
     */
    const std::unordered_map<Action, KeyCode>& getBindings() const;
    
    /**
     * @brief Load bindings from a map (used by config serializer)
     * @param bindings The bindings to load
     */
    void loadBindings(const std::unordered_map<Action, KeyCode>& bindings);
    
    /**
     * @brief Convert string to KeyCode
     * @param keyName The string representation of the key
     * @return The corresponding KeyCode
     */
    static KeyCode stringToKeyCode(const std::string& keyName);
    
    /**
     * @brief Convert KeyCode to string
     * @param key The key code
     * @return The string representation
     */
    static std::string keyCodeToString(KeyCode key);
    
    /**
     * @brief Convert string to Action
     * @param actionName The string representation of the action
     * @return The corresponding Action
     */
    static Action stringToAction(const std::string& actionName);
    
    /**
     * @brief Convert Action to string
     * @param action The action
     * @return The string representation
     */
    static std::string actionToString(Action action);

private:
    std::unordered_map<Action, KeyCode> bindings_;
    
    /**
     * @brief Check if a key is already bound to another action
     * @param key The key to check
     * @param excludeAction Action to exclude from the check
     * @return true if key is already bound
     */
    bool isKeyAlreadyBound(KeyCode key, Action excludeAction) const;
};
