/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MoveLeftBehavior - Implementation
*/

#include "MoveLeftBehavior.hpp"

namespace rtype::games::rtype::shared {

void MoveLeftBehavior::apply(AIComponent& ai,
                             const TransformComponent& /*transform*/,
                             VelocityComponent& velocity, float /*deltaTime*/) {
    velocity.vx = -ai.speed;
    velocity.vy = 0.0F;
}

}  // namespace rtype::games::rtype::shared
