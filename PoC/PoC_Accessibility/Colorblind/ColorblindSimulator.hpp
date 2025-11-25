#pragma once

#include <SFML/Graphics.hpp>
#include <array>

/**
 * @brief Types of color vision deficiency (CVD)
 */
enum class CVDType {
    Normal,          // No colorblindness
    Protanopia,      // Red-blind (1% of males)
    Deuteranopia,    // Green-blind (1% of males)
    Tritanopia,      // Blue-blind (0.001% of population)
};

/**
 * @brief Simulates colorblind vision by applying transformation matrices
 *
 * This class applies mathematical transformations to RGB colors to simulate
 * how they would appear to people with different types of color vision deficiency.
 */
class ColorblindSimulator {
public:
    /**
     * @brief Transform a color based on the selected CVD type
     * @param color Original color
     * @param type Type of color vision deficiency
     * @return Transformed color as seen by someone with the specified CVD
     */
    static sf::Color transformColor(const sf::Color& color, CVDType type);

    /**
     * @brief Get a human-readable name for the CVD type
     */
    static const char* getCVDTypeName(CVDType type);

private:
    /**
     * @brief Apply a transformation matrix to an RGB color
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * @param matrix 3x3 transformation matrix
     */
    static void applyMatrix(float& r, float& g, float& b,
                          const std::array<std::array<float, 3>, 3>& matrix);

    // Transformation matrices for different CVD types
    // Based on Brettel, Viénot and Mollon JPEG algorithm
    static constexpr std::array<std::array<float, 3>, 3> PROTANOPIA_MATRIX = {{
        {{0.567f, 0.433f, 0.0f}},
        {{0.558f, 0.442f, 0.0f}},
        {{0.0f, 0.242f, 0.758f}}
    }};

    static constexpr std::array<std::array<float, 3>, 3> DEUTERANOPIA_MATRIX = {{
        {{0.625f, 0.375f, 0.0f}},
        {{0.7f, 0.3f, 0.0f}},
        {{0.0f, 0.3f, 0.7f}}
    }};

    static constexpr std::array<std::array<float, 3>, 3> TRITANOPIA_MATRIX = {{
        {{0.95f, 0.05f, 0.0f}},
        {{0.0f, 0.433f, 0.567f}},
        {{0.0f, 0.475f, 0.525f}}
    }};
};
