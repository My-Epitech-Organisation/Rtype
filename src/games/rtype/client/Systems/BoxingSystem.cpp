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
#include "../shared/Components/PositionComponent.hpp"
#include "Graphic/Accessibility.hpp"

namespace rtype::games::rtype::client {

BoxingSystem::BoxingSystem(std::shared_ptr<sf::RenderTarget> target)
    : ::rtype::engine::ASystem("BoxingSystem"), _target(std::move(target)) {}

void BoxingSystem::update(ECS::Registry& registry, float dt) {
    if (registry.hasSingleton<AccessibilitySettings>()) {
        const auto& acc = registry.getSingleton<AccessibilitySettings>();
        if (!acc.showHitboxes) {
            return;
        }
    } else {
        return;
    }

    registry.view<::rtype::games::rtype::shared::Position,
                  ::rtype::games::rtype::shared::BoundingBoxComponent>()
        .each([this](auto /*entt*/, const auto& pos, const auto& bbox) {
            sf::RectangleShape hitbox;
            hitbox.setSize({bbox.width, bbox.height});
            hitbox.setOrigin({bbox.width / 2.0F, bbox.height / 2.0F});
            hitbox.setPosition({pos.x, pos.y});
            hitbox.setFillColor(sf::Color(255, 0, 0, 30));
            hitbox.setOutlineColor(sf::Color(255, 0, 0, 180));
            hitbox.setOutlineThickness(2.0F);
            this->_target->draw(hitbox);
        });

    registry
        .view<::rtype::games::rtype::client::Image,
              ::rtype::games::rtype::client::BoxingComponent>()
        .each([this](ECS::Entity _,
                     const ::rtype::games::rtype::client::Image& img,
                     ::rtype::games::rtype::client::BoxingComponent& box) {
            sf::FloatRect bounds = img.sprite.getGlobalBounds();

            box.box.setSize({bounds.size.x, bounds.size.y});
            box.box.setPosition({bounds.position.x, bounds.position.y});

            box.box.setFillColor(box.fillColor);
            box.box.setOutlineColor(box.outlineColor);
            box.box.setOutlineThickness(box.outlineThickness);

            this->_target->draw(box.box);
        });
}
}  // namespace rtype::games::rtype::client
