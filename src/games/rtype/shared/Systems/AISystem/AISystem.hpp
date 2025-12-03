/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** AISystem - Shared AI behavior logic
*/

#pragma once

#include "../../../../../engine/ISystem.hpp"
#include "../../Components/AIComponent.hpp"
#include "../../Components/TransformComponent.hpp"
#include "../../Components/VelocityComponent.hpp"
#include "Behaviors/BehaviorRegistry.hpp"

namespace rtype::games::rtype::shared {

/**
 * @class AISystem
 * @brief System that processes AI behavior for entities
 *
 * Shared between client (for prediction) and server (authoritative).
 * Uses the BehaviorRegistry to apply behavior strategies.
 *
 * Make sure to call registerDefaultBehaviors() before using this system.
 */
class AISystem : public ::rtype::engine::ISystem {
   public:
    AISystem() = default;

    /**
     * @brief Update all entities with AI components
     * @param registry ECS registry containing entities
     * @param deltaTime Time elapsed since last update
     */
    void update(ECS::Registry& registry, float deltaTime) override;

    [[nodiscard]] const std::string getName() const noexcept override {
        return "AISystem";
    }
};

}  // namespace rtype::games::rtype::shared
