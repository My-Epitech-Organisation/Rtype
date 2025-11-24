/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Scripted Movement PoC - Main
*/

#include "../../ECS/ECS.hpp"
#include "ScriptedMovement.hpp"
#include <iostream>
#include <iomanip>

void printPosition(const char* name, const Movement::Position& pos) {
    std::cout << std::setw(20) << name << ": ("
              << std::fixed << std::setprecision(2)
              << std::setw(7) << pos.x << ", "
              << std::setw(7) << pos.y << ")" << std::endl;
}

void printScriptState(const Movement::MovementScript& script) {
    if (script.isComplete()) {
        std::cout << "  Script: COMPLETED" << std::endl;
    } else {
        std::cout << "  Script: Command " << (script.currentCommand + 1) 
                  << "/" << script.commands.size() << " - "
                  << script.commands[script.currentCommand]->getName() << std::endl;
    }
}

int main() {
    std::cout << "=== Scripted Movement PoC ===" << std::endl;
    std::cout << "Parse and execute movement commands from text files" << std::endl << std::endl;

    ECS::Registry registry;

    // Example 1: Enemy with file-based script
    std::cout << "=== Example 1: File-based Script ===" << std::endl;
    auto fileEnemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(fileEnemy, 0.0f, 0.0f);
    
    try {
        auto fileScript = Movement::ScriptParser::parseFile("movement_script.txt");
        std::cout << "Loaded script with " << fileScript->commands.size() << " commands" << std::endl << std::endl;
        registry.emplaceComponent<Movement::MovementScript>(fileEnemy, std::move(*fileScript));
    } catch (const std::exception& e) {
        std::cout << "Note: Could not load file (this is expected in PoC): " << e.what() << std::endl;
        std::cout << "Continuing with inline script..." << std::endl << std::endl;
    }

    // Example 2: Enemy with inline script
    std::cout << "=== Example 2: Inline Script ===" << std::endl;
    auto inlineEnemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(inlineEnemy, 0.0f, 0.0f);
    
    std::string inlineScript = R"(
# Patrol pattern
Move(Type=Linear, Speed=60, DirX=1, DirY=0)
Wait(Duration=1.0)
Move(Type=Linear, Speed=60, DirX=0, DirY=1)
Wait(Duration=1.0)
Move(Type=Linear, Speed=60, DirX=-1, DirY=0)
Wait(Duration=1.0)
Move(Type=Linear, Speed=60, DirX=0, DirY=-1)
    )";
    
    auto parsedScript = Movement::ScriptParser::parseString(inlineScript);
    std::cout << "Parsed inline script with " << parsedScript->commands.size() << " commands" << std::endl << std::endl;
    registry.emplaceComponent<Movement::MovementScript>(inlineEnemy, std::move(*parsedScript));

    // Example 3: Enemy with programmatic script
    std::cout << "=== Example 3: Programmatic Script ===" << std::endl;
    auto progEnemy = registry.spawnEntity();
    registry.emplaceComponent<Movement::Position>(progEnemy, 50.0f, 50.0f);
    
    Movement::MovementScript progScript;
    progScript.addCommand(std::make_unique<Movement::MoveToCommand>(100.0f, 0.0f, 80.0f));
    progScript.addCommand(std::make_unique<Movement::WaitCommand>(0.5f));
    progScript.addCommand(std::make_unique<Movement::MoveToCommand>(0.0f, 100.0f, 80.0f));
    progScript.addCommand(std::make_unique<Movement::WaitCommand>(0.5f));
    progScript.addCommand(std::make_unique<Movement::LinearCommand>(50.0f, 1.0f, 0.0f));
    std::cout << "Created programmatic script with " << progScript.commands.size() << " commands" << std::endl << std::endl;
    registry.emplaceComponent<Movement::MovementScript>(progEnemy, std::move(progScript));

    // Simulate movement
    const float deltaTime = 0.1f;  // 100ms per frame
    const int numFrames = 50;

    for (int frame = 0; frame <= numFrames && frame <= 10; ++frame) {
        if (frame % 5 == 0) {
            std::cout << "Frame " << frame << " (t=" 
                      << std::fixed << std::setprecision(1) 
                      << frame * deltaTime << "s):" << std::endl;
            
            auto& inlinePos = registry.getComponent<Movement::Position>(inlineEnemy);
            auto& inlineScr = registry.getComponent<Movement::MovementScript>(inlineEnemy);
            
            auto& progPos = registry.getComponent<Movement::Position>(progEnemy);
            auto& progScr = registry.getComponent<Movement::MovementScript>(progEnemy);

            printPosition("Inline Enemy", inlinePos);
            printScriptState(inlineScr);
            
            printPosition("Programmatic Enemy", progPos);
            printScriptState(progScr);
            std::cout << std::endl;
        }

        if (frame < numFrames) {
            Movement::ScriptedMovementSystem::update(registry, deltaTime);
        }
    }

    std::cout << "✓ Scripted Movement PoC completed successfully!" << std::endl;
    std::cout << "  - Text-based movement definition" << std::endl;
    std::cout << "  - Easy for designers to modify" << std::endl;
    std::cout << "  - Supports sequential command execution" << std::endl;
    std::cout << "  - Extensible command system" << std::endl;
    std::cout << "  - Can load from files or define programmatically" << std::endl;

    return 0;
}
