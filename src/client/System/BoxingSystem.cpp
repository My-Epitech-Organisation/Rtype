/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingSystem.cpp
*/

#include "BoxingSystem.hpp"

#include <memory>

#include "Graphic/BoxingComponent.hpp"
#include "Graphic/ImageComponent.hpp"

void BoxingSystem::draw(const std::shared_ptr<ECS::Registry>& registry,
                        const std::shared_ptr<sf::RenderWindow>& window) {
    registry->view<Image, BoxingComponent>().each(
        [&window](ECS::Entity _, const Image& img, BoxingComponent& box) {
            sf::FloatRect bounds = img.sprite.getGlobalBounds();

            box.box.setSize({bounds.size.x, bounds.size.y});
            box.box.setPosition({bounds.position.x, bounds.position.y});

            box.box.setFillColor(sf::Color::Transparent);
            box.box.setOutlineColor(sf::Color::Red);
            box.box.setOutlineThickness(1.f);

            window->draw(box.box);
        });
}
