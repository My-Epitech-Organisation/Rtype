/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** keyboardActions.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_KEYBOARDACTIONS_HPP_
#define SRC_CLIENT_GRAPHIC_KEYBOARDACTIONS_HPP_

#include <map>
#include <optional>
#include <string>

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "../GameAction.hpp"

enum class InputMode { Keyboard, Controller };

class KeyboardActions {
   private:
    std::map<GameAction, sf::Keyboard::Key> _keyBindings;
    std::map<GameAction, unsigned int> _joyButtonBindings;
    std::map<GameAction, sf::Joystick::Axis> _joyAxisBindings;
    std::map<GameAction, bool> _joyAxisInverted;
    InputMode _inputMode = InputMode::Keyboard;

   public:
    void setInputMode(InputMode mode) { _inputMode = mode; }
    InputMode getInputMode() const { return _inputMode; }

    void setKeyBinding(const GameAction& action, const sf::Keyboard::Key& key);

    auto getKeyBinding(const sf::Keyboard::Key& key)
        -> std::optional<GameAction>;
    auto getKeyBinding(const GameAction& action)
        -> std::optional<sf::Keyboard::Key>;

    void setJoyButtonBinding(const GameAction& action, unsigned int button);
    auto getJoyButtonBinding(const GameAction& action)
        -> std::optional<unsigned int>;

    void setJoyAxisBinding(const GameAction& action,
                           const sf::Joystick::Axis& axis);
    auto getJoyAxisBinding(const GameAction& action)
        -> std::optional<sf::Joystick::Axis>;

    void setJoyAxisInverted(const GameAction& action, bool inverted);
    bool isJoyAxisInverted(const GameAction& action) const;

    static std::string getXboxButtonName(unsigned int buttonIndex);

    KeyboardActions();
    ~KeyboardActions() = default;

    KeyboardActions(const KeyboardActions&) = delete;
    KeyboardActions& operator=(const KeyboardActions&) = delete;
    KeyboardActions(KeyboardActions&&) = delete;
    KeyboardActions& operator=(KeyboardActions&&) = delete;
};

#endif  // SRC_CLIENT_GRAPHIC_KEYBOARDACTIONS_HPP_
