/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextComponent.hpp
*/

#ifndef R_TYPE_TEXT_HPP
#define R_TYPE_TEXT_HPP


#include <string>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

struct Text {
    sf::Font font;
    sf::Text text;
    std::string textContent;
    sf::Color color;
    unsigned int size;

    Text(
        const sf::Font &font,
        const sf::Color &color,
        unsigned int size = 30,
        const std::string &textContent = ""
    ) : font(font), text(font, textContent), textContent(textContent), color(color), size(size) {}
    Text(const Text &other) = default;
};


#endif //R_TYPE_TEXT_HPP