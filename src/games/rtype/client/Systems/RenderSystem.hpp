/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_

#include <SFML/Graphics/RenderWindow.hpp>

#include "../Components/HiddenComponent.hpp"
#include "ASystem.hpp"
#include "ecs/ECS.hpp"
namespace rtype::games::rtype::client {
class RenderSystem : public ::rtype::engine::ASystem {
   private:
    std::shared_ptr<sf::RenderWindow> _window;

   public:
    static bool isEntityHidden(ECS::Registry& registry, ECS::Entity entity);
    explicit RenderSystem(std::shared_ptr<sf::RenderWindow> window);
    void update(ECS::Registry& registry, float dt) override;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_
