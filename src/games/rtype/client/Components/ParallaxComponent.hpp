/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ParallaxComponent.hpp
*/

#ifndef SRC_GAMES_RTYPE_CLIENT_COMPONENTS_PARALLAXCOMPONENT_HPP_
#define SRC_GAMES_RTYPE_CLIENT_COMPONENTS_PARALLAXCOMPONENT_HPP_

namespace rtype::games::rtype::client {
struct Parallax {
    float scrollFactor = 0;
    bool isRepeating = true;

    Parallax() = default;
    Parallax(const float& scrollFactor, const bool& isRepeating)
        : scrollFactor(scrollFactor), isRepeating(isRepeating) {}
};
}  // namespace rtype::games::rtype::client

#endif  // SRC_GAMES_RTYPE_CLIENT_COMPONENTS_PARALLAXCOMPONENT_HPP_
