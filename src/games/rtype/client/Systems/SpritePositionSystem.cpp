/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SpritePositionSystem - Synchronizes sprite rendering positions with game state
*/

#include "SpritePositionSystem.hpp"

#include "../AllComponents.hpp"

// Use shorter aliases for readability
namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

namespace rtype::games::rtype::client {

SpritePositionSystem::SpritePositionSystem()
    : ::rtype::engine::ASystem("SpritePositionSystem") {}

void SpritePositionSystem::update(ECS::Registry& registry, float dt) {
    // Synchronize sprite positions with Position component
    // Note: Movement logic is handled by shared::MovementSystem
    // This system only updates the visual representation
    registry.view<rs::TransformComponent, Image>().each(
        [dt](auto /*entity*/, auto& position, auto& spriteData) {
            spriteData.sprite.setPosition(sf::Vector2f(position.x, position.y));
        });
}

}  // namespace rtype::games::rtype::client
