/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** StationaryBehavior - Implementation
*/

#include "StationaryBehavior.hpp"

namespace rtype::games::rtype::shared {

void StationaryBehavior::apply(AIComponent& /*ai*/,
                               const TransformComponent& /*transform*/,
                               VelocityComponent& velocity,
                               float /*deltaTime*/) {
    velocity.vx = 0.0F;
    velocity.vy = 0.0F;
}

}  // namespace rtype::games::rtype::shared
