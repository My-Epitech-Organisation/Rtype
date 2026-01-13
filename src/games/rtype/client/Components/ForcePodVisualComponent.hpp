/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ForcePodVisualComponent - Visual state for Force Pod rendering
*/

#pragma once

#include <SFML/Graphics.hpp>

namespace rtype::games::rtype::client {

struct ForcePodVisual {
    float glowIntensity{0.0F};
    float rotationAngle{0.0F};
    bool showTrail{false};
    sf::Color tintColor{255, 255, 255, 255};
};

}  // namespace rtype::games::rtype::client
