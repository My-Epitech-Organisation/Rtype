/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** EventSystem.cpp
*/

#include "EventSystem.hpp"

#include <iostream>

#include "Graphic/TagComponent.hpp"

void EventSystem::mouseMoved(const sf::Event& e,
                             const std::shared_ptr<sf::RenderWindow>& window,
                             UserEvent& actionType, const Rectangle& rect) {
    if (const auto* mouseMove = e.getIf<sf::Event::MouseMoved>()) {
        sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
        sf::Vector2f worldPos = window->mapPixelToCoords(mouseMove->position);

        if (rectBounds.contains(worldPos)) {
            actionType.isHovered = true;
        } else {
            actionType.isHovered = false;
            actionType.isClicked = false;
        }
    }
}

void EventSystem::mousePressed(const sf::Event& e,
                               const std::shared_ptr<sf::RenderWindow>& window,
                               UserEvent& actionType, const Rectangle& rect) {
    if (const auto* mousePress = e.getIf<sf::Event::MouseButtonPressed>()) {
        sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
        if (mousePress->button == sf::Mouse::Button::Left) {
            sf::Vector2f worldPos =
                window->mapPixelToCoords(mousePress->position);

            if (rectBounds.contains(worldPos)) actionType.isClicked = true;
        }
    }
}

void EventSystem::mouseReleased(const sf::Event& e,
                                const std::shared_ptr<sf::RenderWindow>& window,
                                UserEvent& actionType, const Rectangle& rect) {
    if (const auto* mouseRelease = e.getIf<sf::Event::MouseButtonReleased>()) {
        sf::FloatRect rectBounds = rect.rectangle.getGlobalBounds();
        if (mouseRelease->button == sf::Mouse::Button::Left) {
            sf::Vector2f worldPos =
                window->mapPixelToCoords(mouseRelease->position);

            if (rectBounds.contains(worldPos) && actionType.isClicked)
                actionType.isReleased = true;
            actionType.isClicked = false;
        }
    }
}

void EventSystem::processEvents(
    const std::shared_ptr<ECS::Registry>& registry, const sf::Event& e,
    const std::shared_ptr<sf::RenderWindow>& window) {
    registry->view<Rectangle, UserEvent>().each(
        [&e, &window](auto entity, const Rectangle& rect,
                      UserEvent& actionType) {
            mouseMoved(e, window, actionType, rect);
            mousePressed(e, window, actionType, rect);
            mouseReleased(e, window, actionType, rect);
        });
}
