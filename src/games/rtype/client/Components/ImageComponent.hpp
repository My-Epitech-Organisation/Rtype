/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ImageComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_IMAGECOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_IMAGECOMPONENT_HPP_

#include <utility>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

struct Image {
    sf::Texture textureId;
    sf::Sprite sprite;

    explicit Image(const sf::Texture& texture)
        : textureId(texture), sprite(texture) {}
    Image(const Image& other) = default;
};

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_IMAGECOMPONENT_HPP_
