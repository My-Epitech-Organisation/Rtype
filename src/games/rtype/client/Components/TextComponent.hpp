/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTCOMPONENT_HPP_

#include <string>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

namespace rtype::games::rtype::client {
struct Text {
    sf::Font font;
    sf::Text text;
    std::string textContent;
    sf::Color color;
    unsigned int size;

    Text(const sf::Font& font, const sf::Color& color, unsigned int size = 30,
         const std::string& textContent = "")
        : font(font),
          text(font, textContent),
          textContent(textContent),
          color(color),
          size(size) {}
    Text(const Text& other) = default;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTCOMPONENT_HPP_
