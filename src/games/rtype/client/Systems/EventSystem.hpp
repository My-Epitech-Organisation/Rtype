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
#include "ASystem.hpp"
#include "ecs/ECS.hpp"

namespace rtype::games::rtype::client {
class EventSystem : public ::rtype::engine::ASystem {
   private:
    const sf::Event& _event;
    std::shared_ptr<sf::RenderWindow> _window;

    void _mouseMoved(
        ::rtype::games::rtype::client::UserEvent& actionType,
        const ::rtype::games::rtype::client::Rectangle& rect) const;
    void _mousePressed(
        ::rtype::games::rtype::client::UserEvent& actionType,
        const ::rtype::games::rtype::client::Rectangle& rect) const;
    void _mouseReleased(
        ::rtype::games::rtype::client::UserEvent& actionType,
        const ::rtype::games::rtype::client::Rectangle& rect) const;

   public:
    void update(ECS::Registry& registry, float) override;
    EventSystem(std::shared_ptr<sf::RenderWindow> window,
                const sf::Event& event);
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_SYSTEMS_EVENTSYSTEM_HPP_
