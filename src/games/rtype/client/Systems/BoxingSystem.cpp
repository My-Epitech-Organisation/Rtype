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
#include "../Components/RotationComponent.hpp"
#include "../Components/SizeComponent.hpp"
#include "../Components/TextureRectComponent.hpp"
#include "../shared/Components/BoundingBoxComponent.hpp"
#include "Graphic/Accessibility.hpp"
#include "Logger/Macros.hpp"
#include "games/rtype/shared/Components/TransformComponent.hpp"

namespace rtype::games::rtype::client {

BoxingSystem::BoxingSystem(std::shared_ptr<::rtype::display::IDisplay> display)
    : ::rtype::engine::ASystem("BoxingSystem"), _display(std::move(display)) {}

void BoxingSystem::update(ECS::Registry& registry, float dt) {
    if (!registry.hasSingleton<AccessibilitySettings>()) {
        return;
    }

    const auto& acc = registry.getSingleton<AccessibilitySettings>();
    if (!acc.showHitboxes) {
        return;
    }

    std::size_t entityCount = 0;
    registry
        .view<::rtype::games::rtype::shared::TransformComponent,
              ::rtype::games::rtype::shared::BoundingBoxComponent>()
        .each([this, &entityCount](auto entity, const auto& transform,
                                   const auto& bbox) {
            entityCount++;
            ::rtype::display::Vector2f position;
            position.x = transform.x - bbox.width / 2.0f;
            position.y = transform.y - bbox.height / 2.0f;
            ::rtype::display::Vector2f size = {bbox.width, bbox.height};
            ::rtype::display::Color fillColor = {255, 0, 0, 80};
            ::rtype::display::Color outlineColor = {255, 0, 0, 255};

            this->_display->drawRectangle(position, size, fillColor,
                                          outlineColor, 2.0f);
        });

    static int logCounter = 0;
    if (logCounter++ % 60 == 0) {
        LOG_INFO("[BoxingSystem] Drawing hitboxes for " << entityCount
                                                        << " entities");
    }

    registry
        .view<::rtype::games::rtype::client::Image,
              ::rtype::games::rtype::client::BoxingComponent>()
        .each([this](ECS::Entity _,
                     const ::rtype::games::rtype::client::Image& img,
                     ::rtype::games::rtype::client::BoxingComponent& box) {
            this->_display->drawRectangle(box.position, box.size, box.fillColor,
                                          box.outlineColor,
                                          box.outlineThickness);
        });
}
}  // namespace rtype::games::rtype::client
