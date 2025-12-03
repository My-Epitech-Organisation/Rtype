/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PatrolBehavior - Implementation
*/

#include "PatrolBehavior.hpp"

namespace rtype::games::rtype::shared {

void PatrolBehavior::apply(AIComponent& ai,
                           const TransformComponent& /*transform*/,
                           VelocityComponent& velocity, float /*deltaTime*/) {
    // TODO(Sam): Extend to support waypoint-based patrol
    velocity.vx = -ai.speed;
    velocity.vy = 0.0F;
}

}  // namespace rtype::games::rtype::shared
