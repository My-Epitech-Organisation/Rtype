/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MovementSystem
*/

#include "MovementSystem.hpp"

namespace rtype::games::rtype::shared {

// Minimal movement system placeholder
TransformComponent updateMovement(TransformComponent transform,
                    const VelocityComponent& velocity, float deltaTime) {
    transform.x += velocity.vx * deltaTime;
    transform.y += velocity.vy * deltaTime;
    return transform;
}

}  // namespace rtype::games::rtype::shared
