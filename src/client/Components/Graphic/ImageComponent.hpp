/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ImageComponent.hpp
*/

#ifndef SRC_CLIENT_COMPONENTS_GRAPHIC_IMAGECOMPONENT_HPP_
#define SRC_CLIENT_COMPONENTS_GRAPHIC_IMAGECOMPONENT_HPP_

#include <utility>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

struct Image {
    sf::Texture textureId;
    sf::IntRect textureRect;
    sf::Sprite sprite;

    Image(const sf::Texture& texture, const sf::IntRect& rect)
        : textureId(texture), sprite(texture) {
        this->textureRect = rect;
    }
    Image(const Image& other) = default;
};

#endif  // SRC_CLIENT_COMPONENTS_GRAPHIC_IMAGECOMPONENT_HPP_
