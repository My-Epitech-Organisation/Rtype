/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingSystem.cpp
*/

#include "BoxingSystem.hpp"

#include <memory>
#include <utility>

#include "../Components/BoxingComponent.hpp"
#include "../Components/ImageComponent.hpp"
#include "../shared/Components/BoundingBoxComponent.hpp"
#include "Graphic/Accessibility.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

namespace rtype::games::rtype::client {

BoxingSystem::BoxingSystem(std::shared_ptr<::rtype::display::IDisplay> display)
    : ::rtype::engine::ASystem("BoxingSystem"), _display(std::move(display)) {}

void BoxingSystem::update(ECS::Registry& registry, float dt) {
    if (registry.hasSingleton<AccessibilitySettings>()) {
        const auto& acc = registry.getSingleton<AccessibilitySettings>();
        if (!acc.showHitboxes) {
            return;
        }
    } else {
        return;
    }

    registry
        .view<::rtype::games::rtype::shared::TransformComponent,
              ::rtype::games::rtype::shared::BoundingBoxComponent>()
        .each([this](auto /*entt*/, const auto& pos, const auto& bbox) {
            // Note: IDisplay::drawRectangle might need an origin or we adjust
            // position Original code used setOrigin({bbox.width / 2.0F,
            // bbox.height / 2.0F})
            ::rtype::display::Vector2f position = {pos.x - bbox.width / 2.0f,
                                                   pos.y - bbox.height / 2.0f};
            ::rtype::display::Vector2f size = {bbox.width, bbox.height};
            ::rtype::display::Color fillColor = {255, 0, 0, 30};
            ::rtype::display::Color outlineColor = {255, 0, 0, 180};

            this->_display->drawRectangle(position, size, fillColor,
                                          outlineColor, 2.0f);
        });

    registry
        .view<::rtype::games::rtype::client::Image,
              ::rtype::games::rtype::client::BoxingComponent>()
        .each([this](ECS::Entity _,
                     const ::rtype::games::rtype::client::Image& img,
                     ::rtype::games::rtype::client::BoxingComponent& box) {
            // We don't have img.sprite.getGlobalBounds() anymore.
            // We should probably use the transform or something else.
            // For now, let's just draw the box as it is.
            this->_display->drawRectangle(box.position, box.size, box.fillColor,
                                          box.outlineColor,
                                          box.outlineThickness);
        });
}
}  // namespace rtype::games::rtype::client
