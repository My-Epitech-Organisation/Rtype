/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Bézier Curve Movement PoC - Main
*/

#include "../../ECS/ECS.hpp"
#include "BezierMovement.hpp"
#include <iostream>
#include <iomanip>

void printPosition(const char* name, const Movement::Position& pos, float t) {
    std::cout << std::setw(20) << name << " (t=" 
              << std::fixed << std::setprecision(2) << std::setw(4) << t << "): ("
              << std::setw(7) << pos.x << ", "
              << std::setw(7) << pos.y << ")" << std::endl;
}

int main() {
    std::cout << "=== Bézier Curve Movement PoC ===" << std::endl;
    std::cout << "Quadratic: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2" << std::endl;
    std::cout << "Cubic: B(t) = (1-t)³P0 + 3(1-t)²tP1 + 3(1-t)t²P2 + t³P3" << std::endl << std::endl;

    ECS::Registry registry;

    // Create entity with quadratic Bézier (simple arc)
    auto quadEnemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(quadEnemy, 0.0f, 0.0f);
    registry.emplaceComponent<Movement::QuadraticBezier>(quadEnemy,
        Movement::Point(0.0f, 50.0f),      // Start
        Movement::Point(50.0f, 0.0f),      // Control (creates arc)
        Movement::Point(100.0f, 50.0f),    // End
        0.5f                                // Speed (completes in 2 seconds)
    );

    // Create entity with quadratic Bézier (dive pattern)
    auto divingEnemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(divingEnemy, 100.0f, 0.0f);
    registry.emplaceComponent<Movement::QuadraticBezier>(divingEnemy,
        Movement::Point(100.0f, 0.0f),     // Start (top)
        Movement::Point(50.0f, 80.0f),     // Control (dive point)
        Movement::Point(0.0f, 0.0f),       // End (back to top)
        0.5f
    );

    // Create entity with cubic Bézier (S-curve)
    auto sCurveEnemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(sCurveEnemy, 0.0f, 0.0f);
    registry.emplaceComponent<Movement::CubicBezier>(sCurveEnemy,
        Movement::Point(0.0f, 0.0f),       // Start
        Movement::Point(30.0f, 70.0f),     // Control 1
        Movement::Point(70.0f, -20.0f),    // Control 2
        Movement::Point(100.0f, 50.0f),    // End
        0.5f
    );

    // Simulate movement over 2.5 seconds
    const float deltaTime = 0.1f;  // 100ms per frame
    const int numFrames = 25;

    std::cout << "=== Quadratic Bézier Examples ===" << std::endl << std::endl;

    for (int frame = 0; frame <= numFrames; ++frame) {
        if (frame % 5 == 0) {  // Print every 5th frame for clarity
            std::cout << "Time: " << std::fixed << std::setprecision(1) 
                      << frame * deltaTime << "s" << std::endl;
            
            auto& quadPos = registry.getComponent<Movement::Position>(quadEnemy);
            auto& quadBez = registry.getComponent<Movement::QuadraticBezier>(quadEnemy);
            
            auto& divePos = registry.getComponent<Movement::Position>(divingEnemy);
            auto& diveBez = registry.getComponent<Movement::QuadraticBezier>(divingEnemy);
            
            auto& sPos = registry.getComponent<Movement::Position>(sCurveEnemy);
            auto& sBez = registry.getComponent<Movement::CubicBezier>(sCurveEnemy);

            printPosition("Arc Path", quadPos, quadBez.t);
            printPosition("Dive Pattern", divePos, diveBez.t);
            printPosition("S-Curve (Cubic)", sPos, sBez.t);
            std::cout << std::endl;
        }

        if (frame < numFrames) {
            Movement::QuadraticBezierSystem::update(registry, deltaTime);
            Movement::CubicBezierSystem::update(registry, deltaTime);
        }
    }

    std::cout << "✓ Bézier Curve Movement PoC completed successfully!" << std::endl;
    std::cout << "  - Smooth curved paths" << std::endl;
    std::cout << "  - Quadratic Bézier: Simple arcs with 3 control points" << std::endl;
    std::cout << "  - Cubic Bézier: Complex S-curves with 4 control points" << std::endl;
    std::cout << "  - Perfect for cinematic enemy entrances" << std::endl;
    std::cout << "  - Ideal for boss movement patterns" << std::endl;

    return 0;
}
