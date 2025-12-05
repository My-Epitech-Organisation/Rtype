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
    sf::Color color = sf::Color::Red;

    explicit BoxingComponent(sf::FloatRect bounds) : bounds(bounds) {
        this->box = sf::RectangleShape();
        this->box.setSize({bounds.size.x, bounds.size.y});
        this->box.setPosition({bounds.position.x, bounds.position.y});
        this->box.setFillColor(sf::Color::Transparent);
        this->box.setOutlineColor(this->color);
        this->box.setOutlineThickness(1.f);
    }
};
}  // namespace rtype::games::rtype::client
#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_BOXINGCOMPONENT_HPP_
