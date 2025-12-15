/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DiveBombBehavior - Dives toward target Y while drifting left
*/

#include "DiveBombBehavior.hpp"

#include <cmath>

namespace rtype::games::rtype::shared {

void DiveBombBehavior::apply(AIComponent& ai,
                             const TransformComponent& transform,
                             VelocityComponent& velocity, float /*deltaTime*/) {
    velocity.vx = -ai.speed;

    float dy = ai.targetY - transform.y;
    float direction = (std::abs(dy) < 1.0F) ? 0.0F : (dy > 0.0F ? 1.0F : -1.0F);
    velocity.vy = _adjustSpeed * direction;
}

}  // namespace rtype::games::rtype::shared
