/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOXINGCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOXINGCOMPONENT_HPP_
#include <SFML/Graphics/RectangleShape.hpp>

namespace rtype::games::rtype::client {
struct BoxingComponent {
    sf::RectangleShape box;
    sf::FloatRect bounds;
    sf::Color outlineColor = sf::Color::Red;
    sf::Color fillColor = sf::Color(255, 255, 255, 30);
    float outlineThickness = 2.f;

    explicit BoxingComponent(sf::FloatRect bounds) : bounds(bounds) {
        this->box = sf::RectangleShape();
        this->box.setSize({bounds.size.x, bounds.size.y});
        this->box.setPosition({bounds.position.x, bounds.position.y});
        this->box.setFillColor(this->fillColor);
        this->box.setOutlineColor(this->outlineColor);
        this->box.setOutlineThickness(this->outlineThickness);
    }
};
}  // namespace rtype::games::rtype::client
#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOXINGCOMPONENT_HPP_
