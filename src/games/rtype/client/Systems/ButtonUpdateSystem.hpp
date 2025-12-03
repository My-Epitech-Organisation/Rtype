/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** UpdateSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BUTTONUPDATESYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BUTTONUPDATESYSTEM_HPP_
#include <SFML/Graphics/RenderWindow.hpp>

#include "ecs/core/Registry/Registry.hpp"

class ButtonUpdateSystem {
   public:
    static void update(const std::shared_ptr<ECS::Registry>& registry,
                       const std::shared_ptr<sf::RenderWindow>& window);
};

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_BUTTONUPDATESYSTEM_HPP_
