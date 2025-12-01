/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ImageComponent.hpp
*/

#ifndef R_TYPE_IMAGE_HPP
#define R_TYPE_IMAGE_HPP

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <utility>

struct Image {
    sf::Texture textureId;
    sf::IntRect textureRect;
    sf::Sprite sprite;

    Image(
        const sf::Texture &texture,
        const sf::IntRect &rect
    ) : textureId(texture), sprite(texture) {
        this->textureRect = rect;
    }
    Image(const Image &other) = default;
};


#endif //R_TYPE_IMAGE_HPP