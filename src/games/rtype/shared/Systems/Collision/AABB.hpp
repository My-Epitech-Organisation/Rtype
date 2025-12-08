/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AABB - Axis-aligned bounding box helpers
*/

#pragma once

#include "../../Components/BoundingBoxComponent.hpp"
#include "../../Components/TransformComponent.hpp"

namespace rtype::games::rtype::shared::collision {

inline bool overlaps(const TransformComponent& aTransform,
                     const BoundingBoxComponent& aBox,
                     const TransformComponent& bTransform,
                     const BoundingBoxComponent& bBox) {
    const float aHalfW = aBox.width * 0.5F;
    const float aHalfH = aBox.height * 0.5F;
    const float bHalfW = bBox.width * 0.5F;
    const float bHalfH = bBox.height * 0.5F;

    const float aLeft = aTransform.x - aHalfW;
    const float aRight = aTransform.x + aHalfW;
    const float aTop = aTransform.y - aHalfH;
    const float aBottom = aTransform.y + aHalfH;

    const float bLeft = bTransform.x - bHalfW;
    const float bRight = bTransform.x + bHalfW;
    const float bTop = bTransform.y - bHalfH;
    const float bBottom = bTransform.y + bHalfH;

    const bool separated = aRight < bLeft || bRight < aLeft || aBottom < bTop ||
                           bBottom < aTop;
    return !separated;
}

}  // namespace rtype::games::rtype::shared::collision
