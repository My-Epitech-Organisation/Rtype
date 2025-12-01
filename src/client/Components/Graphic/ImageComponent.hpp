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
    std::string textureId;
    sf::IntRect textureRect;
    Image(
        const std::string &textureId,
        const sf::IntRect &rect
    ) : textureId(textureId) {
        this->textureRect = rect;
    }
};


#endif //R_TYPE_IMAGE_HPP