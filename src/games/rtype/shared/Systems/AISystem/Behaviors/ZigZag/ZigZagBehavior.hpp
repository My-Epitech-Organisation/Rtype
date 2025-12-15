/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** ZigZagBehavior - Alternating vertical steps while moving left
*/

#pragma once

#include "../IAIBehavior.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class ZigZagBehavior
 * @brief AI that moves left while alternating vertical direction at intervals
 */
class ZigZagBehavior final : public IAIBehavior {
   public:
    explicit ZigZagBehavior(float switchInterval = 0.8F,
                            float stepSpeed = 80.0F)
        : _switchInterval(switchInterval), _stepSpeed(stepSpeed) {}

    void apply(AIComponent& ai, const TransformComponent& transform,
               VelocityComponent& velocity, float deltaTime) override;

    [[nodiscard]] AIBehavior getType() const noexcept override {
        return AIBehavior::ZigZag;
    }

    [[nodiscard]] const std::string getName() const noexcept override {
        return "ZigZagBehavior";
    }

   private:
    float _switchInterval;
    float _stepSpeed;
};

}  // namespace rtype::games::rtype::shared
