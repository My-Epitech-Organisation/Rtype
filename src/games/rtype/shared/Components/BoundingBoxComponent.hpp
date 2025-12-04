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
    float width = 32.0F;   // Box width in pixels
    float height = 32.0F;  // Box height in pixels
};

}  // namespace rtype::games::rtype::shared
