/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** NetworkIdComponent - Network synchronization identifier
*/

#pragma once

#include <cstdint>
#include <limits>

namespace rtype::games::rtype::shared {

/**
 * @brief Sentinel value indicating an uninitialized network ID
 */
inline constexpr uint32_t INVALID_NETWORK_ID =
    std::numeric_limits<uint32_t>::max();

/**
 * @struct NetworkIdComponent
 * @brief Component for network entity identification
 *
 * Used to synchronize entities between server and clients.
 * Each networked entity has a unique network ID assigned by the server.
 */
struct NetworkIdComponent {
    uint32_t networkId = INVALID_NETWORK_ID;

    /**
     * @brief Check if the network ID has been initialized
     * @return true if the ID is valid, false if uninitialized
     */
    [[nodiscard]] bool isValid() const noexcept {
        return networkId != INVALID_NETWORK_ID;
    }
};

}  // namespace rtype::games::rtype::shared
