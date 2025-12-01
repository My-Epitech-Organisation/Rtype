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

#include <SFML/Window/Keyboard.hpp>

#include "../GameAction.hpp"

class KeyboardActions {
   private:
    std::map<GameAction, sf::Keyboard::Key> _keyBindings;

   public:
    void setKeyBinding(const GameAction& action, const sf::Keyboard::Key& key);

    auto getKeyBinding(const sf::Keyboard::Key& key)
        -> std::optional<GameAction>;
    auto getKeyBinding(const GameAction& action)
        -> std::optional<sf::Keyboard::Key>;

    KeyboardActions();
    ~KeyboardActions() = default;

    KeyboardActions(const KeyboardActions&) = delete;
    KeyboardActions& operator=(const KeyboardActions&) = delete;
    KeyboardActions(KeyboardActions&&) = delete;
    KeyboardActions& operator=(KeyboardActions&&) = delete;
};

#endif  // SRC_CLIENT_GRAPHIC_KEYBOARDACTIONS_HPP_
