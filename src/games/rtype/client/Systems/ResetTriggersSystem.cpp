/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ResetTriggersSystem.cpp
*/

#include "ResetTriggersSystem.hpp"

#include "../Components/UserEventComponent.hpp"

ResetTriggersSystem::ResetTriggersSystem()
    : rtype::engine::ASystem("ResetTriggersSystem") {}

void ResetTriggersSystem::update(ECS::Registry& registry, float dt) {
    registry.view<rtype::games::rtype::client::UserEvent>().each(
        [](auto _, rtype::games::rtype::client::UserEvent& event) {
            event.isReleased = false;
        });
}
