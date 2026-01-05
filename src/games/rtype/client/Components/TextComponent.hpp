/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** TextComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTCOMPONENT_HPP_

#include <string>
#include <string_view>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

namespace rtype::games::rtype::client {

/**
 * @brief Text component for rendering text on screen.
 *
 * Used for labels, button text, titles, and any displayed text.
 * Stores both the SFML text object and the raw string content.
 */
struct Text {
    std::shared_ptr<sf::Font> font;
    sf::Text text;
    std::string textContent;
    sf::Color color;
    unsigned int size;

    /**
     * @brief Construct a new Text component.
     * @param font Reference to the font (must outlive this component)
     * @param color Text color
     * @param size Font size in pixels (default: 30)
     * @param textContent Initial text string (default: empty)
     */
    Text(std::shared_ptr<sf::Font> f, const sf::Color& col,
         unsigned int sz = 30, std::string_view content = "")
        : font(f), text(*f), textContent(content), color(col), size(sz) {
        text.setString(std::string(textContent));
        text.setCharacterSize(size);
        text.setFillColor(color);
    }

    Text(const Text& other)
        : font(other.font),
          text(*font),
          textContent(other.textContent),
          color(other.color),
          size(other.size) {
        text.setString(textContent);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setOrigin(other.text.getOrigin());
    }

    Text(Text&& other) noexcept = default;
    Text& operator=(const Text& other) = default;
    Text& operator=(Text&& other) noexcept = default;
    ~Text() = default;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTCOMPONENT_HPP_
