/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodComponent - Force Pod state and configuration data
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

/**
 * @brief Force Pod state enumeration
 * - Attached: Pod follows owner player
 * - Detached: Pod launched and moving independently
 * - Returning: Pod returning to owner player
 * - Orphan: Owner player died, pod available for pickup by any player
 */
enum class ForcePodState : uint8_t {
    Attached = 0,
    Detached,
    Returning,
    Orphan
};

struct ForcePodComponent {
    ForcePodState state{ForcePodState::Attached};
    float offsetX{0.0F};
    float offsetY{0.0F};
    uint32_t ownerNetworkId{0};

    /**
     * @brief Make this pod orphan (owner died)
     * Resets owner and sets state to Orphan
     */
    void makeOrphan() {
        state = ForcePodState::Orphan;
        ownerNetworkId = 0;
    }

    /**
     * @brief Adopt this pod (new owner picks it up)
     * @param newOwnerNetworkId Network ID of the new owner
     */
    void adopt(uint32_t newOwnerNetworkId) {
        state = ForcePodState::Attached;
        ownerNetworkId = newOwnerNetworkId;
    }
};

}  // namespace rtype::games::rtype::shared
