/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ButtonUpdateSystem.cpp
*/

#include "ButtonUpdateSystem.hpp"

#include <exception>
#include <iostream>

#include "../Components/ButtonComponent.hpp"
#include "../Components/RectangleComponent.hpp"
#include "../Components/TagComponent.hpp"
#include "../Components/UserEventComponent.hpp"
#include "SceneManager/SceneException.hpp"

ButtonUpdateSystem::ButtonUpdateSystem(std::shared_ptr<sf::RenderWindow> window)
    : rtype::engine::ASystem("ButtonUpdateSystem"),
      _window(std::move(window)) {}

void ButtonUpdateSystem::update(ECS::Registry& registry, float dt) {
    registry
        .view<rtype::games::rtype::client::Button<>,
              rtype::games::rtype::client::UserEvent>()
        .each([](auto _, auto& buttonAct, auto& actionType) {
            if (actionType.isClicked) {
                try {
                    buttonAct.callback();
                } catch (SceneNotFound& e) {
                    std::cerr << "Error executing button callback: " << e.what()
                              << std::endl;
                }
            }
        });
    registry
        .view<rtype::games::rtype::client::Rectangle,
              rtype::games::rtype::client::UserEvent,
              rtype::games::rtype::client::ButtonTag>()
        .each([](auto _, auto& rect, auto& actionType, auto __) {
            if (actionType.isHovered) {
                rect.currentColor = rect.hoveredColor;
            } else {
                rect.currentColor = rect.mainColor;
            }
        });
}
