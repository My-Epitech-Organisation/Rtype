/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** MovementSystem.cpp
*/

#include "MovementSystem.hpp"

#include "Components/PositionComponent.hpp"
#include "../Components/ImageComponent.hpp"
#include "../Components/VelocityComponent.hpp"

void MovementSystem::update(const std::shared_ptr<ECS::Registry>& registry,
                            float dt) {
    registry->view<Velocity, Position, Image>().each(
        [dt](auto _, auto& velocity, auto& position, auto& spriteData) {
            position.x += velocity.x * dt;
            position.y += velocity.y * dt;

            spriteData.sprite.setPosition(sf::Vector2f(position.x, position.y));
        });
}
