/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** SpritePositionSystem - Synchronizes sprite rendering positions with game
* state
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_SPRITEPOSITIONSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_SPRITEPOSITIONSYSTEM_HPP_
#include <memory>

#include "ASystem.hpp"
#include "ECS.hpp"

namespace rtype::games::rtype::client {

/**
 * @class SpritePositionSystem
 * @brief Synchronizes SFML sprite positions with entity Position components
 *
 * This is a CLIENT-ONLY rendering system that updates sf::Sprite positions
 * to match the game state. It does NOT handle movement logic - that's done
 * by the shared MovementSystem which updates TransformComponent/Position.
 *
 * Responsibility: Update sprite.setPosition() based on Position component
 */
class SpritePositionSystem : public ::rtype::engine::ASystem {
   public:
    SpritePositionSystem();
    void update(ECS::Registry& registry, float dt) override;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_SPRITEPOSITIONSYSTEM_HPP_
