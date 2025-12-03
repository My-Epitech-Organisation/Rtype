/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** PatrolBehavior - Patrol movement AI behavior
*/

#pragma once

#include "../IAIBehavior.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class PatrolBehavior
 * @brief AI that patrols (currently moves left, can be extended for waypoints)
 *
 * Simple patrol behavior. Can be extended to support waypoint-based movement.
 */
class PatrolBehavior final : public IAIBehavior {
   public:
    PatrolBehavior() = default;

    void apply(AIComponent& ai, const TransformComponent& transform,
               VelocityComponent& velocity, float deltaTime) override;

    [[nodiscard]] AIBehavior getType() const noexcept override {
        return AIBehavior::Patrol;
    }

    [[nodiscard]] const std::string getName() const noexcept override {
        return "PatrolBehavior";
    }
};

}  // namespace rtype::games::rtype::shared
