/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextureRectComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTURERECTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTURERECTCOMPONENT_HPP_
#include <utility>

#include <SFML/Graphics/Rect.hpp>

struct TextureRect {
   public:
    sf::IntRect rect;
    TextureRect(std::pair<int, int> position, std::pair<int, int> size)
        : rect({position.first, position.second}, {size.first, size.second}) {}
};

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTURERECTCOMPONENT_HPP_
