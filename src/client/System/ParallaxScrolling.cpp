/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ParallaxScrolling.cpp
*/

#include "ParallaxScrolling.hpp"

#include "Graphic/ImageComponent.hpp"
#include "Graphic/ParallaxComponent.hpp"

void ParallaxScrolling::update(const std::shared_ptr<ECS::Registry>& registry,
                               sf::View view) {
    registry->view<Parallax, Image>().each(
        [&view](auto _, auto& parallax, auto& spriteData) {
            float effectiveOffset = view.getCenter().x * parallax.scrollFactor;

            spriteData.sprite.setPosition(
                {view.getCenter().x - view.getSize().x / 2.f, 0.f});

            sf::IntRect newRect(
                {static_cast<int>(effectiveOffset), 0},
                {static_cast<int>(view.getSize().x),
                 static_cast<int>(spriteData.sprite.getTexture().getSize().y)});

            spriteData.sprite.setTextureRect(newRect);
        });
}
