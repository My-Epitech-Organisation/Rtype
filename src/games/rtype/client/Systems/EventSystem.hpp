/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_SYSTEMS_EVENTSYSTEM_HPP_
#define SRC_GAMES_RTYPE_CLIENT_SYSTEMS_EVENTSYSTEM_HPP_
#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "../Components/RectangleComponent.hpp"
#include "../Components/UserEventComponent.hpp"
#include "ecs/ECS.hpp"

class EventSystem {
   private:
   public:
    static void processEvents(std::shared_ptr<ECS::Registry> registry,
                              const sf::Event& e,
                              std::shared_ptr<sf::RenderWindow> window);
    static void mouseMoved(const sf::Event& e,
                           std::shared_ptr<sf::RenderWindow> window,
                           UserEvent& actionType, const Rectangle& rect);
    static void mousePressed(const sf::Event& e,
                             std::shared_ptr<sf::RenderWindow> window,
                             UserEvent& actionType, const Rectangle& rect);
    static void mouseReleased(const sf::Event& e,
                              std::shared_ptr<sf::RenderWindow> window,
                              UserEvent& actionType, const Rectangle& rect);
};

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_EVENTSYSTEM_HPP_
