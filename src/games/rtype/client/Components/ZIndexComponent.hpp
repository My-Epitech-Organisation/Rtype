/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ZIndexComponent.hpp - Component for render ordering
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_ZINDEXCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_ZINDEXCOMPONENT_HPP_

namespace rtype::games::rtype::client {

/**
 * @brief Component controlling render order (depth) of entities.
 *
 * Entities with lower depth values are rendered first (behind).
 * Entities with higher depth values are rendered last (in front).
 *
 * Typical depth values:
 * - Background: -10 to -1
 * - Game entities: 0
 * - UI elements: 1 to 10
 * - Overlays: 10+
 */
struct ZIndex {
    int depth = 0;  ///< Render depth (lower = behind, higher = front)
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_ZINDEXCOMPONENT_HPP_
