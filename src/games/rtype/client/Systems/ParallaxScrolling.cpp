/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ParallaxScrolling.cpp
*/

#include "ParallaxScrolling.hpp"

#include "../Components/ImageComponent.hpp"
#include "../Components/ParallaxComponent.hpp"

void ParallaxScrolling::update(std::shared_ptr<ECS::Registry> registry,
                               const sf::View& view) {
    registry->view<Parallax, Image>().each(
        [&view](auto _, auto& parallax, auto& spriteData) {
            float effectiveOffset = view.getCenter().x * parallax.scrollFactor;
            int intOffset = static_cast<int>(effectiveOffset);

            spriteData.sprite.setPosition(
                {view.getCenter().x - view.getSize().x / 2.f,
                 view.getCenter().y - view.getSize().y / 2.f});

            sf::IntRect newRect(
                {intOffset, 0},
                {static_cast<int>(view.getSize().x) + 1,
                 static_cast<int>(spriteData.sprite.getTexture().getSize().y)});

            spriteData.sprite.setTextureRect(newRect);
        });
}
