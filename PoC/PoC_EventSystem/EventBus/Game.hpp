/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game class for Event Bus PoC
*/

#ifndef GAME_HPP
    #define GAME_HPP

    #include "ECS.hpp"
    #include "EventBus.hpp"
    #include "PhysicsSystem.hpp"
    #include "AudioSystem.hpp"

namespace PoC {

    /**
     * @brief Main game class using Event Bus architecture
     * 
     * This class demonstrates LOOSE COUPLING:
     * - Game doesn't call systems directly
     * - Systems communicate through EventBus
     * - Easy to add/remove systems without modifying Game
     * - Systems are independent and testable
     * 
     * Pros:
     * - Low coupling between systems
     * - Easy to extend with new systems
     * - Systems don't know about each other
     * - Better testability
     * 
     * Cons:
     * - More complex architecture
     * - Runtime overhead from event dispatching
     * - Harder to trace execution flow
     * - Need to manage event lifetimes
     */
    class Game {
    public:
        Game();

        /**
         * @brief Initialize game entities and systems
         */
        void setup();

        /**
         * @brief Main update loop - systems communicate via events
         */
        void update(float deltaTime);

        /**
         * @brief Run the game for a number of frames
         */
        void run(int frames);

        ECS::Registry& getRegistry() { return _registry; }
        EventBus& getEventBus() { return _eventBus; }

    private:
        ECS::Registry _registry;
        EventBus _eventBus;           // Central event hub
        PhysicsSystem _physicsSystem; // Publishes events
        AudioSystem _audioSystem;     // Subscribes to events
    };

} // namespace PoC

#endif // GAME_HPP
