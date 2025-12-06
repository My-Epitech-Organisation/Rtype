/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ParallaxScrolling.cpp
*/

#include "ParallaxScrolling.hpp"

#include "../Components/ImageComponent.hpp"
#include "../Components/ParallaxComponent.hpp"

namespace rtype::games::rtype::client {
ParallaxScrolling::ParallaxScrolling(std::shared_ptr<sf::View> view)
    : ::rtype::engine::ASystem("ParallaxScrolling"), _view(std::move(view)) {}

void ParallaxScrolling::update(ECS::Registry& registry, float dt) {
    registry
        .view<::rtype::games::rtype::client::Parallax,
              ::rtype::games::rtype::client::Image>()
        .each([this](auto _, auto& parallax, auto& spriteData) {
            float effectiveOffset =
                this->_view->getCenter().x * parallax.scrollFactor;

            int intOffset = static_cast<int>(effectiveOffset);
            spriteData.sprite.setPosition(
                {this->_view->getCenter().x - this->_view->getSize().x / 2.f,
                 this->_view->getCenter().y - this->_view->getSize().y / 2.f});

            sf::IntRect newRect(
                {intOffset, 0},
                {static_cast<int>(this->_view->getSize().x) + 1,
                 static_cast<int>(spriteData.sprite.getTexture().getSize().y)});

            spriteData.sprite.setTextureRect(newRect);
        });
}
}  // namespace rtype::games::rtype::client
