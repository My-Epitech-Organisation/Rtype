/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Rect - AABB (Axis-Aligned Bounding Box) structure
*/

#ifndef AABB_RECT_HPP
    #define AABB_RECT_HPP

#include <cstdint>
#include <compare>

namespace AABB {

    /**
     * @brief Axis-Aligned Bounding Box rectangle structure.
     *
     * Represents a rectangle aligned with the coordinate axes.
     * Used for simple and efficient collision detection.
     *
     * @note Using float for coordinates to support sub-pixel positioning
     *       and smooth movement in games.
     */
    struct Rect {
        float x;      ///< X coordinate of the top-left corner
        float y;      ///< Y coordinate of the top-left corner
        float w;      ///< Width of the rectangle
        float h;      ///< Height of the rectangle

        /**
         * @brief Default constructor - creates a zero-sized rectangle at origin.
         */
        constexpr Rect() noexcept : x(0.0f), y(0.0f), w(0.0f), h(0.0f) {}

        /**
         * @brief Parameterized constructor.
         * @param x X coordinate
         * @param y Y coordinate
         * @param w Width
         * @param h Height
         */
        constexpr Rect(float x, float y, float w, float h) noexcept
            : x(x), y(y), w(w), h(h) {}

        /**
         * @brief Gets the left edge coordinate.
         * @return X coordinate of the left edge
         */
        [[nodiscard]] constexpr float left() const noexcept { return x; }

        /**
         * @brief Gets the right edge coordinate.
         * @return X coordinate of the right edge
         */
        [[nodiscard]] constexpr float right() const noexcept { return x + w; }

        /**
         * @brief Gets the top edge coordinate.
         * @return Y coordinate of the top edge
         */
        [[nodiscard]] constexpr float top() const noexcept { return y; }

        /**
         * @brief Gets the bottom edge coordinate.
         * @return Y coordinate of the bottom edge
         */
        [[nodiscard]] constexpr float bottom() const noexcept { return y + h; }

        /**
         * @brief Gets the center X coordinate.
         * @return X coordinate of the center
         */
        [[nodiscard]] constexpr float centerX() const noexcept { return x + w * 0.5f; }

        /**
         * @brief Gets the center Y coordinate.
         * @return Y coordinate of the center
         */
        [[nodiscard]] constexpr float centerY() const noexcept { return y + h * 0.5f; }

        /**
         * @brief Calculates the area of the rectangle.
         * @return Area (width * height)
         */
        [[nodiscard]] constexpr float area() const noexcept { return w * h; }

        /**
         * @brief Checks if the rectangle is valid (positive dimensions).
         * @return true if both width and height are positive
         */
        [[nodiscard]] constexpr bool isValid() const noexcept { return w > 0.0f && h > 0.0f; }

        /**
         * @brief Equality comparison.
         */
        auto operator<=>(const Rect&) const noexcept = default;
    };

} // namespace AABB

#endif // AABB_RECT_HPP
