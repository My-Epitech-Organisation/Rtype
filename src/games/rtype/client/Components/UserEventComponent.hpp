/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** UserActionComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_USEREVENTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_USEREVENTCOMPONENT_HPP_

struct UserEvent {
    bool idle = true;
    bool isHovered = false;
    bool isClicked = false;
    bool isReleased = false;
};

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_USEREVENTCOMPONENT_HPP_
