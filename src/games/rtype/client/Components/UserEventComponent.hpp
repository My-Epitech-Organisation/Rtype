/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** UserActionComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_USEREVENTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_USEREVENTCOMPONENT_HPP_

namespace rtype::games::rtype::client {

struct UserEvent {
    bool idle = true;
    bool isHovered = false;
    bool isClicked = false;
    bool isReleased = false;
};
}  // namespace rtype::games::rtype::client
#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_USEREVENTCOMPONENT_HPP_
