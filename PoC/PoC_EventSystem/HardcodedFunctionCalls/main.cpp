/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Main entry point for Hardcoded Function Calls PoC
*/

#include "Game.hpp"
#include <iostream>

/**
 * @brief Demonstrates hardcoded function calls between systems
 * 
 * This PoC shows the traditional approach where Game::update()
 * directly calls PhysicsSystem::checkCollisions().
 * 
 * Key observation:
 * - Game class has a direct dependency on PhysicsSystem
 * - Game must include PhysicsSystem header
 * - Cannot add/remove systems without modifying Game class
 * - Clear and simple control flow
 * - No indirection or runtime overhead
 */
int main() {
    PoC::Game game;
    
    game.setup();
    game.run(5);

    std::cout << std::endl;
    std::cout << "=== Analysis ===" << std::endl;
    std::cout << "Coupling Level: HIGH" << std::endl;
    std::cout << "- Game knows about PhysicsSystem" << std::endl;
    std::cout << "- Direct compile-time dependency" << std::endl;
    std::cout << "- Must recompile Game when PhysicsSystem changes" << std::endl;
    std::cout << std::endl;
    std::cout << "Flexibility: LOW" << std::endl;
    std::cout << "- Cannot dynamically add/remove systems" << std::endl;
    std::cout << "- Hard to test systems in isolation" << std::endl;
    std::cout << std::endl;
    std::cout << "Performance: EXCELLENT" << std::endl;
    std::cout << "- No runtime overhead" << std::endl;
    std::cout << "- Direct function calls" << std::endl;
    std::cout << "- Compiler can inline optimizations" << std::endl;

    return 0;
}
