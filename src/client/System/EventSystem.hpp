/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.hpp
*/

#ifndef SRC_CLIENT_SYSTEM_EVENTSYSTEM_HPP_
#define SRC_CLIENT_SYSTEM_EVENTSYSTEM_HPP_
#include <memory>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "ecs/ECS.hpp"

class EventSystem {
   public:
    static void processEvents(const std::shared_ptr<ECS::Registry>& registry,
                              const sf::Event& e, sf::RenderWindow& window);
};

#endif  // SRC_CLIENT_SYSTEM_EVENTSYSTEM_HPP_
