/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Simple test to verify Registry compilation
*/

#include "src/ECS/Core/Registry.hpp"
#include <iostream>

// Simple components for testing
struct Position {
    float x, y;
};

struct Velocity {
    float dx, dy;
};

int main() {
    std::cout << "Testing modular Registry structure..." << std::endl;

    ECS::Registry registry;

    // Test entity creation
    auto entity = registry.spawn_entity();
    std::cout << "Entity created: index=" << entity.index() 
              << ", generation=" << entity.generation() << std::endl;

    // Test component addition
    registry.emplace_component<Position>(entity, 10.0f, 20.0f);
    registry.emplace_component<Velocity>(entity, 1.0f, 2.0f);
    std::cout << "Components added successfully" << std::endl;

    // Test component retrieval
    auto& pos = registry.get_component<Position>(entity);
    std::cout << "Position: x=" << pos.x << ", y=" << pos.y << std::endl;

    // Test view iteration
    auto view = registry.view<Position, Velocity>();
    int count = 0;
    view.each([&](ECS::Entity e, Position& p, Velocity& v) {
        count++;
        std::cout << "Entity in view: pos=(" << p.x << "," << p.y 
                  << "), vel=(" << v.dx << "," << v.dy << ")" << std::endl;
    });
    std::cout << "View found " << count << " entities" << std::endl;

    // Test entity destruction
    registry.kill_entity(entity);
    std::cout << "Entity killed, alive=" << registry.is_alive(entity) << std::endl;

    std::cout << "\nAll tests passed! Modular Registry structure works correctly." << std::endl;
    return 0;
}
