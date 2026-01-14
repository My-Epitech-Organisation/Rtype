/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** WeakPointComponent - Implementation of weak point utility functions
*/

#include "WeakPointComponent.hpp"

namespace rtype::games::rtype::shared {

const char* weakPointTypeToString(WeakPointType type) noexcept {
    switch (type) {
        case WeakPointType::Generic:
            return "Generic";
        case WeakPointType::Head:
            return "Head";
        case WeakPointType::Tail:
            return "Tail";
        case WeakPointType::Core:
            return "Core";
        case WeakPointType::Arm:
            return "Arm";
        case WeakPointType::Cannon:
            return "Cannon";
        case WeakPointType::Engine:
            return "Engine";
        case WeakPointType::Shield:
            return "Shield";
        default:
            return "Unknown";
    }
}

WeakPointType stringToWeakPointType(const std::string& str) noexcept {
    if (str == "head") return WeakPointType::Head;
    if (str == "tail") return WeakPointType::Tail;
    if (str == "core") return WeakPointType::Core;
    if (str == "arm") return WeakPointType::Arm;
    if (str == "cannon") return WeakPointType::Cannon;
    if (str == "engine") return WeakPointType::Engine;
    if (str == "shield") return WeakPointType::Shield;
    return WeakPointType::Generic;
}

}  // namespace rtype::games::rtype::shared
