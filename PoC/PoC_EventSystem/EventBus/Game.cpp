/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Game implementation
*/

#include "Game.hpp"
#include "Components.hpp"
#include <iostream>

namespace PoC {

    Game::Game()
        : _physicsSystem(_eventBus),
          _audioSystem(_eventBus, _registry) {
        std::cout << "=== Event Bus / Observer Pattern PoC ===" << std::endl;
        std::cout << "Systems communicate through EventBus" << std::endl;
        std::cout << std::endl;
    }

    void Game::setup() {
        std::cout << "[Game] Setting up systems..." << std::endl;
        
        // Initialize audio system (subscribes to events)
        _audioSystem.initialize();
        
        std::cout << "[Game] Setting up entities..." << std::endl;

        // Create entity 1 at (0, 0)
        auto entity1 = _registry.spawnEntity();
        _registry.emplaceComponent<Position>(entity1, 0.0f, 0.0f);
        _registry.emplaceComponent<Velocity>(entity1, 1.0f, 0.0f);
        _registry.emplaceComponent<Collider>(entity1, 1.0f);
        std::cout << "[Game] Created Entity " << entity1.index() << " at (0, 0)" << std::endl;

        // Create entity 2 at (1.5, 0) - will collide after first update
        auto entity2 = _registry.spawnEntity();
        _registry.emplaceComponent<Position>(entity2, 1.5f, 0.0f);
        _registry.emplaceComponent<Velocity>(entity2, -0.5f, 0.0f);
        _registry.emplaceComponent<Collider>(entity2, 1.0f);
        std::cout << "[Game] Created Entity " << entity2.index() << " at (1.5, 0)" << std::endl;

        // Create entity 3 far away - won't collide
        auto entity3 = _registry.spawnEntity();
        _registry.emplaceComponent<Position>(entity3, 10.0f, 10.0f);
        _registry.emplaceComponent<Velocity>(entity3, 0.0f, 0.0f);
        _registry.emplaceComponent<Collider>(entity3, 1.0f);
        std::cout << "[Game] Created Entity " << entity3.index() << " at (10, 10)" << std::endl;
        
        std::cout << std::endl;
        std::cout << "[Game] EventBus has " << _eventBus.subscriberCount<CollisionEvent>() 
                  << " subscriber(s) to CollisionEvent" << std::endl;
        std::cout << std::endl;
    }

    void Game::update(float deltaTime) {
        // Update positions based on velocity
        _registry.view<Position, Velocity>().each([deltaTime](auto entity, auto& pos, auto& vel) {
            pos.x += vel.dx * deltaTime;
            pos.y += vel.dy * deltaTime;
        });

        // ✅ DECOUPLED SYSTEM CALL
        // PhysicsSystem publishes events, AudioSystem receives them automatically
        // Game doesn't need to know about AudioSystem
        // We can add more systems (ParticleSystem, NetworkSystem, etc.) without changing this code
        int collisions = _physicsSystem.checkCollisions(_registry);
        
        if (collisions > 0) {
            std::cout << "[Game] Total collisions this frame: " << collisions << std::endl;
        }
    }

    void Game::run(int frames) {
        std::cout << "[Game] Running simulation for " << frames << " frames" << std::endl;
        std::cout << std::endl;

        for (int i = 0; i < frames; ++i) {
            std::cout << "--- Frame " << (i + 1) << " ---" << std::endl;
            update(1.0f);
            std::cout << std::endl;
        }

        std::cout << "[Game] Simulation complete" << std::endl;
    }

} // namespace PoC
