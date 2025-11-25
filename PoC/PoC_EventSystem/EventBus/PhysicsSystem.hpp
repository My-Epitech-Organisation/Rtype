/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PhysicsSystem for Event Bus PoC
*/

#ifndef PHYSICSSYSTEM_HPP
    #define PHYSICSSYSTEM_HPP

    #include "ECS.hpp"
    #include "EventBus.hpp"
    #include "Components.hpp"

namespace PoC {

    /**
     * @brief Physics system that publishes collision events
     * 
     * This system is DECOUPLED from other systems:
     * - Doesn't know who listens to collision events
     * - Doesn't call any other system directly
     * - Only publishes events through EventBus
     */
    class PhysicsSystem {
    public:
        PhysicsSystem(EventBus& eventBus);

        /**
         * @brief Checks for collisions and publishes events
         * @param registry ECS registry containing all entities
         * @return Number of collisions detected
         */
        int checkCollisions(ECS::Registry& registry);

    private:
        EventBus& _eventBus;

        /**
         * @brief Calculates distance between two positions
         */
        float distance(const Position& a, const Position& b) const;
    };

} // namespace PoC

#endif // PHYSICSSYSTEM_HPP
