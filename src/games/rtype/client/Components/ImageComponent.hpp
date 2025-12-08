/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ImageComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_IMAGECOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_IMAGECOMPONENT_HPP_

#include <utility>

#include <SFML/Graphics.hpp>

namespace rtype::games::rtype::client {

/**
 * @brief Image component storing a sprite with a reference to a texture.
 *
 * The texture is NOT copied - it stores a reference to the original texture
 * managed by TextureManager. This avoids expensive texture copies.
 *
 * @warning The texture must outlive the Image component.
 */
struct Image {
    sf::Sprite sprite;

    /**
     * @brief Construct an Image component with a texture reference.
     * @param texture Reference to a texture (must outlive this component)
     */
    explicit Image(const sf::Texture& texture) : sprite(texture) {}

    // Default copy/move - sprite handles texture reference correctly
    Image(const Image& other) = default;
    Image(Image&& other) noexcept = default;
    Image& operator=(const Image& other) = default;
    Image& operator=(Image&& other) noexcept = default;
    ~Image() = default;
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_IMAGECOMPONENT_HPP_
