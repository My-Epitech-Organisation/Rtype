/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** RectangleComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_RECTANGLECOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_RECTANGLECOMPONENT_HPP_

#include <utility>

#include <SFML/Graphics/RectangleShape.hpp>

namespace rtype::games::rtype::client {

/**
 * @brief Rectangle component for rendering rectangular shapes.
 *
 * Used for UI elements like buttons, panels, and backgrounds.
 * Supports hover state with different colors.
 */
struct Rectangle {
    std::pair<float, float> size;        ///< Width and height of the rectangle
    sf::Color mainColor;                 ///< Default color when not hovered
    sf::Color hoveredColor;              ///< Color when mouse is hovering
    sf::Color currentColor = mainColor;  ///< Current display color
    float outlineThickness = 0;          ///< Border thickness (0 = no border)
    sf::Color outlineColor = sf::Color::Black;  ///< Border color
    sf::RectangleShape rectangle;               ///< SFML shape for rendering

    /**
     * @brief Construct a new Rectangle component.
     * @param size Width and height as a pair
     * @param color Default fill color
     * @param hoveredColor Color when hovered
     */
    Rectangle(const std::pair<float, float>& size, const sf::Color& color,
              const sf::Color& hoveredColor)
        : size(size), mainColor(color), hoveredColor(hoveredColor) {}

    Rectangle(const Rectangle& other) = default;
    Rectangle(Rectangle&& other) noexcept = default;
    Rectangle& operator=(const Rectangle& other) = default;
    Rectangle& operator=(Rectangle&& other) noexcept = default;
    ~Rectangle() = default;
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_RECTANGLECOMPONENT_HPP_
