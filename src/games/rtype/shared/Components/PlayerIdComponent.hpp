/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PlayerIdComponent - Player identification component
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @struct PlayerIdComponent
 * @brief Component to store the player's ID (1-4)
 *
 * Used to identify which player controls an entity and determine
 * which sprite variation (color) to render for that player.
 *
 * Player IDs range from 1 to 4, with each ID corresponding to a specific
 * sprite color in the player_vessel sprite sheet.
 */
struct PlayerIdComponent {
    uint32_t playerId = 0;  ///< Player ID (1-4), 0 means unassigned

    /**
     * @brief Check if the player ID is valid (1-4)
     * @return true if playerId is between 1 and 4
     */
    [[nodiscard]] bool isValid() const noexcept {
        return playerId >= 1 && playerId <= 4;
    }
};

}  // namespace rtype::games::rtype::shared
