/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** BoxingComponent.hpp
*/

#ifndef R_TYPE_BOXINGCOMPONENT_HPP
#define R_TYPE_BOXINGCOMPONENT_HPP
#include <SFML/Graphics/RectangleShape.hpp>

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

#endif  // R_TYPE_BOXINGCOMPONENT_HPP