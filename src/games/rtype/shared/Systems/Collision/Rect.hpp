/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Rect - Axis-Aligned Bounding Box structure for QuadTree
*/

#pragma once

#include <compare>

namespace rtype::games::rtype::shared::collision {

/**
 * @struct Rect
 * @brief Axis-Aligned Bounding Box rectangle structure.
 *
 * Represents a rectangle aligned with the coordinate axes.
 * Used for spatial partitioning and collision detection in QuadTree.
 */
struct Rect {
    float x;  ///< X coordinate of the top-left corner
    float y;  ///< Y coordinate of the top-left corner
    float w;  ///< Width of the rectangle
    float h;  ///< Height of the rectangle

    /**
     * @brief Default constructor - creates a zero-sized rectangle at origin.
     */
    constexpr Rect() noexcept : x(0.0F), y(0.0F), w(0.0F), h(0.0F) {}

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
    [[nodiscard]] constexpr float centerX() const noexcept {
        return x + w * 0.5F;
    }

    /**
     * @brief Gets the center Y coordinate.
     * @return Y coordinate of the center
     */
    [[nodiscard]] constexpr float centerY() const noexcept {
        return y + h * 0.5F;
    }

    /**
     * @brief Calculates the area of the rectangle.
     * @return Area (width * height)
     */
    [[nodiscard]] constexpr float area() const noexcept { return w * h; }

    /**
     * @brief Checks if the rectangle is valid (positive dimensions).
     * @return true if both width and height are positive
     */
    [[nodiscard]] constexpr bool isValid() const noexcept {
        return w > 0.0F && h > 0.0F;
    }

    /**
     * @brief Checks if this rectangle intersects with another.
     *
     * Note: Rectangles that touch exactly at an edge or corner are considered
     * intersecting (uses strict inequality <, not <=). This is intentional
     * for collision detection where shared boundaries should trigger checks.
     *
     * @param other The other rectangle
     * @return true if rectangles overlap or touch at edges
     */
    [[nodiscard]] constexpr bool intersects(const Rect& other) const noexcept {
        return !(right() < other.left() || other.right() < left() ||
                 bottom() < other.top() || other.bottom() < top());
    }

    /**
     * @brief Checks if this rectangle contains another.
     * @param other The other rectangle
     * @return true if this rectangle fully contains the other
     */
    [[nodiscard]] constexpr bool contains(const Rect& other) const noexcept {
        return other.left() >= left() && other.right() <= right() &&
               other.top() >= top() && other.bottom() <= bottom();
    }

    /**
     * @brief Checks if point is inside rectangle.
     * @param px Point X coordinate
     * @param py Point Y coordinate
     * @return true if point is inside or on the edge of the rectangle
     */
    [[nodiscard]] constexpr bool containsPoint(float px,
                                               float py) const noexcept {
        return px >= left() && px <= right() && py >= top() && py <= bottom();
    }

    /**
     * @brief Equality comparison operator.
     */
    auto operator<=>(const Rect&) const noexcept = default;
};

}  // namespace rtype::games::rtype::shared::collision
