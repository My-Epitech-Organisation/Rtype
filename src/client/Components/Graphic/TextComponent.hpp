/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextComponent.hpp
*/

#ifndef R_TYPE_TEXT_HPP
#define R_TYPE_TEXT_HPP

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <utility>

struct Text {
    std::string fontId;

    sf::IntRect textureRect;
    Text(
        const std::string &textureId,
        const sf::IntRect &rect
    ) : textureId(textureId) {
        this->textureRect = rect;
    }
};


#endif //R_TYPE_TEXT_HPP