/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChaseBehavior - Implementation
*/

#include "ChaseBehavior.hpp"

#include <cmath>

namespace rtype::games::rtype::shared {

void ChaseBehavior::apply(AIComponent& ai, const TransformComponent& transform,
                          VelocityComponent& velocity, float /*deltaTime*/) {
    float dx = ai.targetX - transform.x;
    float dy = ai.targetY - transform.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist > _stopDistance) {
        velocity.vx = (dx / dist) * ai.speed;
        velocity.vy = (dy / dist) * ai.speed;
    } else {
        velocity.vx = 0.0F;
        velocity.vy = 0.0F;
    }
}

}  // namespace rtype::games::rtype::shared
