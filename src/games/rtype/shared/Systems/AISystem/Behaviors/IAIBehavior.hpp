/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** IAIBehavior - Interface for AI behavior strategies
*/

#pragma once

#include <string>

#include "../../../Components/AIComponent.hpp"
#include "../../../Components/TransformComponent.hpp"
#include "../../../Components/VelocityComponent.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class IAIBehavior
 * @brief Abstract interface for AI behavior strategies
 *
 * Each behavior implements the Strategy pattern, allowing easy extension
 * of AI capabilities without modifying the AISystem.
 *
 * To add a new behavior:
 * 1. Add a new enum value in AIBehavior (AIComponent.hpp)
 * 2. Create a new class inheriting from IAIBehavior
 * 3. Register it in BehaviorRegistry
 */
class IAIBehavior {
   public:
    virtual ~IAIBehavior() = default;

    IAIBehavior(const IAIBehavior&) = delete;
    IAIBehavior& operator=(const IAIBehavior&) = delete;
    IAIBehavior(IAIBehavior&&) = delete;
    IAIBehavior& operator=(IAIBehavior&&) = delete;

    /**
     * @brief Apply the behavior to update entity velocity
     * @param ai AI component with behavior configuration
     * @param transform Current entity transform (position)
     * @param velocity Velocity component to update
     * @param deltaTime Time elapsed since last update
     */
    virtual void apply(AIComponent& ai, const TransformComponent& transform,
                       VelocityComponent& velocity, float deltaTime) = 0;

    /**
     * @brief Get the behavior type this strategy handles
     * @return The AIBehavior enum value
     */
    [[nodiscard]] virtual AIBehavior getType() const noexcept = 0;

    /**
     * @brief Get a human-readable name for debugging
     * @return Behavior name string
     */
    [[nodiscard]] virtual const std::string getName() const noexcept = 0;

   protected:
    IAIBehavior() = default;
};

}  // namespace rtype::games::rtype::shared
