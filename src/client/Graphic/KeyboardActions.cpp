/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** keyboardInput.cpp
*/
#include "KeyboardActions.hpp"

#include <optional>

KeyboardActions::KeyboardActions() {
    this->_keyBindings.emplace(GameAction::MOVE_UP, sf::Keyboard::Key::Up);
    this->_keyBindings.emplace(GameAction::MOVE_DOWN, sf::Keyboard::Key::Down);
    this->_keyBindings.emplace(GameAction::MOVE_RIGHT,
                               sf::Keyboard::Key::Right);
    this->_keyBindings.emplace(GameAction::MOVE_LEFT, sf::Keyboard::Key::Left);
    this->_keyBindings.emplace(GameAction::SHOOT, sf::Keyboard::Key::Space);
    this->_keyBindings.emplace(GameAction::PAUSE, sf::Keyboard::Key::Escape);
}

auto KeyboardActions::getKeyBinding(const GameAction& action)
    -> std::optional<sf::Keyboard::Key> {
    if (!this->_keyBindings.contains(action)) return {};
    return this->_keyBindings[action];
}

auto KeyboardActions::getKeyBinding(const sf::Keyboard::Key& key)
    -> std::optional<GameAction> {
    for (const auto& [fst, snd] : this->_keyBindings) {
        if (snd == key) {
            return fst;
        }
    }
    return {};
}

void KeyboardActions::setKeyBinding(const GameAction& action,
                                    const sf::Keyboard::Key& key) {
    this->_keyBindings[action] = key;
}
