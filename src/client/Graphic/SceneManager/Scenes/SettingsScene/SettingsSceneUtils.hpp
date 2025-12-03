/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SettingsSceneUtils.hpp
*/

#ifndef R_TYPE_SETTINGSSCENEUTILS_HPP
#define R_TYPE_SETTINGSSCENEUTILS_HPP
#include <string>

#include <SFML/Window/Keyboard.hpp>

#include "GameAction.hpp"


class SettingsSceneUtils {
public:
    static std::string actionToString(GameAction action);
    static std::string keyToString(sf::Keyboard::Key key);
};


#endif //R_TYPE_SETTINGSSCENEUTILS_HPP