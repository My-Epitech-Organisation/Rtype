/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RectangleComponent.hpp
*/

#ifndef R_TYPE_RECTANGLE_HPP
#define R_TYPE_RECTANGLE_HPP
#include <SFML/Graphics/RectangleShape.hpp>
#include <utility>

struct Rectangle {
    std::pair<float, float> size;
    sf::Color mainColor;
    sf::Color hoveredColor;
    sf::Color currentColor = mainColor;
    float outlineThickness = 0;
    sf::Color outlineColor = sf::Color::Black;
    sf::RectangleShape rectangle;

    Rectangle(const std::pair<float, float> &size, const sf::Color &color, const sf::Color &hoveredColor) : size(size), mainColor(color), hoveredColor(hoveredColor) {}
    Rectangle(const Rectangle &other) = default;
};


#endif //R_TYPE_RECTANGLE_HPP