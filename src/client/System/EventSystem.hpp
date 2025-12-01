/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.hpp
*/

#ifndef R_TYPE_EVENTSYSTEM_HPP
#define R_TYPE_EVENTSYSTEM_HPP
#include <memory>
#include <SFML/Window/Event.hpp>
#include "ecs/ECS.hpp"

class EventSystem {
public:
    static void processEvents(const std::shared_ptr<ECS::Registry> &registry, const sf::Event &e);
};


#endif //R_TYPE_EVENTSYSTEM_HPP