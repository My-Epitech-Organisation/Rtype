/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** Accessibility.hpp
*/

#ifndef SRC_CLIENT_GRAPHIC_ACCESSIBILITY_HPP_
#define SRC_CLIENT_GRAPHIC_ACCESSIBILITY_HPP_

#include <cstdint>

enum class ColorBlindMode : std::uint8_t {
    None = 0,
    Protanopia,
    Deuteranopia,
    Tritanopia,
    Achromatopsia,
    HighContrast
};

struct AccessibilitySettings {
    ColorBlindMode colorMode = ColorBlindMode::None;
    float intensity = 1.0f;
    bool showHitboxes = false;
    bool showVisualCues = false;
};

#endif  // SRC_CLIENT_GRAPHIC_ACCESSIBILITY_HPP_
