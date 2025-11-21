/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MovementSystem
*/

#include "rtype/games/rtype/shared/Components.hpp"

namespace rtype::games::rtype::shared {

// Minimal movement system placeholder
void updateMovement(TransformComponent& transform, const VelocityComponent& velocity, float deltaTime) {
    transform.x += velocity.vx * deltaTime;
    transform.y += velocity.vy * deltaTime;
}

} // namespace rtype::games::rtype::shared
