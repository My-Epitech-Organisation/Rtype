/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** SettingsSceneUtils.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_SETTINGSSCENE_SETTINGSSCENEUTILS_HPP_
#define SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_SETTINGSSCENE_SETTINGSSCENEUTILS_HPP_
#include <string>

#include "rtype/display/IDisplay.hpp"
#include "GameAction.hpp"

class SettingsSceneUtils {
   public:
    static std::string actionToString(GameAction action);
    static std::string keyToString(::rtype::display::Key key);
};

#endif  // SRC_CLIENT_GRAPHIC_SCENEMANAGER_SCENES_SETTINGSSCENE_SETTINGSSCENEUTILS_HPP_
