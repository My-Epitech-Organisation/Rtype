/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.hpp
*/

#ifndef SRC_CLIENT_SYSTEM_EVENTSYSTEM_HPP_
#define SRC_CLIENT_SYSTEM_EVENTSYSTEM_HPP_
#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

#include "Graphic/RectangleComponent.hpp"
#include "Graphic/UserEventComponent.hpp"
#include "ecs/ECS.hpp"

class EventSystem {
   private:
   public:
    static void processEvents(const std::shared_ptr<ECS::Registry>& registry,
                              const sf::Event& e,
                              const std::shared_ptr<sf::RenderWindow>& window);
    static void mouseMoved(const sf::Event& e,
                           const std::shared_ptr<sf::RenderWindow>& window,
                           UserEvent& actionType, const Rectangle& rect);
    static void mousePressed(const sf::Event& e,
                             const std::shared_ptr<sf::RenderWindow>& window,
                             UserEvent& actionType, const Rectangle& rect);
    static void mouseReleased(const sf::Event& e,
                              const std::shared_ptr<sf::RenderWindow>& window,
                              UserEvent& actionType, const Rectangle& rect);
};

#endif  // SRC_CLIENT_SYSTEM_EVENTSYSTEM_HPP_
