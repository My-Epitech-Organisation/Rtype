/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** StationaryBehavior - Stationary (no movement) AI behavior
*/

#pragma once

#include "../IAIBehavior.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class StationaryBehavior
 * @brief AI that stays in place (zero velocity)
 *
 * Used for turrets, mines, or other static enemies.
 */
class StationaryBehavior final : public IAIBehavior {
   public:
    StationaryBehavior() = default;

    void apply(AIComponent& ai, const TransformComponent& transform,
               VelocityComponent& velocity, float deltaTime) override;

    [[nodiscard]] AIBehavior getType() const noexcept override {
        return AIBehavior::Stationary;
    }

    [[nodiscard]] const std::string getName() const noexcept override {
        return "StationaryBehavior";
    }
};

}  // namespace rtype::games::rtype::shared
