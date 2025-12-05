/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ResetTriggersSystem.cpp
*/

#include "ResetTriggersSystem.hpp"

#include "../Components/UserEventComponent.hpp"

void ResetTriggersSystem::update(std::shared_ptr<ECS::Registry> registry) {
    registry->view<UserEvent>().each(
        [](auto _, UserEvent& event) { event.isReleased = false; });
}
