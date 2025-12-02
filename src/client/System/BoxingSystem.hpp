/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingSystem.hpp
*/

#ifndef R_TYPE_BOXINGSYSTEM_HPP
#define R_TYPE_BOXINGSYSTEM_HPP

#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>

#include "ecs/ECS.hpp"

class BoxingSystem {
   public:
    static void update(const std::shared_ptr<ECS::Registry>& registry,
                       sf::RenderWindow& window);
};

#endif  // R_TYPE_BOXINGSYSTEM_HPP