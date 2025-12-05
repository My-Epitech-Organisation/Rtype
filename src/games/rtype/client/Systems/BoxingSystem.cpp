/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingSystem.cpp
*/

#include "BoxingSystem.hpp"

#include <memory>

#include "../Components/BoxingComponent.hpp"
#include "../Components/ImageComponent.hpp"

BoxingSystem::BoxingSystem(std::shared_ptr<sf::RenderWindow> window)
    : rtype::engine::ASystem("BoxingSystem"), _window(std::move(window)) {}

void BoxingSystem::update(ECS::Registry& registry, float dt) {
    registry
        .view<rtype::games::rtype::client::Image,
              rtype::games::rtype::client::BoxingComponent>()
        .each([this](ECS::Entity _,
                     const rtype::games::rtype::client::Image& img,
                     rtype::games::rtype::client::BoxingComponent& box) {
            sf::FloatRect bounds = img.sprite.getGlobalBounds();

            box.box.setSize({bounds.size.x, bounds.size.y});
            box.box.setPosition({bounds.position.x, bounds.position.y});

            box.box.setFillColor(sf::Color::Transparent);
            box.box.setOutlineColor(sf::Color::Red);
            box.box.setOutlineThickness(1.f);

            this->_window->draw(box.box);
        });
}
