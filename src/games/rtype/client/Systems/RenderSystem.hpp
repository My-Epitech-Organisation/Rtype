/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_

#include <SFML/Graphics/RenderWindow.hpp>

#include "ecs/ECS.hpp"

class RenderSystem {
   public:
    static void draw(const std::shared_ptr<ECS::Registry>& registry,
                     const std::shared_ptr<sf::RenderWindow>& window);
};

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_RENDERSYSTEM_HPP_
