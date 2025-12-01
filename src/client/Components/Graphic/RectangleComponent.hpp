/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RectangleComponent.hpp
*/

#ifndef R_TYPE_RECTANGLE_HPP
#define R_TYPE_RECTANGLE_HPP
#include <SFML/Graphics/RectangleShape.hpp>


struct Rectangle {
    sf::RectangleShape _rect;
    explicit Rectangle(sf::RectangleShape rect) : _rect(std::move(rect)) {}
};


#endif //R_TYPE_RECTANGLE_HPP