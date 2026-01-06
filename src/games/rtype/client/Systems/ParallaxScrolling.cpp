/*
** EPITECH PROJECT, 2025
** r-type
** File description:
** ParallaxScrolling.cpp
*/

#include "ParallaxScrolling.hpp"

#include "../Components/ImageComponent.hpp"
#include "../Components/ParallaxComponent.hpp"
#include "../Components/TextureRectComponent.hpp"
#include "../GraphicsConstants.hpp"
#include "../shared/Components/TransformComponent.hpp"

namespace rtype::games::rtype::client {

namespace rc = ::rtype::games::rtype::client;
namespace rs = ::rtype::games::rtype::shared;

ParallaxScrolling::ParallaxScrolling(
    std::shared_ptr<::rtype::display::IDisplay> display)
    : ::rtype::engine::ASystem("ParallaxScrolling"),
      _display(std::move(display)) {}

void ParallaxScrolling::_updateCache() {
    if (!_cacheValid) {
        display::Vector2f viewSize = _display->getViewSize();
        _cachedHalfWidth = viewSize.x / 2.0f;
        _cachedHalfHeight = viewSize.y / 2.0f;
        _cacheValid = true;
    }
}

void ParallaxScrolling::update(ECS::Registry& registry, float dt) {
    _updateCache();
    _totalScroll += GraphicsConfig::SCROLL_SPEED * dt;

    const display::Vector2f center = _display->getViewCenter();
    const float viewWidth = _cachedHalfWidth * 2.0f;
    const float spriteX = center.x - _cachedHalfWidth;
    const float spriteY = center.y - _cachedHalfHeight;

    // Only process entities that HAVE the Parallax component.
    // We also need Image and Transform. TextureRect will be added if missing.
    registry.view<rc::Parallax, rc::Image, rs::TransformComponent>().each(
        [&registry, this, &center, viewWidth, spriteX, spriteY](
            auto entity, auto& parallax, auto& img, auto& transform) {
            // Ensure TextureRect exists
            if (!registry.hasComponent<rc::TextureRect>(entity)) {
                registry.emplaceComponent<rc::TextureRect>(entity);
            }
            auto& texRect = registry.getComponent<rc::TextureRect>(entity);

            float effectiveOffset = _totalScroll * parallax.scrollFactor;
            int intOffset = static_cast<int>(effectiveOffset);

            // Pin to camera
            transform.x = spriteX;
            transform.y = spriteY;

            auto texture = _display->getTexture(img.textureName);
            if (texture) {
                display::Vector2u texSize = texture->getSize();
                texRect.rect = {intOffset, 0, static_cast<int>(viewWidth) + 1,
                                static_cast<int>(texSize.y)};
            }
        });
}

}  // namespace rtype::games::rtype::client
