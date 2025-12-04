/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SineWaveBehavior - Sine wave movement AI behavior
*/

#pragma once

#include "../IAIBehavior.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class SineWaveBehavior
 * @brief AI that moves left while oscillating vertically in a sine wave
 *
 * Creates a wavy movement pattern, commonly used for mid-tier enemies.
 */
class SineWaveBehavior final : public IAIBehavior {
   public:
    /**
     * @brief Construct with configurable wave parameters
     * @param amplitude Vertical oscillation amplitude in pixels
     * @param frequency Oscillation frequency multiplier
     */
    explicit SineWaveBehavior(float amplitude = 50.0F, float frequency = 2.0F)
        : _amplitude(amplitude), _frequency(frequency) {}

    void apply(AIComponent& ai, const TransformComponent& transform,
               VelocityComponent& velocity, float deltaTime) override;

    [[nodiscard]] AIBehavior getType() const noexcept override {
        return AIBehavior::SineWave;
    }

    [[nodiscard]] const std::string getName() const noexcept override {
        return "SineWaveBehavior";
    }

   private:
    float _amplitude;
    float _frequency;
};

}  // namespace rtype::games::rtype::shared
