/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.hpp
*/

#ifndef SRC_CLIENT_SYSTEM_RENDERSYSTEM_HPP_
#define SRC_CLIENT_SYSTEM_RENDERSYSTEM_HPP_

#include <SFML/Graphics/RenderWindow.hpp>

#include "ecs/ECS.hpp"

class RenderSystem {
   public:
    static void draw(const std::shared_ptr<ECS::Registry>& registry,
                     sf::RenderWindow& window);
};

#endif  // SRC_CLIENT_SYSTEM_RENDERSYSTEM_HPP_
