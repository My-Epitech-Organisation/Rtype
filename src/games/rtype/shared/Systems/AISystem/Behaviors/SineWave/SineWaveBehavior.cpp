/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SineWaveBehavior - Implementation
*/

#include "SineWaveBehavior.hpp"

#include <cmath>

namespace rtype::games::rtype::shared {

void SineWaveBehavior::apply(AIComponent& ai,
                             const TransformComponent& /*transform*/,
                             VelocityComponent& velocity, float deltaTime) {
    ai.stateTimer += deltaTime;
    velocity.vx = -ai.speed;
    velocity.vy =
        _amplitude * _frequency * std::cos(_frequency * ai.stateTimer);
}

}  // namespace rtype::games::rtype::shared
