/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Linear Movement PoC - Main
*/

#include "../../ECS/ECS.hpp"
#include "LinearMovement.hpp"
#include <iostream>
#include <iomanip>

void printPosition(const char* name, const Movement::Position& pos) {
    std::cout << std::setw(12) << name << ": ("
              << std::fixed << std::setprecision(2)
              << std::setw(7) << pos.x << ", "
              << std::setw(7) << pos.y << ")" << std::endl;
}

int main() {
    std::cout << "=== Linear Movement PoC ===" << std::endl;
    std::cout << "Formula: pos += dir * speed * dt" << std::endl << std::endl;

    ECS::Registry registry;

    // Create bullet moving right
    auto bullet = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(bullet, 0.0f, 0.0f);
    auto& bulletDir = registry.emplaceComponent<Movement::Direction>(bullet, 1.0f, 0.0f);
    bulletDir.normalize();
    registry.emplaceComponent<Movement::Speed>(bullet, 100.0f);

    // Create enemy moving down-left
    auto enemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(enemy, 100.0f, 0.0f);
    auto& enemyDir = registry.emplaceComponent<Movement::Direction>(enemy, -1.0f, 1.0f);
    enemyDir.normalize();
    registry.emplaceComponent<Movement::Speed>(enemy, 50.0f);

    // Create particle moving up-right
    auto particle = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(particle, 50.0f, 50.0f);
    auto& particleDir = registry.emplaceComponent<Movement::Direction>(particle, 1.0f, -1.0f);
    particleDir.normalize();
    registry.emplaceComponent<Movement::Speed>(particle, 75.0f);

    // Simulate 5 frames at 60 FPS (0.0166s per frame)
    const float deltaTime = 1.0f / 60.0f;
    const int numFrames = 5;

    for (int frame = 0; frame <= numFrames; ++frame) {
        std::cout << "Frame " << frame << ":" << std::endl;
        
        auto& bulletPos = registry.getComponent<Movement::Position>(bullet);
        auto& enemyPos = registry.getComponent<Movement::Position>(enemy);
        auto& particlePos = registry.getComponent<Movement::Position>(particle);

        printPosition("Bullet", bulletPos);
        printPosition("Enemy", enemyPos);
        printPosition("Particle", particlePos);
        std::cout << std::endl;

        if (frame < numFrames) {
            Movement::LinearMovementSystem::update(registry, deltaTime);
        }
    }

    std::cout << "✓ Linear Movement PoC completed successfully!" << std::endl;
    std::cout << "  - Simple and predictable movement" << std::endl;
    std::cout << "  - Constant velocity in fixed direction" << std::endl;
    std::cout << "  - Perfect for bullets and projectiles" << std::endl;

    return 0;
}
