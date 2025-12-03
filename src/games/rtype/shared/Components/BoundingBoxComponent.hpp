/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** BoundingBoxComponent - Collision detection bounds
*/

#pragma once

namespace rtype::games::rtype::shared {

/**
 * @struct BoundingBoxComponent
 * @brief Component for axis-aligned bounding box collision
 *
 * Used for collision detection between entities.
 * The box is centered on the entity's transform position.
 */
struct BoundingBoxComponent {
    float width = 32.0F;   ///< Box width in pixels
    float height = 32.0F;  ///< Box height in pixels

    /**
     * @brief Get half-width for centered collision checks
     * @return Half of the width
     */
    [[nodiscard]] float halfWidth() const noexcept { return width * 0.5F; }

    /**
     * @brief Get half-height for centered collision checks
     * @return Half of the height
     */
    [[nodiscard]] float halfHeight() const noexcept { return height * 0.5F; }
};

}  // namespace rtype::games::rtype::shared
