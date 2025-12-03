/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ParallaxComponent.hpp
*/

#ifndef SRC_CLIENT_COMPONENTS_GRAPHIC_PARALLAXCOMPONENT_HPP_
#define SRC_CLIENT_COMPONENTS_GRAPHIC_PARALLAXCOMPONENT_HPP_
struct Parallax {
    float scrollFactor = 0;
    bool isRepeating = true;

    Parallax() = default;
    Parallax(const float& scrollFactor, const bool& isRepeating)
        : scrollFactor(scrollFactor), isRepeating(isRepeating) {}
};

#endif  // SRC_CLIENT_COMPONENTS_GRAPHIC_PARALLAXCOMPONENT_HPP_
