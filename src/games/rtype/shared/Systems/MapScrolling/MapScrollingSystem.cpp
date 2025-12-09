/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** MapScrollingSystem - Implementation of map element position updates
*/

#include "MapScrollingSystem.hpp"

#include "Logger/Macros.hpp"

namespace rtype::games::rtype::shared {

MapScrollingSystem::MapScrollingSystem()
    : engine::ASystem("MapScrollingSystem") {}

void MapScrollingSystem::update(ECS::Registry& registry, float /*deltaTime*/) {
    if (m_scrollState == nullptr) {
        return;
    }

    // Update all map elements with scroll component
    registry.view<MapElementTag, ScrollComponent, TransformComponent>().each(
        [this](auto /*entity*/, const auto& /*mapTag*/, const auto& scroll,
               auto& transform) {
            // Convert level X position to screen X position
            transform.x = m_scrollState->levelToScreenX(scroll.initialX);
        });
}

}  // namespace rtype::games::rtype::shared
