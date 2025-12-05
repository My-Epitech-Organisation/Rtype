/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.cpp
*/

#include "EventSystem.hpp"

#include <iostream>
#include <utility>

#include "../Components/TagComponent.hpp"

namespace rtype::games::rtype::client {

void EventSystem::update(ECS::Registry& registry, float) {
    registry
        .view<::rtype::games::rtype::client::Rectangle,
              ::rtype::games::rtype::client::UserEvent>()
        .each([this](auto _,
                     const ::rtype::games::rtype::client::Rectangle& rect,
                     ::rtype::games::rtype::client::UserEvent& actionType) {
            this->_mouseMoved(actionType, rect);
            this->_mousePressed(actionType, rect);
            this->_mouseReleased(actionType, rect);
        });
}

EventSystem::EventSystem(std::shared_ptr<sf::RenderWindow> window,
                         const sf::Event& event)
    : ASystem("EventSystem"), _event(event), _window(std::move(window)) {}

void EventSystem::_mouseMoved(
    ::rtype::games::rtype::client::UserEvent& actionType,
    const ::rtype::games::rtype::client::Rectangle& rect) const {
    if (const auto& mouseMove = this->_event.getIf<sf::Event::MouseMoved>()) {
        sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
        sf::Vector2f worldPos =
            this->_window->mapPixelToCoords(mouseMove->position);

        if (rectBounds.contains(worldPos)) {
            actionType.isHovered = true;
        } else {
            actionType.isHovered = false;
            actionType.isClicked = false;
        }
    }
}

void EventSystem::_mousePressed(
    ::rtype::games::rtype::client::UserEvent& actionType,
    const ::rtype::games::rtype::client::Rectangle& rect) const {
    if (const auto& mousePress =
            this->_event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
        if (mousePress->button == sf::Mouse::Button::Left) {
            sf::Vector2f worldPos =
                this->_window->mapPixelToCoords(mousePress->position);

            if (rectBounds.contains(worldPos)) actionType.isClicked = true;
        }
    }
}

void EventSystem::_mouseReleased(
    ::rtype::games::rtype::client::UserEvent& actionType,
    const ::rtype::games::rtype::client::Rectangle& rect) const {
    if (const auto& mouseRelease =
            this->_event.getIf<sf::Event::MouseButtonReleased>()) {
        sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
        if (mouseRelease->button == sf::Mouse::Button::Left) {
            sf::Vector2f worldPos =
                this->_window->mapPixelToCoords(mouseRelease->position);

            if (rectBounds.contains(worldPos) && actionType.isClicked)
                actionType.isReleased = true;
            actionType.isClicked = false;
        }
    }
}
}  // namespace rtype::games::rtype::client
