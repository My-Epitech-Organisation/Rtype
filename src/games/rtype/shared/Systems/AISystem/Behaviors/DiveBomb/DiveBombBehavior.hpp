/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** DiveBombBehavior - Dives toward target Y while drifting left
*/

#pragma once

#include "../IAIBehavior.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class DiveBombBehavior
 * @brief AI that drifts left and adjusts vertical velocity toward a target Y
 */
class DiveBombBehavior final : public IAIBehavior {
   public:
    explicit DiveBombBehavior(float adjustSpeed = 120.0F)
        : _adjustSpeed(adjustSpeed) {}

    void apply(AIComponent& ai, const TransformComponent& transform,
               VelocityComponent& velocity, float deltaTime) override;

    [[nodiscard]] AIBehavior getType() const noexcept override {
        return AIBehavior::DiveBomb;
    }

    [[nodiscard]] const std::string getName() const noexcept override {
        return "DiveBombBehavior";
    }

   private:
    float _adjustSpeed;
};

}  // namespace rtype::games::rtype::shared
