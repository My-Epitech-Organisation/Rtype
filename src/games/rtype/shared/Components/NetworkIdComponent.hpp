/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NetworkIdComponent - Network synchronization identifier
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @struct NetworkIdComponent
 * @brief Component for network entity identification
 *
 * Used to synchronize entities between server and clients.
 * Each networked entity has a unique network ID assigned by the server.
 */
struct NetworkIdComponent {
    uint32_t networkId = 0;  ///< Unique network identifier
};

}  // namespace rtype::games::rtype::shared
