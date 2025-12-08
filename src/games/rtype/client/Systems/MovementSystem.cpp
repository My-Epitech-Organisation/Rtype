/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MovementSystem.cpp
*/

#include "MovementSystem.hpp"

#include "../AllComponents.hpp"

// Use shorter aliases for readability
namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

namespace rtype::games::rtype::client {

MovementSystem::MovementSystem() : ::rtype::engine::ASystem("MovementSystem") {}

void MovementSystem::update(ECS::Registry& registry, float dt) {
    registry.view<rs::VelocityComponent, rs::Position, Image>().each(
        [dt](auto /*entity*/, auto& velocity, auto& position,
             auto& spriteData) {
            position.x += velocity.vx * dt;
            position.y += velocity.vy * dt;
            spriteData.sprite.setPosition(sf::Vector2f(position.x, position.y));
        });
}

}  // namespace rtype::games::rtype::client
