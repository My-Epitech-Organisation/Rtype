/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.cpp
*/

#include "EventSystem.hpp"

#include "Graphic/RectangleComponent.hpp"
#include "Graphic/TagComponent.hpp"
#include "Graphic/UserEventComponent.hpp"
#include <iostream>

void EventSystem::processEvents(const std::shared_ptr<ECS::Registry> &registry, const sf::Event &e)
{
    registry->view<Rectangle, UserEvent>().each([e] (auto _, auto rect, auto &actionType) {
        if (e.is<sf::Event::MouseMoved>()) {
            sf::Vector2i mousePos = e.getIf<sf::Event::MouseMoved>()->position;
            sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
            if (rectBounds.contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                actionType.isHovered = true;
            } else {
                actionType.isHovered = false;
            }
        }
        if (e.is<sf::Event::MouseButtonPressed>()) {
            if (e.getIf<sf::Event::MouseButtonPressed>()->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = e.getIf<sf::Event::MouseButtonPressed>()->position;
                sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
                if (rectBounds.contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                    actionType.isClicked = true;
                }
            }
        }
         if (e.is<sf::Event::MouseButtonReleased>()) {
            if (e.getIf<sf::Event::MouseButtonReleased>()->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePos = e.getIf<sf::Event::MouseButtonReleased>()->position;
                sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
                if (rectBounds.contains({static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)})) {
                    actionType.isClicked = false;
                }
            }
        }
    });
}
