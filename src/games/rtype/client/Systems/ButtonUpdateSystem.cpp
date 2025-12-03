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

void ButtonUpdateSystem::update(
    const std::shared_ptr<ECS::Registry>& registry,
    const std::shared_ptr<sf::RenderWindow>& window) {
    registry->view<Button<>, UserEvent>().each(
        [](auto _, auto& buttonAct, auto& actionType) {
            if (actionType.isClicked) {
                try {
                    buttonAct.callback();
                } catch (SceneNotFound& e) {
                    std::cerr << "Error executing button callback: " << e.what()
                              << std::endl;
                }
            }
        });
    registry->view<Rectangle, UserEvent, ButtonTag>().each(
        [](auto _, auto& rect, auto& actionType, auto __) {
            if (actionType.isHovered) {
                rect.currentColor = rect.hoveredColor;
            } else {
                rect.currentColor = rect.mainColor;
            }
        });
}
