/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** CountdownComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_COUNTDOWNCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_COUNTDOWNCOMPONENT_HPP_

namespace rtype::games::rtype::client {
/**
 * @struct CountdownPlayer
 * @brief Component for managing player laser shooting cooldown.
 *
 * This component tracks the remaining cooldown time before the player can shoot
 * another laser.
 */
struct CountdownPlayer {
    /**
     * @brief Remaining cooldown time in seconds before the player can shoot
     * again.
     */
    float laserCD;
};
};  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_COUNTDOWNCOMPONENT_HPP_
