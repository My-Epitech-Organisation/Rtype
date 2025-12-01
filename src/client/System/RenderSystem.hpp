/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RenderSystem.hpp
*/

#ifndef R_TYPE_RENDERSYSTEM_HPP
#define R_TYPE_RENDERSYSTEM_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include "ecs/ECS.hpp"

class RenderSystem {
public:
    static void draw(const std::shared_ptr<ECS::Registry> &registry, sf::RenderWindow &window);
};


#endif //R_TYPE_RENDERSYSTEM_HPP