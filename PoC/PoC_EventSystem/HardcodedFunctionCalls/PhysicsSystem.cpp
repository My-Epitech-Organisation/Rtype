/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PhysicsSystem implementation
*/

#include "PhysicsSystem.hpp"
#include <cmath>
#include <iostream>
#include <vector>

namespace PoC {

    int PhysicsSystem::checkCollisions(ECS::Registry& registry) {
        int collisionCount = 0;
        
        // Get all entities with Position and Collider
        std::vector<std::tuple<ECS::Entity, Position&, Collider&>> entities;
        
        registry.view<Position, Collider>().each([&entities](auto entity, auto& pos, auto& col) {
            entities.push_back({entity, pos, col});
        });

        // Check collisions between all pairs
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                auto& [entityA, posA, colA] = entities[i];
                auto& [entityB, posB, colB] = entities[j];

                float dist = distance(posA, posB);
                float collisionDist = colA.radius + colB.radius;

                if (dist < collisionDist) {
                    std::cout << "[Physics] Collision detected between Entity " 
                              << entityA.index() << " and Entity " << entityB.index() << std::endl;
                    
                    // Tag entities as collided
                    registry.emplaceComponent<CollisionTag>(entityA);
                    registry.emplaceComponent<CollisionTag>(entityB);
                    
                    collisionCount++;
                }
            }
        }

        return collisionCount;
    }

    float PhysicsSystem::distance(const Position& a, const Position& b) const {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

} // namespace PoC
