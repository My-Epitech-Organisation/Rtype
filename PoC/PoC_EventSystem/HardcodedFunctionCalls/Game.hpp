/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game class for Hardcoded Function Calls PoC
*/

#ifndef GAME_HPP
    #define GAME_HPP

    #include "ECS.hpp"
    #include "PhysicsSystem.hpp"

namespace PoC {

    /**
     * @brief Main game class that orchestrates all systems
     * 
     * This class demonstrates tight coupling by directly calling
     * PhysicsSystem::checkCollisions() from its update method.
     * 
     * Pros:
     * - Simple and straightforward
     * - No overhead from event dispatching
     * - Direct control flow
     * - Easy to debug (clear call stack)
     * 
     * Cons:
     * - Tight coupling between Game and PhysicsSystem
     * - Hard to test in isolation
     * - Difficult to add/remove systems dynamically
     * - Game class must know about all system implementations
     */
    class Game {
    public:
        Game();

        /**
         * @brief Initialize game entities
         */
        void setup();

        /**
         * @brief Main update loop - directly calls physics
         * 
         * HARDCODED FUNCTION CALL: Physics::checkCollisions()
         * This creates tight coupling between Game and PhysicsSystem
         */
        void update(float deltaTime);

        /**
         * @brief Run the game for a number of frames
         */
        void run(int frames);

        ECS::Registry& getRegistry() { return _registry; }

    private:
        ECS::Registry _registry;
        PhysicsSystem _physicsSystem;  // Direct dependency on PhysicsSystem
    };

} // namespace PoC

#endif // GAME_HPP
