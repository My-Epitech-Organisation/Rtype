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

#include "../../../../../include/rtype/display/DisplayTypes.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Text component for rendering text on screen.
 *
 * Used for labels, button text, titles, and any displayed text.
 * Stores the font name and the raw string content.
 */
struct Text {
    std::string fontName;
    std::string textContent;
    display::Color color;
    unsigned int size;

    /**
     * @brief Construct a new Text component.
     * @param fontName Name of the font in the asset manager
     * @param color Text color
     * @param size Font size in pixels (default: 30)
     * @param textContent Initial text string (default: empty)
     */
    Text(std::string fName, const display::Color& col, unsigned int sz = 30,
         std::string_view content = "")
        : fontName(std::move(fName)),
          textContent(content),
          color(col),
          size(sz) {}

    Text(const Text& other) = default;
    Text(Text&& other) noexcept = default;
    Text& operator=(const Text& other) = default;
    Text& operator=(Text&& other) noexcept = default;
    ~Text() = default;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_TEXTCOMPONENT_HPP_
