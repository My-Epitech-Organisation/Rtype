/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** UserEventComponent.hpp - Component for tracking user interaction states
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_USEREVENTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_USEREVENTCOMPONENT_HPP_

namespace rtype::games::rtype::client {

/**
 * @brief Component tracking user interaction states for UI elements.
 *
 * Used by EventSystem to track mouse interactions with entities.
 * Typically combined with Rectangle and Button components.
 *
 * State transitions:
 * - idle -> isHovered (mouse enters)
 * - isHovered -> isClicked (mouse pressed)
 * - isClicked -> isReleased (mouse released while inside)
 * - isReleased triggers button callbacks, then resets to idle
 */
struct UserEvent {
    bool idle = true;         ///< No interaction occurring
    bool isHovered = false;   ///< Mouse is over the element
    bool isClicked = false;   ///< Mouse button is pressed on element
    bool isReleased = false;  ///< Mouse button was released (triggers action)
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_USEREVENTCOMPONENT_HPP_
