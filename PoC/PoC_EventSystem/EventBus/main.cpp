/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Main entry point for Event Bus PoC
*/

#include "Game.hpp"
#include <iostream>

/**
 * @brief Demonstrates Observer Pattern / Event Bus architecture
 * 
 * This PoC shows how systems can be decoupled using an event bus:
 * - PhysicsSystem publishes CollisionEvent
 * - AudioSystem subscribes to CollisionEvent
 * - Neither system knows about the other
 * - Game class doesn't need to coordinate between systems
 * 
 * Key observation:
 * - Adding a new system (e.g., ParticleSystem) is trivial
 * - Just subscribe to relevant events
 * - No need to modify existing code
 * - Clear separation of concerns
 */
int main() {
    PoC::Game game;
    
    game.setup();
    game.run(5);

    std::cout << std::endl;
    std::cout << "=== Analysis ===" << std::endl;
    std::cout << "Coupling Level: LOW" << std::endl;
    std::cout << "- Systems communicate through events" << std::endl;
    std::cout << "- PhysicsSystem doesn't know about AudioSystem" << std::endl;
    std::cout << "- Game doesn't coordinate system interactions" << std::endl;
    std::cout << std::endl;
    std::cout << "Flexibility: HIGH" << std::endl;
    std::cout << "- Easy to add new systems (just subscribe)" << std::endl;
    std::cout << "- Easy to remove systems (just unsubscribe)" << std::endl;
    std::cout << "- Systems are independently testable" << std::endl;
    std::cout << std::endl;
    std::cout << "Performance: GOOD" << std::endl;
    std::cout << "- Small overhead from event dispatch" << std::endl;
    std::cout << "- Function pointer indirection" << std::endl;
    std::cout << "- Memory allocation for event objects" << std::endl;
    std::cout << "- Still very acceptable for most games" << std::endl;

    return 0;
}
