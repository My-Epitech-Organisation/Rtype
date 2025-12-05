/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_

#include <SFML/Graphics/RenderWindow.hpp>

#include "ASystem.hpp"
#include "ecs/ECS.hpp"

class RenderSystem : public rtype::engine::ASystem {
   private:
    std::shared_ptr<sf::RenderWindow> _window;

   public:
    explicit RenderSystem(std::shared_ptr<sf::RenderWindow> window);
    void update(ECS::Registry& registry, float dt) override;
};

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_
