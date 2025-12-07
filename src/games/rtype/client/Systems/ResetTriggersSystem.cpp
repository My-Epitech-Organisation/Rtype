/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ResetTriggersSystem.cpp
*/

#include "ResetTriggersSystem.hpp"

#include "../Components/UserEventComponent.hpp"

namespace rtype::games::rtype::client {

ResetTriggersSystem::ResetTriggersSystem()
    : ::rtype::engine::ASystem("ResetTriggersSystem") {}

void ResetTriggersSystem::update(ECS::Registry& registry, float /*dt*/) {
    registry.view<UserEvent>().each(
        [](auto /*entity*/, UserEvent& event) { event.isReleased = false; });
}

}  // namespace rtype::games::rtype::client
