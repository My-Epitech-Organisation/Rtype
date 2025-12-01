/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** UpdateSystem.hpp
*/

#ifndef R_TYPE_UPDATESYSTEM_HPP
#define R_TYPE_UPDATESYSTEM_HPP
#include <SFML/Graphics/RenderWindow.hpp>
#include "ecs/core/Registry/Registry.hpp"


class ButtonUpdateSystem {
public:
    static void update(const std::shared_ptr<ECS::Registry> &registry, sf::RenderWindow &window);
};


#endif //R_TYPE_UPDATESYSTEM_HPP