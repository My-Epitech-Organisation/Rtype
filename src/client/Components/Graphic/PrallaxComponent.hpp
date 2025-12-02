/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** PrallaxComponent.hpp
*/

#ifndef SRC_CLIENT_COMPONENTS_GRAPHIC_PRALLAXCOMPONENT_HPP_
#define SRC_CLIENT_COMPONENTS_GRAPHIC_PRALLAXCOMPONENT_HPP_

struct Parallax {
    float scrollFactor = 0;
    bool isRepeating = true;

    Parallax() = default;
    Parallax(const float& scrollFactor, const bool& isRepeating)
        : scrollFactor(scrollFactor), isRepeating(isRepeating) {}
};

#endif  // SRC_CLIENT_COMPONENTS_GRAPHIC_PRALLAXCOMPONENT_HPP_
