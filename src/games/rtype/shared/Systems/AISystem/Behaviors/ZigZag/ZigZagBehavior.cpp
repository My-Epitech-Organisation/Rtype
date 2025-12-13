/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ZigZagBehavior - Alternating vertical steps while moving left
*/

#include "ZigZagBehavior.hpp"

namespace rtype::games::rtype::shared {

void ZigZagBehavior::apply(AIComponent& ai,
                           const TransformComponent& /*transform*/,
                           VelocityComponent& velocity, float deltaTime) {
    ai.stateTimer += deltaTime;

    if (ai.targetY == 0.0F) {
        ai.targetY = 1.0F;
    }
    if (ai.stateTimer >= _switchInterval) {
        ai.stateTimer = 0.0F;
        ai.targetY = (ai.targetY >= 0.0F) ? -1.0F : 1.0F;
    }

    const float direction = (ai.targetY >= 0.0F) ? 1.0F : -1.0F;
    velocity.vx = -ai.speed;
    velocity.vy = _stepSpeed * direction;
}

}  // namespace rtype::games::rtype::shared
