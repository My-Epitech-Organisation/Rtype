/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** keyboardActions.hpp
*/

#ifndef R_TYPE_KEYBOARDACTION_HPP
#define R_TYPE_KEYBOARDACTION_HPP

#include <map>
#include <optional>
#include <SFML/Window/Keyboard.hpp>
#include "GameAction.hpp"

class KeyboardActions {
private:
    std::map<GameAction, sf::Keyboard::Key> _keyBindings;

public:
    void setKeyBinding(const GameAction &action, const sf::Keyboard::Key &key);

    std::optional<GameAction> getKeyBinding(const sf::Keyboard::Key &key);
    std::optional<sf::Keyboard::Key> getKeyBinding(const GameAction &action);

    KeyboardActions();
    ~KeyboardActions() = default;
};


#endif //R_TYPE_KEYBOARDACTION_HPP