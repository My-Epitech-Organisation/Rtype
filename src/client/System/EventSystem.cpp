/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.cpp
*/

#include "EventSystem.hpp"

#include <iostream>

#include "Graphic/RectangleComponent.hpp"
#include "Graphic/TagComponent.hpp"
#include "Graphic/UserEventComponent.hpp"

void EventSystem::processEvents(const std::shared_ptr<ECS::Registry>& registry,
                                const sf::Event& e, sf::RenderWindow& window) {
    registry->view<Rectangle, UserEvent>().each([e, &window](auto _, auto rect,
                                                             auto& actionType) {
        if (e.is<sf::Event::MouseMoved>()) {
            sf::Vector2i mousePos = e.getIf<sf::Event::MouseMoved>()->position;
            sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
            sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
            if (rectBounds.contains(worldPos)) {
                actionType.isHovered = true;
            } else {
                actionType.isHovered = false;
            }
        }
        if (e.is<sf::Event::MouseButtonPressed>()) {
            if (e.getIf<sf::Event::MouseButtonPressed>()->button ==
                sf::Mouse::Button::Left) {
                sf::Vector2i mousePos =
                    e.getIf<sf::Event::MouseButtonPressed>()->position;
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
                sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
                if (rectBounds.contains(worldPos)) {
                    actionType.isClicked = true;
                }
            }
        }
        if (e.is<sf::Event::MouseButtonReleased>()) {
            if (e.getIf<sf::Event::MouseButtonReleased>()->button ==
                sf::Mouse::Button::Left) {
                sf::Vector2i mousePos =
                    e.getIf<sf::Event::MouseButtonReleased>()->position;
                sf::Vector2f worldPos = window.mapPixelToCoords(mousePos);
                sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
                if (rectBounds.contains(worldPos)) {
                    actionType.isClicked = false;
                }
            }
        }
    });
}
