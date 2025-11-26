#include "ColorblindSimulator.hpp"
#include <algorithm>
#include <cmath>

sf::Color ColorblindSimulator::transformColor(const sf::Color& color, CVDType type) {
    if (type == CVDType::Normal) {
        return color;
    }

    // Normalize RGB to 0-1 range
    float r = color.r / 255.0f;
    float g = color.g / 255.0f;
    float b = color.b / 255.0f;

    // Apply the appropriate transformation matrix
    switch (type) {
        case CVDType::Protanopia:
            applyMatrix(r, g, b, PROTANOPIA_MATRIX);
            break;
        case CVDType::Deuteranopia:
            applyMatrix(r, g, b, DEUTERANOPIA_MATRIX);
            break;
        case CVDType::Tritanopia:
            applyMatrix(r, g, b, TRITANOPIA_MATRIX);
            break;
        default:
            break;
    }

    // Clamp and convert back to 0-255 range
    return sf::Color(
        static_cast<sf::Uint8>(std::clamp(r * 255.0f, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(std::clamp(g * 255.0f, 0.0f, 255.0f)),
        static_cast<sf::Uint8>(std::clamp(b * 255.0f, 0.0f, 255.0f)),
        color.a
    );
}

void ColorblindSimulator::applyMatrix(float& r, float& g, float& b,
                                     const std::array<std::array<float, 3>, 3>& matrix) {
    float newR = matrix[0][0] * r + matrix[0][1] * g + matrix[0][2] * b;
    float newG = matrix[1][0] * r + matrix[1][1] * g + matrix[1][2] * b;
    float newB = matrix[2][0] * r + matrix[2][1] * g + matrix[2][2] * b;

    r = newR;
    g = newG;
    b = newB;
}

const char* ColorblindSimulator::getCVDTypeName(CVDType type) {
    switch (type) {
        case CVDType::Normal:
            return "Normal Vision";
        case CVDType::Protanopia:
            return "Protanopia (Red-blind)";
        case CVDType::Deuteranopia:
            return "Deuteranopia (Green-blind)";
        case CVDType::Tritanopia:
            return "Tritanopia (Blue-blind)";
        default:
            return "Unknown";
    }
}
