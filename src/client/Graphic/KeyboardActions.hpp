/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** keyboardInput.hpp
*/

#ifndef R_TYPE_KEYBOARDINPUT_HPP
#define R_TYPE_KEYBOARDINPUT_HPP
#include <map>
#include <SFML/Window/Keyboard.hpp>
#include "GameAction.hpp"

enum class GameAction;

class KeyboardActions {
private:
    std::map<GameAction, sf::Keyboard::Key> _keyBindings;

public:
    void setKeyBinding(const GameAction &action, const sf::Keyboard::Key &key);

    GameAction getKeyBinding(const sf::Keyboard::Key &key);
    sf::Keyboard::Key getKeyBinding(const GameAction &action);

    KeyboardActions();
    ~KeyboardActions() = default;
};


#endif //R_TYPE_KEYBOARDINPUT_HPP