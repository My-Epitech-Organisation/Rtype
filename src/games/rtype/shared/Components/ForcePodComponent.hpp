/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodComponent - Force Pod state and configuration data
*/

#pragma once

#include <cstdint>

namespace rtype::games::rtype::shared {

enum class ForcePodState : uint8_t { Attached = 0, Detached, Returning };

struct ForcePodComponent {
    ForcePodState state{ForcePodState::Attached};
    float offsetX{0.0F};
    float offsetY{0.0F};
    uint32_t ownerNetworkId{0};
};

}  // namespace rtype::games::rtype::shared
