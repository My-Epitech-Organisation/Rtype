/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PhysicsSystem for Hardcoded Function Calls PoC
*/

#ifndef PHYSICSSYSTEM_HPP
    #define PHYSICSSYSTEM_HPP

    #include "ECS.hpp"
    #include "Components.hpp"

namespace PoC {

    /**
     * @brief Physics system that handles collision detection
     * 
     * This system is directly called by Game::update(), demonstrating
     * tight coupling between systems.
     */
    class PhysicsSystem {
    public:
        /**
         * @brief Checks for collisions between entities
         * @param registry ECS registry containing all entities
         * @return Number of collisions detected
         */
        int checkCollisions(ECS::Registry& registry);

    private:
        /**
         * @brief Calculates distance between two positions
         */
        float distance(const Position& a, const Position& b) const;
    };

} // namespace PoC

#endif // PHYSICSSYSTEM_HPP
