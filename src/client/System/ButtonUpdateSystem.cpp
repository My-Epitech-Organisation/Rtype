/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ButtonUpdateSystem.cpp
*/

#include <iostream>
#include <exception>
#include "ButtonUpdateSystem.hpp"
#include "Graphic/UserEventComponent.hpp"
#include "Graphic/ButtonComponent.hpp"
#include "Graphic/RectangleComponent.hpp"
#include "Graphic/TagComponent.hpp"
#include "SceneManager/SceneException.hpp"

void ButtonUpdateSystem::update(const std::shared_ptr<ECS::Registry> &registry, sf::RenderWindow &window) {
    registry->view<Button<>, UserEvent>().each([](auto _, auto &buttonAct, auto &actionType) {
        if (actionType.isClicked) {
            try {
                buttonAct.callback();
            } catch (SceneNotFound &e) {
                std::cerr << "Error executing button callback: " << e.what() << std::endl;
            }
        }
    });
    registry->view<Rectangle, UserEvent, ButtonTag>().each([](auto _, auto &rect, auto &actionType, auto __) {
        if (actionType.isHovered) {
            rect.currentColor = rect.hoveredColor;
        } else {
            rect.currentColor = rect.mainColor;
        }
    });
}
