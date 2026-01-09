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

#include "../../../include/rtype/display/IDisplay.hpp"
#include "../GameAction.hpp"

enum class InputMode { Keyboard, Controller };

class KeyboardActions {
   private:
    std::map<GameAction, ::rtype::display::Key> _keyBindings;
    std::map<GameAction, unsigned int> _joyButtonBindings;
    std::map<GameAction, ::rtype::display::JoystickAxis> _joyAxisBindings;
    std::map<GameAction, bool> _joyAxisInverted;
    InputMode _inputMode = InputMode::Keyboard;

   public:
    KeyboardActions();
    void initialize(::rtype::display::IDisplay& display);

    void setInputMode(InputMode mode) { _inputMode = mode; }
    InputMode getInputMode() const { return _inputMode; }

    void setKeyBinding(const GameAction& action,
                       const ::rtype::display::Key& key);

    auto getKeyBinding(const ::rtype::display::Key& key)
        -> std::optional<GameAction>;
    auto getKeyBinding(const GameAction& action)
        -> std::optional<::rtype::display::Key>;

    void setJoyButtonBinding(const GameAction& action, unsigned int button);
    auto getJoyButtonBinding(const GameAction& action)
        -> std::optional<unsigned int>;

    void setJoyAxisBinding(const GameAction& action,
                           const ::rtype::display::JoystickAxis& axis);
    auto getJoyAxisBinding(const GameAction& action)
        -> std::optional<::rtype::display::JoystickAxis>;

    void setJoyAxisInverted(const GameAction& action, bool inverted);
    bool isJoyAxisInverted(const GameAction& action) const;

    static std::string getXboxButtonName(unsigned int buttonIndex);

    ~KeyboardActions() = default;

    KeyboardActions(const KeyboardActions&) = delete;
    KeyboardActions& operator=(const KeyboardActions&) = delete;
    KeyboardActions(KeyboardActions&&) = delete;
    KeyboardActions& operator=(KeyboardActions&&) = delete;
};

#endif  // SRC_CLIENT_GRAPHIC_KEYBOARDACTIONS_HPP_
