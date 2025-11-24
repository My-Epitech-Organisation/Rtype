/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Collision - AABB collision detection functions
*/

#ifndef AABB_COLLISION_HPP
    #define AABB_COLLISION_HPP

#include "Rect.hpp"
#include <algorithm>
#include <optional>

namespace AABB {

    /**
     * @brief Checks if two AABB rectangles overlap (collision detection).
     *
     * Uses the Separating Axis Theorem (SAT) for axis-aligned boxes.
     * Two AABBs collide if they overlap on BOTH X and Y axes.
     *
     * Algorithm:
     * - Check if rectangles are separated on X axis
     * - Check if rectangles are separated on Y axis
     * - If separated on either axis, no collision
     * - Otherwise, collision detected
     *
     * Time Complexity: O(1) - Constant time
     * Space Complexity: O(1) - No allocations
     *
     * @param a First rectangle
     * @param b Second rectangle
     * @return true if rectangles overlap, false otherwise
     *
     * @note Edge touching is considered a collision (a.right() == b.left() returns true)
     * @note Invalid rectangles (negative dimensions) may produce incorrect results
     */
    [[nodiscard]] constexpr bool checkCollision(const Rect& a, const Rect& b) noexcept {
        // Check if separated on X axis
        // a is completely to the left of b, or b is completely to the left of a
        if (a.right() < b.left() || b.right() < a.left()) {
            return false;
        }

        // Check if separated on Y axis
        // a is completely above b, or b is completely above a
        if (a.bottom() < b.top() || b.bottom() < a.top()) {
            return false;
        }

        // Not separated on any axis = collision
        return true;
    }

    /**
     * @brief Checks if two AABB rectangles overlap with strict inequality (no edge touching).
     *
     * Similar to checkCollision, but edge touching is NOT considered a collision.
     *
     * @param a First rectangle
     * @param b Second rectangle
     * @return true if rectangles overlap (excluding edges), false otherwise
     */
    [[nodiscard]] constexpr bool checkCollisionStrict(const Rect& a, const Rect& b) noexcept {
        // Check if separated or just touching on X axis
        if (a.right() <= b.left() || b.right() <= a.left()) {
            return false;
        }

        // Check if separated or just touching on Y axis
        if (a.bottom() <= b.top() || b.bottom() <= a.top()) {
            return false;
        }

        return true;
    }

    /**
     * @brief Checks if point is inside rectangle.
     *
     * @param rect Rectangle to test
     * @param px Point X coordinate
     * @param py Point Y coordinate
     * @return true if point is inside or on the edge of the rectangle
     */
    [[nodiscard]] constexpr bool containsPoint(const Rect& rect, float px, float py) noexcept {
        return px >= rect.left() && px <= rect.right() &&
               py >= rect.top() && py <= rect.bottom();
    }

    /**
     * @brief Checks if one rectangle completely contains another.
     *
     * @param outer Potentially containing rectangle
     * @param inner Potentially contained rectangle
     * @return true if outer completely contains inner
     */
    [[nodiscard]] constexpr bool contains(const Rect& outer, const Rect& inner) noexcept {
        return inner.left() >= outer.left() &&
               inner.right() <= outer.right() &&
               inner.top() >= outer.top() &&
               inner.bottom() <= outer.bottom();
    }

    /**
     * @brief Computes the intersection rectangle of two overlapping AABBs.
     *
     * @param a First rectangle
     * @param b Second rectangle
     * @return Intersection rectangle if they overlap, std::nullopt otherwise
     */
    [[nodiscard]] constexpr std::optional<Rect> intersection(const Rect& a, const Rect& b) noexcept {
        if (!checkCollision(a, b)) {
            return std::nullopt;
        }

        float left = std::max(a.left(), b.left());
        float top = std::max(a.top(), b.top());
        float right = std::min(a.right(), b.right());
        float bottom = std::min(a.bottom(), b.bottom());

        return Rect{left, top, right - left, bottom - top};
    }

    /**
     * @brief Computes the union (bounding box) of two rectangles.
     *
     * @param a First rectangle
     * @param b Second rectangle
     * @return Rectangle that encompasses both input rectangles
     */
    [[nodiscard]] constexpr Rect unionBounds(const Rect& a, const Rect& b) noexcept {
        float left = std::min(a.left(), b.left());
        float top = std::min(a.top(), b.top());
        float right = std::max(a.right(), b.right());
        float bottom = std::max(a.bottom(), b.bottom());

        return Rect{left, top, right - left, bottom - top};
    }

    /**
     * @brief Calculates overlap depth on each axis.
     *
     * Useful for collision response - tells you how much to move objects
     * to resolve the collision.
     *
     * @param a First rectangle
     * @param b Second rectangle
     * @param[out] overlapX Overlap distance on X axis (negative if separated)
     * @param[out] overlapY Overlap distance on Y axis (negative if separated)
     * @return true if rectangles overlap, false otherwise
     */
    [[nodiscard]] constexpr bool getOverlapDepth(const Rect& a, const Rect& b,
                                                   float& overlapX, float& overlapY) noexcept {
        // Calculate overlap on X axis
        float leftOverlap = a.right() - b.left();
        float rightOverlap = b.right() - a.left();
        overlapX = std::min(leftOverlap, rightOverlap);

        // Calculate overlap on Y axis
        float topOverlap = a.bottom() - b.top();
        float bottomOverlap = b.bottom() - a.top();
        overlapY = std::min(topOverlap, bottomOverlap);

        // If either overlap is negative or zero, no collision
        return overlapX > 0.0f && overlapY > 0.0f;
    }

} // namespace AABB

#endif // AABB_COLLISION_HPP
