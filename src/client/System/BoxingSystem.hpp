/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingSystem.hpp
*/

#ifndef SRC_CLIENT_SYSTEM_BOXINGSYSTEM_HPP_
#define SRC_CLIENT_SYSTEM_BOXINGSYSTEM_HPP_

#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>

#include "ecs/ECS.hpp"

class BoxingSystem {
   public:
    static void update(const std::shared_ptr<ECS::Registry>& registry,
                       sf::RenderWindow& window);
};

#endif  // SRC_CLIENT_SYSTEM_BOXINGSYSTEM_HPP_
