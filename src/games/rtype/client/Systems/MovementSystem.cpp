/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MovementSystem.cpp
*/

#include "MovementSystem.hpp"

#include "../../shared/Components/VelocityComponent.hpp"
#include "../Components/ImageComponent.hpp"
#include "Components/PositionComponent.hpp"

void MovementSystem::update(const std::shared_ptr<ECS::Registry>& registry,
                            float dt) {
    registry
        ->view<rtype::games::rtype::shared::VelocityComponent, Position,
               Image>()
        .each([dt](auto _, auto& velocity, auto& position, auto& spriteData) {
            position.x += velocity.vx * dt;
            position.y += velocity.vy * dt;

            spriteData.sprite.setPosition(sf::Vector2f(position.x, position.y));
        });
}
