/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Sine Wave Movement PoC - Main
*/

#include "../../ECS/ECS.hpp"
#include "SineWaveMovement.hpp"
#include <iostream>
#include <iomanip>

void printPosition(const char* name, const Movement::Position& pos) {
    std::cout << std::setw(15) << name << ": ("
              << std::fixed << std::setprecision(2)
              << std::setw(7) << pos.x << ", "
              << std::setw(7) << pos.y << ")" << std::endl;
}

int main() {
    std::cout << "=== Sine Wave Movement PoC ===" << std::endl;
    std::cout << "Formula: y = center + sin(time * freq + phase) * amp" << std::endl << std::endl;

    ECS::Registry registry;

    // Create enemy with slow oscillation (classic R-Type enemy)
    auto slowEnemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(slowEnemy, 0.0f, 50.0f);
    registry.emplaceComponent<Movement::SineWave>(slowEnemy, 
        50.0f,    // centerY
        2.0f,     // frequency
        20.0f,    // amplitude
        30.0f,    // horizontalSpeed
        0.0f      // phase
    );
    registry.emplaceComponent<Movement::SineTime>(slowEnemy);

    // Create enemy with fast oscillation
    auto fastEnemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(fastEnemy, 0.0f, 50.0f);
    registry.emplaceComponent<Movement::SineWave>(fastEnemy,
        50.0f,    // centerY
        5.0f,     // frequency (faster)
        15.0f,    // amplitude (smaller)
        30.0f,    // horizontalSpeed
        0.0f      // phase
    );
    registry.emplaceComponent<Movement::SineTime>(fastEnemy);

    // Create enemy with phase offset (out of sync)
    auto phasedEnemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(phasedEnemy, 0.0f, 50.0f);
    registry.emplaceComponent<Movement::SineWave>(phasedEnemy,
        50.0f,    // centerY
        2.0f,     // frequency
        20.0f,    // amplitude
        30.0f,    // horizontalSpeed
        3.14159f  // phase (π radians, 180° offset)
    );
    registry.emplaceComponent<Movement::SineTime>(phasedEnemy);

    // Simulate 10 frames at 60 FPS
    const float deltaTime = 1.0f / 60.0f;
    const int numFrames = 10;

    for (int frame = 0; frame <= numFrames; ++frame) {
        std::cout << "Frame " << frame << " (t=" 
                  << std::fixed << std::setprecision(3) 
                  << frame * deltaTime << "s):" << std::endl;
        
        auto& slowPos = registry.getComponent<Movement::Position>(slowEnemy);
        auto& fastPos = registry.getComponent<Movement::Position>(fastEnemy);
        auto& phasedPos = registry.getComponent<Movement::Position>(phasedEnemy);

        printPosition("Slow Wave", slowPos);
        printPosition("Fast Wave", fastPos);
        printPosition("Phased Wave", phasedPos);
        std::cout << std::endl;

        if (frame < numFrames) {
            Movement::SineWaveMovementSystem::update(registry, deltaTime);
        }
    }

    std::cout << "✓ Sine Wave Movement PoC completed successfully!" << std::endl;
    std::cout << "  - Smooth oscillating patterns" << std::endl;
    std::cout << "  - Adjustable frequency and amplitude" << std::endl;
    std::cout << "  - Phase control for synchronized formations" << std::endl;
    std::cout << "  - Perfect for classic shooter enemy patterns" << std::endl;

    return 0;
}
