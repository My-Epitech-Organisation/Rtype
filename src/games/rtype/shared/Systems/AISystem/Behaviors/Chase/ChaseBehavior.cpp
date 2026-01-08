/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChaseBehavior - Implementation
*/

#include "ChaseBehavior.hpp"

#include <cmath>

#include "Logger/Macros.hpp"

namespace rtype::games::rtype::shared {

void ChaseBehavior::apply(AIComponent& ai, const TransformComponent& transform,
                          VelocityComponent& velocity, float /*deltaTime*/) {
    float dx = ai.targetX - transform.x;
    float dy = ai.targetY - transform.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                  "[ChaseBehavior] pos=("
                      << transform.x << "," << transform.y << ")"
                      << " target=(" << ai.targetX << "," << ai.targetY << ")"
                      << " dist=" << dist << " speed=" << ai.speed);

    if (dist > _stopDistance) {
        velocity.vx = (dx / dist) * ai.speed;
        velocity.vy = (dy / dist) * ai.speed;

        LOG_DEBUG_CAT(::rtype::LogCategory::GameEngine,
                      "[ChaseBehavior] Setting velocity=("
                          << velocity.vx << "," << velocity.vy << ")");
    } else {
        velocity.vx = 0.0F;
        velocity.vy = 0.0F;
    }
}

}  // namespace rtype::games::rtype::shared
