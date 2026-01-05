/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ColorTintComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_COLORTINTCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_COLORTINTCOMPONENT_HPP_

#include <cstdint>

namespace rtype::games::rtype::client {

/**
 * @brief Color tint component for applying color filters to sprites.
 *
 * Used to distinguish enemy types, power-up states, etc.
 */
struct ColorTint {
    std::uint8_t r{255};
    std::uint8_t g{255};
    std::uint8_t b{255};
    std::uint8_t a{255};

    ColorTint() = default;
    ColorTint(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
              std::uint8_t alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_COLORTINTCOMPONENT_HPP_
