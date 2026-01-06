/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** keyboardInput.cpp
*/
#include "KeyboardActions.hpp"

#include <optional>
#include <string>

#include "Logger/Macros.hpp"

KeyboardActions::KeyboardActions() {
    this->_keyBindings.emplace(GameAction::MOVE_UP, ::rtype::display::Key::Up);
    this->_keyBindings.emplace(GameAction::MOVE_DOWN, ::rtype::display::Key::Down);
    this->_keyBindings.emplace(GameAction::MOVE_RIGHT,
                               ::rtype::display::Key::Right);
    this->_keyBindings.emplace(GameAction::MOVE_LEFT, ::rtype::display::Key::Left);
    this->_keyBindings.emplace(GameAction::SHOOT, ::rtype::display::Key::Space);
    this->_keyBindings.emplace(GameAction::PAUSE, ::rtype::display::Key::Escape);
    this->_keyBindings.emplace(GameAction::CHANGE_AMMO, ::rtype::display::Key::Tab);

    this->_joyAxisBindings.emplace(GameAction::MOVE_UP, ::rtype::display::JoystickAxis::Y);
    this->_joyAxisBindings.emplace(GameAction::MOVE_DOWN,
                                   ::rtype::display::JoystickAxis::Y);
    this->_joyAxisBindings.emplace(GameAction::MOVE_LEFT,
                                   ::rtype::display::JoystickAxis::X);
    this->_joyAxisBindings.emplace(GameAction::MOVE_RIGHT,
                                   ::rtype::display::JoystickAxis::X);
    this->_joyButtonBindings.emplace(GameAction::SHOOT, 0);
    this->_joyButtonBindings.emplace(GameAction::PAUSE, 7);
    this->_joyButtonBindings.emplace(GameAction::CHANGE_AMMO, 2);
}

void KeyboardActions::initialize(::rtype::display::IDisplay& display) {
    bool controllerFound = false;
    unsigned int count = display.getJoystickCount();
    for (unsigned int i = 0; i < count; ++i) {
        if (display.isJoystickConnected(i)) {
            controllerFound = true;
            LOG_INFO("[KeyboardActions] Controller detected (Joystick " +
                     std::to_string(i) + ") - defaulting to Controller mode");
            this->_inputMode = InputMode::Controller;
            break;
        }
    }

    if (!controllerFound) {
        LOG_INFO(
            "[KeyboardActions] No controller detected - defaulting to Keyboard "
            "mode");
        this->_inputMode = InputMode::Keyboard;
    }
}

auto KeyboardActions::getKeyBinding(const GameAction& action)
    -> std::optional<::rtype::display::Key> {
    if (!this->_keyBindings.contains(action)) return {};
    return this->_keyBindings[action];
}

auto KeyboardActions::getKeyBinding(const ::rtype::display::Key& key)
    -> std::optional<GameAction> {
    for (const auto& [fst, snd] : this->_keyBindings) {
        if (snd == key) {
            return fst;
        }
    }
    return {};
}

void KeyboardActions::setKeyBinding(const GameAction& action,
                                    const ::rtype::display::Key& key) {
    this->_keyBindings[action] = key;
}

void KeyboardActions::setJoyButtonBinding(const GameAction& action,
                                          unsigned int button) {
    this->_joyButtonBindings[action] = button;
}

auto KeyboardActions::getJoyButtonBinding(const GameAction& action)
    -> std::optional<unsigned int> {
    if (!this->_joyButtonBindings.contains(action)) return {};
    return this->_joyButtonBindings[action];
}

void KeyboardActions::setJoyAxisBinding(const GameAction& action,
                                        const ::rtype::display::JoystickAxis& axis) {
    this->_joyAxisBindings[action] = axis;
}

auto KeyboardActions::getJoyAxisBinding(const GameAction& action)
    -> std::optional<::rtype::display::JoystickAxis> {
    if (!this->_joyAxisBindings.contains(action)) return {};
    return this->_joyAxisBindings[action];
}

void KeyboardActions::setJoyAxisInverted(const GameAction& action,
                                         bool inverted) {
    this->_joyAxisInverted[action] = inverted;
}

bool KeyboardActions::isJoyAxisInverted(const GameAction& action) const {
    if (!this->_joyAxisInverted.contains(action)) return false;
    return this->_joyAxisInverted.at(action);
}

std::string KeyboardActions::getXboxButtonName(unsigned int buttonIndex) {
    switch (buttonIndex) {
        case 0:
            return "A";
        case 1:
            return "B";
        case 2:
            return "X";
        case 3:
            return "Y";
        case 4:
            return "LB";
        case 5:
            return "RB";
        case 6:
            return "Back";
        case 7:
            return "Start";
        case 8:
            return "LS";
        case 9:
            return "RS";
        default:
            return "Button " + std::to_string(buttonIndex);
    }
}
