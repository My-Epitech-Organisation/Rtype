/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ImageComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_IMAGECOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_IMAGECOMPONENT_HPP_

#include <utility>
#include <string>
#include "../../../../../include/rtype/display/DisplayTypes.hpp"

namespace rtype::games::rtype::client {

/**
 * @brief Image component storing a texture name and rendering properties.
 */
struct Image {
    std::string textureName;
    ::rtype::display::Color color = ::rtype::display::Color::White();

    /**
     * @brief Construct an Image component with a texture name.
     * @param name Name of the texture in the asset manager
     */
    explicit Image(std::string name) : textureName(std::move(name)) {}

    Image(const Image& other) = default;
    Image(Image&& other) noexcept = default;
    Image& operator=(const Image& other) = default;
    Image& operator=(Image&& other) noexcept = default;
    ~Image() = default;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_IMAGECOMPONENT_HPP_
