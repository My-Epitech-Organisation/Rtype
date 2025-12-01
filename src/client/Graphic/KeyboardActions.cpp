/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** keyboardInput.cpp
*/

#include "KeyboardActions.hpp"

KeyboardActions::KeyboardActions() {
    this->_keyBindings.emplace(GameAction::MOVE_UP, sf::Keyboard::Key::Up);
    this->_keyBindings.emplace(GameAction::MOVE_DOWN, sf::Keyboard::Key::Down);
    this->_keyBindings.emplace(GameAction::MOVE_RIGHT, sf::Keyboard::Key::Right);
    this->_keyBindings.emplace(GameAction::MOVE_LEFT, sf::Keyboard::Key::Left);
    this->_keyBindings.emplace(GameAction::SHOOT, sf::Keyboard::Key::Space);
    this->_keyBindings.emplace(GameAction::PAUSE, sf::Keyboard::Key::Escape);
}

sf::Keyboard::Key KeyboardActions::getKeyBinding(const GameAction &action)  {
    return this->_keyBindings[action];
}

GameAction KeyboardActions::getKeyBinding(const sf::Keyboard::Key &key)  {
    for (const auto &[fst, snd] : this->_keyBindings)
        if (snd == key)
            return fst;
    return GameAction::NONE;
}

void KeyboardActions::setKeyBinding(const GameAction &action, const sf::Keyboard::Key &key)  {
    this->_keyBindings[action] = key;
}
