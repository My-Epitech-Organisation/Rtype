/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RectangleComponent.hpp
*/

#ifndef SRC_CLIENT_COMPONENTS_GRAPHIC_RECTANGLECOMPONENT_HPP_
#define SRC_CLIENT_COMPONENTS_GRAPHIC_RECTANGLECOMPONENT_HPP_
#include <utility>

#include <SFML/Graphics/RectangleShape.hpp>

struct Rectangle {
    std::pair<float, float> size;
    sf::Color mainColor;
    sf::Color hoveredColor;
    sf::Color currentColor = mainColor;
    float outlineThickness = 0;
    sf::Color outlineColor = sf::Color::Black;
    sf::RectangleShape rectangle;

    Rectangle(const std::pair<float, float>& size, const sf::Color& color,
              const sf::Color& hoveredColor)
        : size(size), mainColor(color), hoveredColor(hoveredColor) {}
    Rectangle(const Rectangle& other) = default;
};

#endif  // SRC_CLIENT_COMPONENTS_GRAPHIC_RECTANGLECOMPONENT_HPP_
