/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ChaseBehavior - Target-chasing AI behavior
*/

#pragma once

#include "../IAIBehavior.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class ChaseBehavior
 * @brief AI that moves toward a target position
 *
 * Calculates direction to target and moves at configured speed.
 * Stops when very close to target to prevent jitter.
 */
class ChaseBehavior final : public IAIBehavior {
   public:
    /**
     * @brief Construct with configurable stop distance
     * @param stopDistance Distance at which entity stops chasing
     */
    explicit ChaseBehavior(float stopDistance = 1.0F)
        : _stopDistance(stopDistance) {}

    void apply(AIComponent& ai, const TransformComponent& transform,
               VelocityComponent& velocity, float deltaTime) override;

    [[nodiscard]] AIBehavior getType() const noexcept override {
        return AIBehavior::Chase;
    }

    [[nodiscard]] const std::string getName() const noexcept override {
        return "ChaseBehavior";
    }

   private:
    float _stopDistance;
};

}  // namespace rtype::games::rtype::shared
