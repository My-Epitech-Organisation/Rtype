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

namespace rc = ::rtype::games::rtype::client;

ParallaxScrolling::ParallaxScrolling(std::shared_ptr<sf::View> view)
    : ::rtype::engine::ASystem("ParallaxScrolling"), _view(std::move(view)) {}

void ParallaxScrolling::_updateCache() {
    if (!_cacheValid) {
        sf::Vector2f size = _view->getSize();
        _cachedHalfWidth = size.x / 2.0f;
        _cachedHalfHeight = size.y / 2.0f;
        _cacheValid = true;
    }
}

void ParallaxScrolling::update(ECS::Registry& registry, float /*dt*/) {
    _updateCache();

    const sf::Vector2f center = _view->getCenter();
    const float viewWidth = _cachedHalfWidth * 2.0f;
    const float spriteX = center.x - _cachedHalfWidth;
    const float spriteY = center.y - _cachedHalfHeight;

    registry.view<rc::Parallax, rc::Image>().each(
        [this, &center, viewWidth, spriteX, spriteY](
            auto /*entity*/, const auto& parallax, auto& spriteData) {
            float effectiveOffset = center.x * parallax.scrollFactor;
            int intOffset = static_cast<int>(effectiveOffset);

            spriteData.sprite.setPosition({spriteX, spriteY});

            sf::IntRect newRect(
                {intOffset, 0},
                {static_cast<int>(viewWidth) + 1,
                 static_cast<int>(spriteData.sprite.getTexture().getSize().y)});

            spriteData.sprite.setTextureRect(newRect);
        });
}

}  // namespace rtype::games::rtype::client
