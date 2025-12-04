/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MoveLeftBehavior - Simple left movement AI behavior
*/

#pragma once

#include "../IAIBehavior.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class MoveLeftBehavior
 * @brief Simple AI that moves continuously to the left
 *
 * Used for basic enemies that simply scroll across the screen.
 */
class MoveLeftBehavior final : public IAIBehavior {
   public:
    MoveLeftBehavior() = default;

    void apply(AIComponent& ai, const TransformComponent& transform,
               VelocityComponent& velocity, float deltaTime) override;

    [[nodiscard]] AIBehavior getType() const noexcept override {
        return AIBehavior::MoveLeft;
    }

    [[nodiscard]] const std::string getName() const noexcept override {
        return "MoveLeftBehavior";
    }
};

}  // namespace rtype::games::rtype::shared
